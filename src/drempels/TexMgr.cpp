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

#include <magick/api.h>
#include <wand/magick-wand.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <kodi/General.h>

#include "noise1234.h"
#include <Rgbhsl/Rgbhsl.h>

using namespace std;

TexMgr::TexMgr()
  : tw(-2)
  , th(-2)
  , prevTex(nullptr)
  , prevW(0), prevH(0)
  , curTex(nullptr)
  , curW(0), curH(0)
  , nextTex(nullptr)
  , nextW(0), nextH(0)
  , ready(false)
  , dirName()
  , imageDir(nullptr)
  , imageThread(nullptr)
  , nextTexMutex(nullptr)
  , nextTexCond(nullptr)
  , exiting(false)
  , gw(256)
  , gh(256)
{
  srand((unsigned)time(nullptr));
}

TexMgr::~TexMgr()
{
  delete [] curTex;
  delete [] nextTex;

  if (nextTexCond)
  {
    pthread_cond_destroy (nextTexCond);
    free (nextTexCond);
  }

  if (nextTexMutex)
  {
    pthread_mutex_destroy (nextTexMutex);
    free (nextTexMutex);
  }

  free (imageThread);
}

void TexMgr::setImageDir(const char *newDirName)
{
  dirName = newDirName;
}

void TexMgr::start()
{
  nextTexMutex = (pthread_mutex_t *)malloc (sizeof(pthread_mutex_t));
  pthread_mutex_init (nextTexMutex, nullptr);

  nextTexCond = (pthread_cond_t *)malloc (sizeof(pthread_cond_t));
  pthread_cond_init (nextTexCond, nullptr);

  imageThread = (pthread_t *)malloc (sizeof(pthread_t));
  pthread_create (imageThread, nullptr, imageThreadMain, (void *)this);
}

void TexMgr::stop()
{
  exiting = true;

  pthread_mutex_lock (nextTexMutex);
  pthread_cond_signal (nextTexCond);
  pthread_mutex_unlock (nextTexMutex);

  pthread_join(*imageThread, nullptr);
}

bool TexMgr::getNext()
{
  bool ok = false;

  if (pthread_mutex_trylock(nextTexMutex))
  {
    if (ready)
    {
      ready = false;
      ok = true;

      {
        unsigned int *tmp = prevTex;
        unsigned int tmpW = prevW, tmpH = prevH;

        prevTex = curTex;
        prevW = curW; prevH = curH;

        curTex = nextTex;
        curW = nextW; curH = nextH;

        nextTex = tmp;
        nextW = tmpW; nextH = tmpH;
      }

      pthread_cond_signal(nextTexCond);
    }

    pthread_mutex_unlock(nextTexMutex);
  }

  return ok;
}

