/*
 *  Copyright (C) 2005-2019 Team Kodi
 *  Copyright (C) 1999-2010 Terence M. Welsh
 *  Ported to Kodi by Alwin Esch <alwinus@kodi.tv>
 *  This file is part of Kodi - https://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kodi; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

/*
 * Code is based on:
 *   https://github.com/reallyslickscreensavers/reallyslickscreensavers
 *   http://rss-glx.sourceforge.net/
 * and reworked to GL 4.0.
 */

#include "smoke.h"
#include "smoketex.h"
#include "main.h"

#include <gli/gli.hpp>
#include <kodi/gui/gl/Texture.h>

CSmoke::~CSmoke()
{
  glDeleteTextures(5, m_smoketex);
}

// Initialize smoke texture objects and display lists
void CSmoke::Init()
{
  int i, j;

  unsigned char smoke1[SMOKETEXSIZE][SMOKETEXSIZE][2];
  unsigned char smoke2[SMOKETEXSIZE][SMOKETEXSIZE][2];
  unsigned char smoke3[SMOKETEXSIZE][SMOKETEXSIZE][2];
  unsigned char smoke4[SMOKETEXSIZE][SMOKETEXSIZE][2];
  unsigned char smoke5[SMOKETEXSIZE][SMOKETEXSIZE][2];

  for (i = 0; i < SMOKETEXSIZE; i++)
  {
    for (j=0; j < SMOKETEXSIZE; j++)
    {
      smoke1[i][j][0] = 255;
      smoke2[i][j][0] = 255;
      smoke3[i][j][0] = 255;
      smoke4[i][j][0] = 255;
      smoke5[i][j][0] = 255;
      smoke1[i][j][1] = presmoke1[i][j];
      smoke2[i][j][1] = presmoke2[i][j];
      smoke3[i][j][1] = presmoke3[i][j];
      smoke4[i][j][1] = presmoke4[i][j];
      smoke5[i][j][1] = presmoke5[i][j];
    }
  }

  {
    gli::texture Texture(gli::TARGET_2D, gli::FORMAT_LA8_UNORM_PACK8, gli::texture::extent_type(SMOKETEXSIZE, SMOKETEXSIZE, 1), 1, 1, 1);
    std::memcpy(Texture.data(), smoke1, Texture.size());
    m_smoketex[0] = kodi::gui::gl::Load(Texture);
  }
  {
    gli::texture Texture(gli::TARGET_2D, gli::FORMAT_LA8_UNORM_PACK8, gli::texture::extent_type(SMOKETEXSIZE, SMOKETEXSIZE, 1), 1, 1, 1);
    std::memcpy(Texture.data(), smoke2, Texture.size());
    m_smoketex[1] = kodi::gui::gl::Load(Texture);
  }
  {
    gli::texture Texture(gli::TARGET_2D, gli::FORMAT_LA8_UNORM_PACK8, gli::texture::extent_type(SMOKETEXSIZE, SMOKETEXSIZE, 1), 1, 1, 1);
    std::memcpy(Texture.data(), smoke3, Texture.size());
    m_smoketex[2] = kodi::gui::gl::Load(Texture);
  }
  {
    gli::texture Texture(gli::TARGET_2D, gli::FORMAT_LA8_UNORM_PACK8, gli::texture::extent_type(SMOKETEXSIZE, SMOKETEXSIZE, 1), 1, 1, 1);
    std::memcpy(Texture.data(), smoke4, Texture.size());
    m_smoketex[3] = kodi::gui::gl::Load(Texture);
  }
  {
    gli::texture Texture(gli::TARGET_2D, gli::FORMAT_LA8_UNORM_PACK8, gli::texture::extent_type(SMOKETEXSIZE, SMOKETEXSIZE, 1), 1, 1, 1);
    std::memcpy(Texture.data(), smoke5, Texture.size());
    m_smoketex[4] = kodi::gui::gl::Load(Texture);
  }

  for (i = 0; i < 5; i++)
  {
    m_smokelist[i][0].coord = sCoord(0.0f, 0.0f);
    m_smokelist[i][0].vertex = sPosition(-0.5f, -0.5f, 0.0f);
    m_smokelist[i][1].coord = sCoord(1.0f, 0.0f);
    m_smokelist[i][1].vertex = sPosition(0.5f, -0.5f, 0.0f);
    m_smokelist[i][2].coord = sCoord(0.0f, 1.0f);
    m_smokelist[i][2].vertex = sPosition(-0.5f, 0.5f, 0.0f);
    m_smokelist[i][3].coord = sCoord(1.0f, 1.0f);
    m_smokelist[i][3].vertex = sPosition(0.5f, 0.5f, 0.0f);
  }

  // set smoke lifespans  ( 1 2 1 4 1 2 1 8 )
  // This deserves a little more explanation:  smoke particles in this saver expand
  // over time.  If they all have the same lifespans, then they overlap too much and
  // that looks bad.  If every other particle fades out, the remaining ones have more
  // room to expand into.  So we use these smoke times to halve the number of particles
  // a few times.
  m_smokeTime[0] = m_smokeTime[2] = m_smokeTime[4] = m_smokeTime[6] = 0.4f;
  m_smokeTime[1] = m_smokeTime[5] = 0.8f;
  m_smokeTime[3] = 2.0f;
  m_smokeTime[7] = 4.0f;
  for (i = 0; i < SMOKETIMES; i++)
  {
    if (m_smokeTime[i] > float(m_base->Settings().dSmoke))
      m_smokeTime[i] = float(m_base->Settings().dSmoke);
  }
  if (m_smokeTime[7] < float(m_base->Settings().dSmoke))
    m_smokeTime[7] = float(m_base->Settings().dSmoke);

  // create table describing which particles will emit smoke
  // 0 = don't emit smoke
  // 1 = emit smoke
  for (i = 0; i < WHICHSMOKES; i++)
    m_whichSmoke[i] = 0;
  if (m_base->Settings().dExplosionsmoke)
  {
    float index = float(WHICHSMOKES) / float(m_base->Settings().dExplosionsmoke);
    for (i = 0; i < m_base->Settings().dExplosionsmoke; i++)
      m_whichSmoke[int(float(i) * index)] = 1;
  }
}

void CSmoke::Draw(unsigned int entry, const sColor& color)
{
  m_smokelist[entry][0].color = m_smokelist[entry][1].color = m_smokelist[entry][2].color = m_smokelist[entry][3].color = color;
  m_base->BindTexture(GL_TEXTURE_2D, m_smoketex[entry]);
  m_base->DrawEntry(GL_TRIANGLE_STRIP, m_smokelist[entry], 4);
}
