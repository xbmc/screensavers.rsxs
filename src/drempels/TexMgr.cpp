/*
 * Copyright (C) 2009 Tugrul Galatali <tugrul@galatali.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#define MAGICKCORE_QUANTUM_DEPTH 16
#define MAGICKCORE_HDRI_ENABLE 0

#include "TexMgr.h"

#include <algorithm>
#include <cmath>
#ifndef M_PI
#include <math.h>
#endif
#include <cstdlib>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <kodi/General.h>
#include <Rgbhsl/Rgbhsl.h>

#include "noise1234.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

using namespace std;

TexMgr::TexMgr()
{
  srand((unsigned)time(nullptr));
}

TexMgr::~TexMgr()
{
  delete m_imageThread;
  delete [] m_curTex;
  delete [] m_nextTex;
}

void TexMgr::setImageDir(const std::string& newDirName)
{
  m_dirName = newDirName;
}

void TexMgr::start()
{
  m_imageThread = new std::thread(&TexMgr::imageThreadMain, this);
}

void TexMgr::stop()
{
  m_exiting = true;

  {
    std::unique_lock<std::mutex> lck(m_nextTexMutex);
    m_nextTexCond.notify_one();
  }

  m_imageThread->join();
}

bool TexMgr::getNext()
{
  bool ok = false;

  std::unique_lock<std::mutex> lck(m_nextTexMutex, std::defer_lock);
  if (lck.try_lock())
  {
    if (m_ready)
    {
      m_ready = false;
      ok = true;

      {
        uint32_t *tmp = m_prevTex;
        uint32_t tmpW = m_prevW, tmpH = m_prevH;

        m_prevTex = m_curTex;
        m_prevW = m_curW; m_prevH = m_curH;

        m_curTex = m_nextTex;
        m_curW = m_nextW; m_curH = m_nextH;

        m_nextTex = tmp;
        m_nextW = tmpW; m_nextH = tmpH;
      }

      m_nextTexCond.notify_one();
    }
  }

  return ok;
}

void TexMgr::genTex()
{
  const int blend_period = (rand() & 0x7) + 1;
  const int aorb_period = (rand() & 0x7) + 1;

  if ((m_nextTex == nullptr) || (m_gw > m_nextW) || (m_gh > m_nextH))
  {
    delete [] m_nextTex;
    m_nextTex = new uint32_t[m_gw * m_gh];
    m_nextW = m_gw;
    m_nextH = m_gh;
  }

  const float gxo = rand() / (float)(RAND_MAX / 256);
  const float gyo = rand() / (float)(RAND_MAX / 256);

  const float sxo = rand() / (float)(RAND_MAX / 256);
  const float syo = rand() / (float)(RAND_MAX / 256);

  const float ah0 = rand() / (float)RAND_MAX, as0 = rand() / (float)RAND_MAX, al0 = rand() / (float)RAND_MAX;
  const float bh0 = rand() / (float)RAND_MAX, bs0 = rand() / (float)RAND_MAX, bl0 = rand() / (float)RAND_MAX;
  float ah1 = 0, as1 = 0, al1 = 0;
  float bh1 = 0, bs1 = 0, bl1 = 0;

  do
  {
    ah1 = rand() / (float)RAND_MAX, as1 = rand() / (float)RAND_MAX, al1 = rand() / (float)RAND_MAX;
  } while (fabsf(ah0 - ah1) + fabsf(as0 - as1) + fabsf(al0 - al1) > 1.0);

  do 
  {
    bh1 = rand() / (float)RAND_MAX, bs1 = rand() / (float)RAND_MAX, bl1 = rand() / (float)RAND_MAX;
  } while (fabsf(bh0 - bh1) + fabsf(bs0 - bs1) + fabsf(bl0 - bl1) > 1.0);

  const bool adir = fabsf(ah1 - ah0) > 0.5 ? 1 : 0;
  const bool bdir = fabsf(bh1 - bh0) > 0.5 ? 1 : 0;

  int uu = 0;
  for (unsigned int ii = 0; ii < m_gh; ++ii)
  {
    for (unsigned int jj = 0; jj < m_gw; ++jj)
    {
      float blend = pnoise2(gxo + ii * blend_period / (float)m_gh, gyo + jj * blend_period / (float)m_gw, blend_period, blend_period);
      float aorb = pnoise2(sxo + ii * aorb_period / (float)m_gh, syo + jj * aorb_period / (float)m_gw, aorb_period, aorb_period);

      // Punch it through cosine to avoid the non-existent bounds guarantee on the noise.
      blend = (cos(blend * 2 * M_PI) + 1.0) / 2.0;

      float h, s, l, r, g, b;
      if (aorb < 0.0) 
      {
        hslTween(ah0, as0, al0, ah1, as1, al1, blend, adir, h, s, l);
      }
      else 
      {
        hslTween(bh0, bs0, bl0, bh1, bs1, bl1, blend, bdir, h, s, l);
      }

      // Darken the boundary
      l = min(l, fabs(aorb) * (aorb_period) + 0.1f);

      hsl2rgb(h, s, l, r, g, b);

      m_nextTex[uu++] = 0xff000000 + (uint32_t)(r * 255) + ((uint32_t)(g * 255) << 8) + ((uint32_t)(b * 255) << 16);
    }
  }

  m_ready = true;
}

static unsigned int computeDesiredSize(const unsigned int input, const int desired)
{
  if (desired == -1)
    return input;

  if (desired == -2)
  {
    unsigned int output = input;

    // http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    output--;
    output |= output >> 1;
    output |= output >> 2;
    output |= output >> 4;
    output |= output >> 8;
    output |= output >> 16;
    output++;

    return output;
  }

  return desired;
}

// Directory scanning + image loading code in a separate function callable either from loadNextImage or another thread if pthreads is available.
void TexMgr::loadNextImageFromDisk()
{
  int dirLoop = 0;
  bool imageLoaded = false;
  int width, height, channels;
  unsigned char *image = nullptr;

  do 
  {
    struct dirent *file;

    if (!m_imageDir)
    {
      if (dirLoop) 
      {
        m_dirName = "";
        return;
      }

      m_imageDir = opendir (m_dirName.c_str());
      dirLoop = 1;
    }

    file = readdir (m_imageDir);
    if (file) 
    {
      struct stat fileStat;
      string full_path_and_name = m_dirName + "/" + file->d_name;

      if (!stat(full_path_and_name.c_str(), (struct stat *)&fileStat)) 
      {
        if (S_ISREG(fileStat.st_mode)) 
        {
          image = stbi_load(full_path_and_name.c_str(),
                            &width,
                            &height,
                            &channels,
                            STBI_rgb_alpha);

          if (!image) 
            kodi::Log(ADDON_LOG_ERROR, "Error loading %s: %s", full_path_and_name.c_str(), stbi_failure_reason());
          else 
            imageLoaded = true;
        }
      }
    } 
    else 
    {
      closedir(m_imageDir);
      m_imageDir = nullptr;
    }
  } while (!imageLoaded);


  if (image)
  {
    const uint32_t oww = computeDesiredSize(width, m_tw);
    const uint32_t ohh = computeDesiredSize(height, m_th);
  
    if ((width != oww) || (height != ohh))
    {
      stbir_resize_uint8(image, width, height, 0, image, oww, ohh, 0, STBI_rgb_alpha);
    }

    if ((m_nextTex == nullptr) || (oww > m_nextW) || (ohh > m_nextH))
    {
      delete [] m_nextTex;
      m_nextTex = new uint32_t[oww * ohh];
      m_nextW = oww;
      m_nextH = ohh;
    }
    
    memcpy(m_nextTex, image, oww * ohh * sizeof(uint32_t));
    stbi_image_free(image);
  }

  m_ready = true;
}

void TexMgr::imageThreadMain()
{
  do 
  {
    std::unique_lock<std::mutex> lck(m_nextTexMutex);
    if (!m_dirName.empty())
    {
      loadNextImageFromDisk();
    }
    else
    {
      genTex();
    }

    m_nextTexCond.wait(lck);
  } while (!m_exiting);

  return;
}