void TexMgr::genTex()
{
  const int blend_period = (rand() & 0x7) + 1;
  const int aorb_period = (rand() & 0x7) + 1;

  if ((nextTex == nullptr) || (gw > nextW) || (gh > nextH))
  {
    delete [] nextTex;
    nextTex = new unsigned int[gw * gh];
    nextW = gw;
    nextH = gh;
  }

  const float gxo = rand() / (float)(RAND_MAX / 256);
  const float gyo = rand() / (float)(RAND_MAX / 256);

  const float sxo = rand() / (float)(RAND_MAX / 256);
  const float syo = rand() / (float)(RAND_MAX / 256);

  const float ah0 = rand() / (float)RAND_MAX, as0 = rand() / (float)RAND_MAX, al0 = rand() / (float)RAND_MAX;
  const float bh0 = rand() / (float)RAND_MAX, bs0 = rand() / (float)RAND_MAX, bl0 = rand() / (float)RAND_MAX;
  float ah1 = 0, as1 = 0, al1 = 0;
  float bh1 = 0, bs1 = 0, bl1 = 0;

  do {
    ah1 = rand() / (float)RAND_MAX, as1 = rand() / (float)RAND_MAX, al1 = rand() / (float)RAND_MAX;
  } while (fabsf(ah0 - ah1) + fabsf(as0 - as1) + fabsf(al0 - al1) > 1.0);

  do {
    bh1 = rand() / (float)RAND_MAX, bs1 = rand() / (float)RAND_MAX, bl1 = rand() / (float)RAND_MAX;
  } while (fabsf(bh0 - bh1) + fabsf(bs0 - bs1) + fabsf(bl0 - bl1) > 1.0);

  const bool adir = fabsf(ah1 - ah0) > 0.5 ? 1 : 0;
  const bool bdir = fabsf(bh1 - bh0) > 0.5 ? 1 : 0;

  int uu = 0;
  for (unsigned int ii = 0; ii < gh; ++ii) {
    for (unsigned int jj = 0; jj < gw; ++jj) {
      float blend = pnoise2(gxo + ii * blend_period / (float)gh, gyo + jj * blend_period / (float)gw, blend_period, blend_period);
      float aorb = pnoise2(sxo + ii * aorb_period / (float)gh, syo + jj * aorb_period / (float)gw, aorb_period, aorb_period);

      // Punch it through cosine to avoid the non-existent bounds guarantee on the noise.
      blend = (cos(blend * 2 * M_PI) + 1.0) / 2.0;

      float h, s, l, r, g, b;
      if (aorb < 0.0) {
        hslTween(ah0, as0, al0, ah1, as1, al1, blend, adir, h, s, l);
      } else {
        hslTween(bh0, bs0, bl0, bh1, bs1, bl1, blend, bdir, h, s, l);
      }

      // Darken the boundary
      l = min(l, fabs(aorb) * (aorb_period) + 0.1f);

      hsl2rgb(h, s, l, r, g, b);

      nextTex[uu++] = 0xff000000 + (unsigned int)(r * 255) + ((unsigned int)(g * 255) << 8) + ((unsigned int)(b * 255) << 16);
    }
  }

  ready = true;
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
  MagickWand *magick_wand = NewMagickWand();
  ExceptionInfo exception;
  int dirLoop = 0;

  GetExceptionInfo (&exception);

  int imageLoaded = 0;
  do {
    struct dirent *file;

    if (!imageDir) {
      if (dirLoop) {
        dirName = "";
        return;
      }

      imageDir = opendir (dirName.c_str());
      dirLoop = 1;
    }

    file = readdir (imageDir);
    if (file) {
      struct stat fileStat;
      string full_path_and_name = dirName + "/" + file->d_name;

      if (!stat(full_path_and_name.c_str(), (struct stat *)&fileStat)) {
        if (S_ISREG(fileStat.st_mode)) {
          if (MagickReadImage(magick_wand, full_path_and_name.c_str ()) == MagickFalse) {
            char *description;
            ExceptionType severity;

            description = MagickGetException(magick_wand, &severity);
            kodi::Log(ADDON_LOG_ERROR, "Error loading %s: %s", full_path_and_name.c_str(), description);
            description = (char *)MagickRelinquishMemory (description);
          } else {
            imageLoaded = 1;
          }
        }
      }
    } else {
      closedir(imageDir);
      imageDir = nullptr;
    }
  } while (!imageLoaded);

  const unsigned int iww = MagickGetImageWidth (magick_wand);
  const unsigned int ihh = MagickGetImageHeight (magick_wand);
  const unsigned int oww = computeDesiredSize(iww, tw);
  const unsigned int ohh = computeDesiredSize(ihh, th);

  if ((iww != oww) || (ihh != ohh))
  {
    MagickScaleImage (magick_wand, oww, ohh);
  }

  if ((nextTex == nullptr) || (oww > nextW) || (ohh > nextH))
  {
    delete [] nextTex;
    nextTex = new unsigned int[oww * ohh];
    nextW = oww;
    nextH = ohh;
  }

  ExportImagePixels (GetImageFromMagickWand(magick_wand), 0, 0, oww, ohh, "RGBA", CharPixel, nextTex, &exception);

  magick_wand = DestroyMagickWand (magick_wand);

  ready = true;
}

void *TexMgr::imageThreadMain(void *vp)
{
  TexMgr *t = (TexMgr *)vp;

  pthread_mutex_lock (t->nextTexMutex);

  do {
    if (t->dirName.length() > 0)
    {
      t->loadNextImageFromDisk();
    }
    else
    {
      t->genTex();
    }

    pthread_cond_wait (t->nextTexCond, t->nextTexMutex);
  } while (!t->exiting);

  pthread_mutex_unlock (t->nextTexMutex);

  pthread_exit(nullptr);

  return nullptr;
}

