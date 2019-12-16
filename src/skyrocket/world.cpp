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

#include <math.h>

#include <rsMath/rsMath.h>
#include <kodi/gui/gl/Texture.h>
#include <glm/ext.hpp>
#include <gli/gli.hpp>

#include "world.h"
#include "flare.h"
#include "cloudtex.h"
#include "moontex.h"
#include "earthtex.h"
#include "main.h"

CWorld::~CWorld()
{
  glDeleteTextures(1, &m_startex);
  glDeleteTextures(1, &m_moontex);
  glDeleteTextures(1, &m_moonglowtex);
  glDeleteTextures(1, &m_cloudtex);
  glDeleteTextures(1, &m_sunsettex);
  glDeleteTextures(1, &m_earthneartex);
  glDeleteTextures(1, &m_earthfartex);
  glDeleteTextures(1, &m_earthlighttex);
}

void CWorld::Init()
{
  int i, j;
  float x, y, z;

  // Initialize cloud texture object even if clouds are not turned on.
  // Sunsets and shockwaves can also use cloud texture.
  gli::texture Texture(gli::TARGET_2D, gli::FORMAT_LA8_UNORM_PACK8, gli::texture::extent_type(CLOUDTEXSIZE, CLOUDTEXSIZE, 1), 1, 1, 1);
  std::memcpy(Texture.data(), cloudmap, Texture.size());
  m_cloudtex = kodi::gui::gl::Load(Texture);

  // initialize star texture
  if (m_base->Settings().dStardensity)
  {
    uint8_t (*starmap)[STARTEXSIZE][3] = new uint8_t[STARTEXSIZE][STARTEXSIZE][3];
    for (i = 0; i < STARTEXSIZE; i++)
    {
      for (j = 0; j < STARTEXSIZE; j++)
      {
        starmap[i][j][0] = starmap[i][j][1] = starmap[i][j][2] = 0;
      }
    }

    int u, v;
    unsigned int rgb[3];
    for (i = 0; i < (m_base->Settings().dStardensity*100); i++)
    {
      u = rsRandi(STARTEXSIZE-4) + 2;
      v = rsRandi(STARTEXSIZE-4) + 2;
      rgb[0] = 220 + rsRandi(36);
      rgb[1] = 220 + rsRandi(36);
      rgb[2] = 220 + rsRandi(36);
      rgb[rsRandi(3)] = 255;
      starmap[u][v][0] = rgb[0];
      starmap[u][v][1] = rgb[1];
      starmap[u][v][2] = rgb[2];
      switch(rsRandi(15))  // different stars
      {
      case 0:  // small
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
        starmap[u][v][0] /= 2;
        starmap[u][v][1] /= 2;
        starmap[u][v][2] /= 2;
        break;
      case 8:  // medium
      case 9:
      case 10:
      case 11:
        starmap[u+1][v][0]=
        starmap[u-1][v][0]=
        starmap[u][v+1][0]=
        starmap[u][v-1][0]=rgb[0]/4;

        starmap[u+1][v][1]=
        starmap[u-1][v][1]=
        starmap[u][v+1][1]=
        starmap[u][v-1][1]=rgb[1]/4;

        starmap[u+1][v][2]=
        starmap[u-1][v][2]=
        starmap[u][v+1][2]=
        starmap[u][v-1][2]=rgb[2]/4;

        starmap[u+1][v+1][0]=
        starmap[u+1][v-1][0]=
        starmap[u-1][v+1][0]=
        starmap[u-1][v-1][0]=rgb[0]/8;

        starmap[u+1][v+1][1]=
        starmap[u+1][v-1][1]=
        starmap[u-1][v+1][1]=
        starmap[u-1][v-1][1]=rgb[1]/8;

        starmap[u+1][v+1][2]=
        starmap[u+1][v-1][2]=
        starmap[u-1][v+1][2]=
        starmap[u-1][v-1][2]=rgb[2]/8;
        break;
      case 12:  // large
      case 13:
        starmap[u+1][v][0]=
        starmap[u-1][v][0]=
        starmap[u][v+1][0]=
        starmap[u][v-1][0]=rgb[0]/2;

        starmap[u+1][v][1]=
        starmap[u-1][v][1]=
        starmap[u][v+1][1]=
        starmap[u][v-1][1]=rgb[1]/2;

        starmap[u+1][v][2]=
        starmap[u-1][v][2]=
        starmap[u][v+1][2]=
        starmap[u][v-1][2]=rgb[2]/2;

        starmap[u+1][v+1][0]=
        starmap[u+1][v-1][0]=
        starmap[u-1][v+1][0]=
        starmap[u-1][v-1][0]=rgb[0]/4;

        starmap[u+1][v+1][1]=
        starmap[u+1][v-1][1]=
        starmap[u-1][v+1][1]=
        starmap[u-1][v-1][1]=rgb[1]/4;

        starmap[u+1][v+1][2]=
        starmap[u+1][v-1][2]=
        starmap[u-1][v+1][2]=
        starmap[u-1][v-1][2]=rgb[2]/4;
        break;
      case 14:  // X-large
        starmap[u+1][v][0]=
        starmap[u-1][v][0]=
        starmap[u][v+1][0]=
        starmap[u][v-1][0]=rgb[0];

        starmap[u+1][v][1]=
        starmap[u-1][v][1]=
        starmap[u][v+1][1]=
        starmap[u][v-1][1]=rgb[1];

        starmap[u+1][v][2]=
        starmap[u-1][v][2]=
        starmap[u][v+1][2]=
        starmap[u][v-1][2]=rgb[2];

        starmap[u+1][v+1][0]=
        starmap[u+1][v-1][0]=
        starmap[u-1][v+1][0]=
        starmap[u-1][v-1][0]=rgb[0]/2;

        starmap[u+1][v+1][1]=
        starmap[u+1][v-1][1]=
        starmap[u-1][v+1][1]=
        starmap[u-1][v-1][1]=rgb[1]/2;

        starmap[u+1][v+1][2]=
        starmap[u+1][v-1][2]=
        starmap[u-1][v+1][2]=
        starmap[u-1][v-1][2]=rgb[2]/2;
      }
    }
    /*if (m_base->Settings().dAmbient > 50){  // blue sky replaces stars (like it's daytime)
      const float black(float(100 - m_base->Settings().dAmbient) / 50.0f);
      const uint8_t blue((uint8_t)(float(m_base->Settings().dAmbient - 50) * 255.0f / 50.0f));
      for (i = 0; i < STARTEXSIZE; i++){
        for (j = 0; j < STARTEXSIZE; j++){
          starmap[i][j][0] = (uint8_t)(float(starmap[i][j][0]) * black);
          starmap[i][j][1] = (uint8_t)(float(starmap[i][j][1]) * black);
          starmap[i][j][2] = (uint8_t)(float(starmap[i][j][2]) * black);
          starmap[i][j][0] += blue / 4;
          starmap[i][j][1] += blue / 4;
          starmap[i][j][2] += blue;
        }
      }
    }*/

    gli::texture Texture(gli::TARGET_2D, gli::FORMAT_RGB8_UNORM_PACK8, gli::texture::extent_type(STARTEXSIZE, STARTEXSIZE, 1), 1, 1, 1);
    std::memcpy(Texture.data(), starmap, Texture.size());
    m_startex = kodi::gui::gl::Load(Texture);
    delete[] starmap;
  }

  //initialize moon texture
  if (m_base->Settings().dMoon)
  {
    gli::texture Texture(gli::TARGET_2D, gli::FORMAT_LA8_UNORM_PACK8, gli::texture::extent_type(MOONTEXSIZE, MOONTEXSIZE, 1), 1, 1, 1);
    std::memcpy(Texture.data(), moonmap, Texture.size());
    m_moontex = kodi::gui::gl::Load(Texture);
  }

  //initialize moon glow texture
  if (m_base->Settings().dMoonglow)
  {
    uint8_t (*moonglowmap)[MOONGLOWTEXSIZE][3] = new uint8_t[MOONGLOWTEXSIZE][MOONGLOWTEXSIZE][3];

    float temp1, temp2, temp3, u, v;
    for (i = 0; i < MOONGLOWTEXSIZE; i++)
    {
      for (j = 0; j < MOONGLOWTEXSIZE; j++)
      {
        u = float(i - MOONGLOWTEXSIZE / 2) / float(MOONGLOWTEXSIZE / 2);
        v = float(j - MOONGLOWTEXSIZE / 2) / float(MOONGLOWTEXSIZE / 2);
        temp1 = 4.0f * ((u * u) + (v * v)) * (1.0f - ((u * u) + (v * v)));
        if (temp1 > 1.0f)
          temp1 = 1.0f;
        if (temp1 < 0.0f)
          temp1 = 0.0f;
        temp1 = temp1 * temp1 * temp1 * temp1;
        u *= 1.2f;
        v *= 1.2f;
        temp2 = 4.0f * ((u * u) + (v * v)) * (1.0f - ((u * u) + (v * v)));
        if (temp2 > 1.0f)
          temp2 = 1.0f;
        if (temp2 < 0.0f)
          temp2 = 0.0f;
        temp2 = temp2 * temp2 * temp2 * temp2;
        u *= 1.25f;
        v *= 1.25f;
        temp3 = 4.0f * ((u * u) + (v * v)) * (1.0f - ((u * u) + (v * v)));
        if (temp3 > 1.0f)
          temp3 = 1.0f;
        if (temp3 < 0.0f)
          temp3 = 0.0f;
        temp3 = temp3 * temp3 * temp3 * temp3;
        //moonglowmap[i][j][0] = (uint8_t)(255.0f * (temp1 * 0.4f + temp2 * 0.4f + temp3 * 0.48f));
        //moonglowmap[i][j][1] = (uint8_t)(255.0f * (temp1 * 0.4f + temp2 * 0.48f + temp3 * 0.38f));
        //moonglowmap[i][j][2] = (uint8_t)(255.0f * (temp1 * 0.48f + temp2 * 0.4f + temp3 * 0.38f));
        moonglowmap[i][j][0] = (uint8_t)(255.0f * (temp1 * 0.45f + temp2 * 0.4f + temp3 * 0.6f));
        moonglowmap[i][j][1] = (uint8_t)(255.0f * (temp1 * 0.45f + temp2 * 0.54f + temp3 * 0.45f));
        moonglowmap[i][j][2] = (uint8_t)(255.0f * (temp1 * 0.6f + temp2 * 0.4f + temp3 * 0.45f));
        //moonglowmap[i][j][3] = (uint8_t)(255.0f * (temp1 * 0.48f + temp2 * 0.48f + temp3 * 0.48f));
      }
    }

    gli::texture Texture(gli::TARGET_2D, gli::FORMAT_RGB8_UNORM_PACK8, gli::texture::extent_type(MOONGLOWTEXSIZE, MOONGLOWTEXSIZE, 1), 1, 1, 1);
    std::memcpy(Texture.data(), moonglowmap, Texture.size());
    m_moonglowtex = kodi::gui::gl::Load(Texture);

    delete[] moonglowmap;
  }

  // do a sunset?
  m_doSunset = 1;
  if (!rsRandi(4))
    m_doSunset = 0;
  // initialize sunset texture
  if (m_doSunset)
  {
    uint8_t (*sunsetmap)[CLOUDTEXSIZE][3] = new uint8_t[CLOUDTEXSIZE][CLOUDTEXSIZE][3];
    uint8_t rgb[3];
    float temp;
    if (rsRandi(3))
      rgb[0] = 60 + rsRandi(42);
    else
      rgb[0] = rsRandi(102);
    rgb[1] = rsRandi(rgb[0]);
    rgb[2] = 0;
    if (rgb[1] < 50)
      rgb[2] = 100 - rsRandi(rgb[0]);
    for (i = 0; i < CLOUDTEXSIZE; i++)
    {
      for (j = 0; j < CLOUDTEXSIZE; j++)
      {
        sunsetmap[i][j][0] = rgb[0];
        sunsetmap[i][j][1] = rgb[1];
        sunsetmap[i][j][2] = rgb[2];
      }
    }
    // clouds in sunset
    if (rsRandi(3))
    {
      float cloudinf;  // influence of clouds
      int xoffset = rsRandi(CLOUDTEXSIZE);
      int yoffset = rsRandi(CLOUDTEXSIZE);
      int x, y;
      for (i = 0; i < CLOUDTEXSIZE; i++)
      {
        for (j = 0; j < CLOUDTEXSIZE; j++)
        {
          x = (i + xoffset) % CLOUDTEXSIZE;
          y = (j + yoffset) % CLOUDTEXSIZE;
          cloudinf = float(cloudmap[x][y][1]) / 256.0f;
          temp = float(sunsetmap[i][j][0]) / 256.0f;
          temp *= cloudinf;
          sunsetmap[i][j][0] = (uint8_t)(temp * 256.0f);
          cloudinf *= float(cloudmap[x][y][0]) / 256.0f;
          temp = float(sunsetmap[i][j][1]) / 256.0f;
          temp *= cloudinf;
          sunsetmap[i][j][1] = (uint8_t)(temp * 256.0f);
        }
      }
    }
    // Fractal mountain generation
    int mountains[CLOUDTEXSIZE+1];
    mountains[0] = mountains[CLOUDTEXSIZE] = rsRandi(10) + 5;
    makeHeights(0, CLOUDTEXSIZE, mountains);
    for (i = 0; i < CLOUDTEXSIZE; i++)
    {
      for (j = 0; j <= mountains[i]; j++)
      {
        sunsetmap[i][j][0] = 0;
        sunsetmap[i][j][1] = 0;
        sunsetmap[i][j][2] = 0;
      }
      sunsetmap[i][mountains[i]+1][0] /= 4;
      sunsetmap[i][mountains[i]+1][1] /= 4;
      sunsetmap[i][mountains[i]+1][2] /= 4;
      sunsetmap[i][mountains[i]+2][0] /= 2;
      sunsetmap[i][mountains[i]+2][1] /= 2;
      sunsetmap[i][mountains[i]+2][2] /= 2;
      sunsetmap[i][mountains[i]+3][0] =
        (uint8_t)(float(sunsetmap[i][mountains[i]+3][0]) * 0.75f);
      sunsetmap[i][mountains[i]+3][1] =
        (uint8_t)(float(sunsetmap[i][mountains[i]+3][1]) * 0.75f);
      sunsetmap[i][mountains[i]+3][2] =
        (uint8_t)(float(sunsetmap[i][mountains[i]+3][2]) * 0.75f);
    }

    gli::texture Texture(gli::TARGET_2D, gli::FORMAT_RGB8_UNORM_PACK8, gli::texture::extent_type(CLOUDTEXSIZE, CLOUDTEXSIZE, 1), 1, 1, 1);
    std::memcpy(Texture.data(), sunsetmap, Texture.size());
    m_sunsettex = kodi::gui::gl::Load(Texture);

    delete[] sunsetmap;
  }

  //initialize earth texture
  if (m_base->Settings().dEarth)
  {
    {
      gli::texture Texture(gli::TARGET_2D, gli::FORMAT_RGB8_UNORM_PACK8, gli::texture::extent_type(EARTHNEARSIZE, EARTHNEARSIZE, 1), 1, 1, 1);
      std::memcpy(Texture.data(), earthnearmap, Texture.size());
      m_earthneartex = kodi::gui::gl::Load(Texture);
    }
    {
      gli::texture Texture(gli::TARGET_2D, gli::FORMAT_RGB8_UNORM_PACK8, gli::texture::extent_type(EARTHFARSIZE, EARTHFARSIZE, 1), 1, 1, 1);
      std::memcpy(Texture.data(), earthfarmap, Texture.size());
      m_earthfartex = kodi::gui::gl::Load(Texture);
    }
    {
      gli::texture Texture(gli::TARGET_2D, gli::FORMAT_RGB8_UNORM_PACK8, gli::texture::extent_type(EARTHFARSIZE, EARTHFARSIZE, 1), 1, 1, 1);
      std::memcpy(Texture.data(), earthlightmap, Texture.size());
      m_earthlighttex = kodi::gui::gl::Load(Texture);
    }
  }

  // initialize star geometry
  if (m_base->Settings().dStardensity)
  {
    for (j = 0; j < STARMESH/2; j++)
    {
      y = sinf(RS_PIo2 * float(j) / float(STARMESH/2));
      for (i = 0; i <= STARMESH; i++)
      {
        x = cosf(RS_PIx2 * float(i) / float(STARMESH)) * cosf(RS_PIo2 * float(j) / float(STARMESH/2));
        z = sinf(RS_PIx2 * float(i) / float(STARMESH)) * cosf(RS_PIo2 * float(j) / float(STARMESH/2));
        // positions
        m_stars[i][j][0] = x * 20000.0f;
        m_stars[i][j][1] = 1500.0f + 18500.0f * y;
        m_stars[i][j][2] = z * 20000.0f;
        // tex coords
        m_stars[i][j][3] = 1.0f * x * (2.0f - y);
        m_stars[i][j][4] = 1.0f * z * (2.0f - y);
        // brightness
        if (m_stars[i][j][1] < 1501.0f)
          m_stars[i][j][5] = 0.0f;
        else
          m_stars[i][j][5] = 1.0f;
      }
    }

    int ptr = 0;
    m_starlistStripEntries = 0;
    for (j = 0; j < (STARMESH/2-1); j++)
    {
      for (i = 0; i <= STARMESH; i++)
      {
        m_starlistStrip[j][ptr  ].color = sColor(m_stars[i][j+1][5], m_stars[i][j+1][5], m_stars[i][j+1][5]);
        m_starlistStrip[j][ptr  ].coord = sCoord(m_stars[i][j+1][3], m_stars[i][j+1][4]);
        m_starlistStrip[j][ptr++].vertex = sPosition(m_stars[i][j+1]);
        m_starlistStrip[j][ptr  ].color = sColor(m_stars[i][j][5], m_stars[i][j][5], m_stars[i][j][5]);
        m_starlistStrip[j][ptr  ].coord = sCoord(m_stars[i][j][3], m_stars[i][j][4]);
        m_starlistStrip[j][ptr++].vertex = sPosition(m_stars[i][j]);
      }
      m_starlistStripSize = ptr;
      m_starlistStripEntries++;
      ptr = 0;
    }

    j = STARMESH / 2 - 1;
    m_starlistFan[ptr  ].color = sColor(1.0f, 1.0f, 1.0f);
    m_starlistFan[ptr  ].coord = sCoord(0.0f, 0.0f);
    m_starlistFan[ptr++].vertex = sPosition(0.0f, 20000.0f, 0.0f);
    for (i = 0; i <= STARMESH; i++)
    {
      m_starlistFan[ptr  ].color = sColor(m_stars[i][j][5], m_stars[i][j][5], m_stars[i][j][5]);
      m_starlistFan[ptr  ].coord = sCoord(m_stars[i][j][3], m_stars[i][j][4]);
      m_starlistFan[ptr++].vertex = sPosition(m_stars[i][j]);
    }
    m_starlistFanSize = ptr;
  }

  // initialize moon geometry
  if (m_base->Settings().dMoon)
  {
    m_moonRotation = rsRandf(360.0f);
    m_moonHeight = rsRandf(40.0f) + 20.0f;
    m_moonlist[0].color = sColor(1.0f, 1.0f, 1.0f, 1.0f);
    m_moonlist[0].coord = sCoord(0.0f, 0.0f);
    m_moonlist[0].vertex = sPosition(-800.0f, -800.0f, 0.0f);
    m_moonlist[1].color = sColor(1.0f, 1.0f, 1.0f, 1.0f);
    m_moonlist[1].coord = sCoord(1.0f, 0.0f);
    m_moonlist[1].vertex = sPosition(800.0f, -800.0f, 0.0f);
    m_moonlist[2].color = sColor(1.0f, 1.0f, 1.0f, 1.0f);
    m_moonlist[2].coord = sCoord(0.0f, 1.0f);
    m_moonlist[2].vertex = sPosition(-800.0f, 800.0f, 0.0f);
    m_moonlist[3].color = sColor(1.0f, 1.0f, 1.0f, 1.0f);
    m_moonlist[3].coord = sCoord(1.0f, 1.0f);
    m_moonlist[3].vertex = sPosition(800.0f, 800.0f, 0.0f);
  }

  // initialize moon glow geometry
  if (m_base->Settings().dMoonglow)
  {
    m_moonglowlist[0].coord = sCoord(0.0f, 0.0f);
    m_moonglowlist[0].vertex = sPosition(-7000.0f, -7000.0f, 0.0f);
    m_moonglowlist[1].coord = sCoord(1.0f, 0.0f);
    m_moonglowlist[1].vertex = sPosition(7000.0f, -7000.0f, 0.0f);
    m_moonglowlist[2].coord = sCoord(0.0f, 1.0f);
    m_moonglowlist[2].vertex = sPosition(-7000.0f, 7000.0f, 0.0f);
    m_moonglowlist[3].coord = sCoord(1.0f, 1.0f);
    m_moonglowlist[3].vertex = sPosition(7000.0f, 7000.0f, 0.0f);
  }

  // initialize cloud geometry
  if (m_base->Settings().dClouds)
  {
    m_cloudShift = 0.0f;
    for (j = 0; j <= CLOUDMESH; j++)
    {
      for (i = 0; i <= CLOUDMESH; i++)
      {
        x = float(i - (CLOUDMESH / 2));
        z = float(j - (CLOUDMESH / 2));
        m_clouds[i][j][0] = x * (40000.0f / float(CLOUDMESH));
        m_clouds[i][j][2] = z * (40000.0f / float(CLOUDMESH));
        x = float(fabs(x / float(CLOUDMESH / 2)));
        z = float(fabs(z / float(CLOUDMESH / 2)));
        m_clouds[i][j][1] = 2000.0f - 1000.0f * float(x * x + z * z);
        m_clouds[i][j][3] = float(-i) / float(CLOUDMESH / 6);  // tex coords
        m_clouds[i][j][4] = float(-j) / float(CLOUDMESH / 6);
        m_clouds[i][j][5] = (m_clouds[i][j][1] - 1000.0f) * 0.00001f * float(m_base->Settings().dAmbient);  // brightness
        if (m_clouds[i][j][5] < 0.0f)
          m_clouds[i][j][5] = 0.0f;
      }
    }
  }

  // initialize sunset geometry
  if (m_doSunset)
  {
    float vert[6] = {0.0f, 7654.0f, 8000.0f, 14142.0f, 18448.0f, 20000.0f};
    m_sunsetlist[0].color = sColor(0.0f, 0.0f, 0.0f);
    m_sunsetlist[0].coord = sCoord(1.0f, 0.0f);
    m_sunsetlist[0].vertex = sPosition(vert[0], vert[2], vert[5]);

    m_sunsetlist[1].color = sColor(0.0f, 0.0f, 0.0f);
    m_sunsetlist[1].coord = sCoord(0.0f, 0.0f);
    m_sunsetlist[1].vertex = sPosition(vert[0], vert[0], vert[5]);

    m_sunsetlist[2].color = sColor(0.0f, 0.0f, 0.0f);
    m_sunsetlist[2].coord = sCoord(1.0f, 0.125f);
    m_sunsetlist[2].vertex = sPosition(-vert[1], vert[2], vert[4]);

    m_sunsetlist[3].color = sColor(0.25f, 0.25f, 0.25f);
    m_sunsetlist[3].coord = sCoord(0.0f, 0.125f);
    m_sunsetlist[3].vertex = sPosition(-vert[1], vert[0], vert[4]);

    m_sunsetlist[4].color = sColor(0.0f, 0.0f, 0.0f);
    m_sunsetlist[4].coord = sCoord(1.0f, 0.25f);
    m_sunsetlist[4].vertex = sPosition(-vert[3], vert[2], vert[3]);

    m_sunsetlist[5].color = sColor(0.5f, 0.5f, 0.5f);
    m_sunsetlist[5].coord = sCoord(0.0f, 0.25f);
    m_sunsetlist[5].vertex = sPosition(-vert[3], vert[0], vert[3]);

    m_sunsetlist[6].color = sColor(0.0f, 0.0f, 0.0f);
    m_sunsetlist[6].coord = sCoord(1.0f, 0.375f);
    m_sunsetlist[6].vertex = sPosition(-vert[4], vert[2], vert[1]);

    m_sunsetlist[7].color = sColor(0.75f, 0.75f, 0.75f);
    m_sunsetlist[7].coord = sCoord(0.0f, 0.375f);
    m_sunsetlist[7].vertex = sPosition(-vert[4], vert[0], vert[1]);

    m_sunsetlist[8].color = sColor(0.0f, 0.0f, 0.0f);
    m_sunsetlist[8].coord = sCoord(1.0f, 0.5f);
    m_sunsetlist[8].vertex = sPosition(-vert[5], vert[2], vert[0]);

    m_sunsetlist[9].color = sColor(1.0f, 1.0f, 1.0f);
    m_sunsetlist[9].coord = sCoord(0.0f, 0.5f);
    m_sunsetlist[9].vertex = sPosition(-vert[5], vert[0], vert[0]);

    m_sunsetlist[10].color = sColor(0.0f, 0.0f, 0.0f);
    m_sunsetlist[10].coord = sCoord(1.0f, 0.625f);
    m_sunsetlist[10].vertex = sPosition(-vert[4], vert[2], -vert[1]);

    m_sunsetlist[11].color = sColor(0.75f, 0.75f, 0.75f);
    m_sunsetlist[11].coord = sCoord(0.0f, 0.625f);
    m_sunsetlist[11].vertex = sPosition(-vert[4], vert[0], -vert[1]);

    m_sunsetlist[12].color = sColor(0.0f, 0.0f, 0.0f);
    m_sunsetlist[12].coord = sCoord(1.0f, 0.75f);
    m_sunsetlist[12].vertex = sPosition(-vert[3], vert[2], -vert[3]);

    m_sunsetlist[13].color = sColor(0.5f, 0.5f, 0.5f);
    m_sunsetlist[13].coord = sCoord(0.0f, 0.75f);
    m_sunsetlist[13].vertex = sPosition(-vert[3], vert[0], -vert[3]);

    m_sunsetlist[14].color = sColor(0.0f, 0.0f, 0.0f);
    m_sunsetlist[14].coord = sCoord(1.0f, 0.875f);
    m_sunsetlist[14].vertex = sPosition(-vert[1], vert[2], -vert[4]);

    m_sunsetlist[15].color = sColor(0.25f, 0.25f, 0.25f);
    m_sunsetlist[15].coord = sCoord(0.0f, 0.875f);
    m_sunsetlist[15].vertex = sPosition(-vert[1], vert[0], -vert[4]);

    m_sunsetlist[16].color = sColor(0.0f, 0.0f, 0.0f);
    m_sunsetlist[16].coord = sCoord(1.0f, 1.0f);
    m_sunsetlist[16].vertex = sPosition(vert[0], vert[2], -vert[5]);

    m_sunsetlist[17].color = sColor(0.0f, 0.0f, 0.0f);
    m_sunsetlist[17].coord = sCoord(0.0f, 1.0f);
    m_sunsetlist[17].vertex = sPosition(vert[0], vert[0], -vert[5]);
  }

  // initialize earth geometry
  if (m_base->Settings().dEarth)
  {
    float lit[] = {float(m_base->Settings().dAmbient) * 0.01f, float(m_base->Settings().dAmbient) * 0.01f, float(m_base->Settings().dAmbient) * 0.01f};
    float unlit[] = {0.0f, 0.0f, 0.0f};
    float vert[2] = {839.68f, 8396.8f};
    float tex[4] = {0.0f, 0.45f, 0.55f, 1.0f};

    m_earthnearlist[0].color = sColor(lit);
    m_earthnearlist[0].coord = sCoord(tex[0], tex[0]);
    m_earthnearlist[0].vertex = sPosition(-vert[0], 0.0f, -vert[0]);
    m_earthnearlist[1].color = sColor(lit);
    m_earthnearlist[1].coord = sCoord(tex[0], tex[3]);
    m_earthnearlist[1].vertex = sPosition(-vert[0], 0.0f, vert[0]);
    m_earthnearlist[2].color = sColor(lit);
    m_earthnearlist[2].coord = sCoord(tex[3], tex[0]);
    m_earthnearlist[2].vertex = sPosition(vert[0], 0.0f, -vert[0]);
    m_earthnearlist[3].color = sColor(lit);
    m_earthnearlist[3].coord = sCoord(tex[3], tex[3]);
    m_earthnearlist[3].vertex = sPosition(vert[0], 0.0f, vert[0]);

    //--------------------------------------------------------------------------

    m_earthfarlist[0][0].color = sColor(lit);
    m_earthfarlist[0][0].coord = sCoord(tex[1], tex[1]);
    m_earthfarlist[0][0].vertex = sPosition(-vert[0], 0.0f, -vert[0]);
    m_earthfarlist[0][1].color = sColor(lit);
    m_earthfarlist[0][1].coord = sCoord(tex[2], tex[1]);
    m_earthfarlist[0][1].vertex = sPosition(vert[0], 0.0f, -vert[0]);
    m_earthfarlist[0][2].color = sColor(unlit);
    m_earthfarlist[0][2].coord = sCoord(tex[0], tex[0]);
    m_earthfarlist[0][2].vertex = sPosition(-vert[1], 0.0f, -vert[1]);
    m_earthfarlist[0][3].color = sColor(unlit);
    m_earthfarlist[0][3].coord = sCoord(tex[3], tex[0]);
    m_earthfarlist[0][3].vertex = sPosition(vert[1], 0.0f, -vert[1]);

    m_earthfarlist[1][0].color = sColor(lit);
    m_earthfarlist[1][0].coord = sCoord(tex[1], tex[2]);
    m_earthfarlist[1][0].vertex = sPosition(-vert[0], 0.0f, vert[0]);
    m_earthfarlist[1][1].color = sColor(lit);
    m_earthfarlist[1][1].coord = sCoord(tex[1], tex[1]);
    m_earthfarlist[1][1].vertex = sPosition(-vert[0], 0.0f, -vert[0]);
    m_earthfarlist[1][2].color = sColor(unlit);
    m_earthfarlist[1][2].coord = sCoord(tex[0], tex[3]);
    m_earthfarlist[1][2].vertex = sPosition(-vert[1], 0.0f, vert[1]);
    m_earthfarlist[1][3].color = sColor(unlit);
    m_earthfarlist[1][3].coord = sCoord(tex[0], tex[0]);
    m_earthfarlist[1][3].vertex = sPosition(-vert[1], 0.0f, -vert[1]);

    m_earthfarlist[2][0].color = sColor(lit);
    m_earthfarlist[2][0].coord = sCoord(tex[2], tex[2]);
    m_earthfarlist[2][0].vertex = sPosition(vert[0], 0.0f, vert[0]);
    m_earthfarlist[2][1].color = sColor(lit);
    m_earthfarlist[2][1].coord = sCoord(tex[1], tex[2]);
    m_earthfarlist[2][1].vertex = sPosition(-vert[0], 0.0f, vert[0]);
    m_earthfarlist[2][2].color = sColor(unlit);
    m_earthfarlist[2][2].coord = sCoord(tex[3], tex[3]);
    m_earthfarlist[2][2].vertex = sPosition(vert[1], 0.0f, vert[1]);
    m_earthfarlist[2][3].color = sColor(unlit);
    m_earthfarlist[2][3].coord = sCoord(tex[0], tex[3]);
    m_earthfarlist[2][3].vertex = sPosition(-vert[1], 0.0f, vert[1]);

    m_earthfarlist[3][0].color = sColor(lit);
    m_earthfarlist[3][0].coord = sCoord(tex[2], tex[1]);
    m_earthfarlist[3][0].vertex = sPosition(vert[0], 0.0f, -vert[0]);
    m_earthfarlist[3][1].color = sColor(lit);
    m_earthfarlist[3][1].coord = sCoord(tex[2], tex[2]);
    m_earthfarlist[3][1].vertex = sPosition(vert[0], 0.0f, vert[0]);
    m_earthfarlist[3][2].color = sColor(unlit);
    m_earthfarlist[3][2].coord = sCoord(tex[3], tex[0]);
    m_earthfarlist[3][2].vertex = sPosition(vert[1], 0.0f, -vert[1]);
    m_earthfarlist[3][3].color = sColor(unlit);
    m_earthfarlist[3][3].coord = sCoord(tex[3], tex[3]);
    m_earthfarlist[3][3].vertex = sPosition(vert[1], 0.0f, vert[1]);

    //--------------------------------------------------------------------------

    lit[0] = lit[1] = lit[2] = 0.25f;

    m_earthlist[0][0].color = sColor(lit);
    m_earthlist[0][0].coord = sCoord(tex[1], tex[1]);
    m_earthlist[0][0].vertex = sPosition(-vert[0], 0.0f, -vert[0]);
    m_earthlist[0][1].color = sColor(lit);
    m_earthlist[0][1].coord = sCoord(tex[1], tex[2]);
    m_earthlist[0][1].vertex = sPosition(-vert[0], 0.0f, vert[0]);
    m_earthlist[0][2].color = sColor(lit);
    m_earthlist[0][2].coord = sCoord(tex[2], tex[1]);
    m_earthlist[0][2].vertex = sPosition(vert[0], 0.0f, -vert[0]);
    m_earthlist[0][3].color = sColor(lit);
    m_earthlist[0][3].coord = sCoord(tex[2], tex[2]);
    m_earthlist[0][3].vertex = sPosition(vert[0], 0.0f, vert[0]);

    m_earthlist[1][0].color = sColor(lit);
    m_earthlist[1][0].coord = sCoord(tex[1], tex[1]);
    m_earthlist[1][0].vertex = sPosition(-vert[0], 0.0f, -vert[0]);
    m_earthlist[1][1].color = sColor(lit);
    m_earthlist[1][1].coord = sCoord(tex[2], tex[1]);
    m_earthlist[1][1].vertex = sPosition(vert[0], 0.0f, -vert[0]);
    m_earthlist[1][2].color = sColor(unlit);
    m_earthlist[1][2].coord = sCoord(tex[0], tex[0]);
    m_earthlist[1][2].vertex = sPosition(-vert[1], 0.0f, -vert[1]);
    m_earthlist[1][3].color = sColor(unlit);
    m_earthlist[1][3].coord = sCoord(tex[3], tex[0]);
    m_earthlist[1][3].vertex = sPosition(vert[1], 0.0f, -vert[1]);

    m_earthlist[2][0].color = sColor(lit);
    m_earthlist[2][0].coord = sCoord(tex[1], tex[2]);
    m_earthlist[2][0].vertex = sPosition(-vert[0], 0.0f, vert[0]);
    m_earthlist[2][1].color = sColor(lit);
    m_earthlist[2][1].coord = sCoord(tex[1], tex[1]);
    m_earthlist[2][1].vertex = sPosition(-vert[0], 0.0f, -vert[0]);
    m_earthlist[2][2].color = sColor(unlit);
    m_earthlist[2][2].coord = sCoord(tex[0], tex[3]);
    m_earthlist[2][2].vertex = sPosition(-vert[1], 0.0f, vert[1]);
    m_earthlist[2][3].color = sColor(unlit);
    m_earthlist[2][3].coord = sCoord(tex[0], tex[0]);
    m_earthlist[2][3].vertex = sPosition(-vert[1], 0.0f, -vert[1]);

    m_earthlist[3][0].color = sColor(lit);
    m_earthlist[3][0].coord = sCoord(tex[2], tex[2]);
    m_earthlist[3][0].vertex = sPosition(vert[0], 0.0f, vert[0]);
    m_earthlist[3][1].color = sColor(lit);
    m_earthlist[3][1].coord = sCoord(tex[1], tex[2]);
    m_earthlist[3][1].vertex = sPosition(-vert[0], 0.0f, vert[0]);
    m_earthlist[3][2].color = sColor(unlit);
    m_earthlist[3][2].coord = sCoord(tex[3], tex[3]);
    m_earthlist[3][2].vertex = sPosition(vert[1], 0.0f, vert[1]);
    m_earthlist[3][3].color = sColor(unlit);
    m_earthlist[3][3].coord = sCoord(tex[0], tex[3]);
    m_earthlist[3][3].vertex = sPosition(-vert[1], 0.0f, vert[1]);

    m_earthlist[4][0].color = sColor(lit);
    m_earthlist[4][0].coord = sCoord(tex[2], tex[1]);
    m_earthlist[4][0].vertex = sPosition(vert[0], 0.0f, -vert[0]);
    m_earthlist[4][1].color = sColor(lit);
    m_earthlist[4][1].coord = sCoord(tex[2], tex[2]);
    m_earthlist[4][1].vertex = sPosition(vert[0], 0.0f, vert[0]);
    m_earthlist[4][2].color = sColor(unlit);
    m_earthlist[4][2].coord = sCoord(tex[3], tex[0]);
    m_earthlist[4][2].vertex = sPosition(vert[1], 0.0f, -vert[1]);
    m_earthlist[4][3].color = sColor(unlit);
    m_earthlist[4][3].coord = sCoord(tex[3], tex[3]);
    m_earthlist[4][3].vertex = sPosition(vert[1], 0.0f, vert[1]);
  }
}

// For building mountain sillohettes in sunset
void CWorld::makeHeights(int first, int last, int *h)
{
  int middle;
  int diff;

  diff = last - first;
  if (diff <= 1)
    return;
  middle = (first + last) / 2;
  h[middle] = (h[first] + h[last]) / 2;
  h[middle] += rsRandi(diff / 2) - (diff / 4);
  if (h[middle] < 1)
    h[middle] = 1;

  makeHeights(first, middle, h);
  makeHeights(middle, last, h);
}

void CWorld::update(float frameTime)
{
  const float recipHalfCloud = 1.0f / float(CLOUDMESH / 6);

  if (m_base->Settings().dClouds)
  {
    // Blow clouds (particles should get the same drift at 2000 feet)
    // Maximum m_base->Settings().dWind is 100 and texture repeats every 6666.67 feet
    // so 100 * 0.00015 * 6666.67 = 100 ft/sec maximum windspeed.
    m_cloudShift += 0.00015f * float(m_base->Settings().dWind) * frameTime;
    while(m_cloudShift > 1.0f)
      m_cloudShift -= 1.0f;
    for (int j = 0; j <= CLOUDMESH; ++j)
    {
      for (int i = 0; i <= CLOUDMESH; ++i)
      {
        // darken clouds
        m_clouds[i][j][6] = m_clouds[i][j][5];
        m_clouds[i][j][7] = m_clouds[i][j][5];
        m_clouds[i][j][8] = m_clouds[i][j][5];
        // move clouds
        m_clouds[i][j][3] = float(-i) * recipHalfCloud + m_cloudShift;
      }
    }
  }
}

void CWorld::draw()
{
  int i, j;

  glDisable(GL_DEPTH_TEST);

  // draw stars
  if (m_base->Settings().dStardensity)
  {
    glDisable(GL_BLEND);
    m_base->BindTexture(GL_TEXTURE_2D, m_startex);
    for (i = 0; i < m_starlistStripEntries; i++)
      m_base->DrawEntry(GL_TRIANGLE_STRIP, m_starlistStrip[i], m_starlistStripSize);
    m_base->DrawEntry(GL_TRIANGLE_FAN, m_starlistFan, m_starlistFanSize);
  }

  // draw moon
  if (m_base->Settings().dMoon)
  {
    glm::mat4& modelMatrix = m_base->ModelMatrix();
    glm::mat4 modelMatrixOld = modelMatrix;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(m_moonRotation), glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(m_moonHeight), glm::vec3(1, 0, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, -20000.0f));
    m_base->BindTexture(GL_TEXTURE_2D, m_moontex);
    m_base->DrawEntry(GL_TRIANGLE_STRIP, m_moonlist, 4);
    modelMatrix = modelMatrixOld;
  }

  // draw clouds
  if (m_base->Settings().dClouds)
  {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_base->BindTexture(GL_TEXTURE_2D, m_cloudtex);

    unsigned int ptr = 0;
    sLight cloudmeshs[(CLOUDMESH+1)*2];
    for (j = 0; j < CLOUDMESH; j++)
    {
      for (i = 0; i <= CLOUDMESH; i++)
      {
        cloudmeshs[ptr  ].color = sColor(m_clouds[i][j+1][6], m_clouds[i][j+1][7], m_clouds[i][j+1][8]);
        cloudmeshs[ptr  ].coord = sCoord(m_clouds[i][j+1][3], m_clouds[i][j+1][4]);
        cloudmeshs[ptr++].vertex = sPosition(m_clouds[i][j+1]);
        cloudmeshs[ptr  ].color = sColor(m_clouds[i][j][6], m_clouds[i][j][7], m_clouds[i][j][8]);
        cloudmeshs[ptr  ].coord = sCoord(m_clouds[i][j][3], m_clouds[i][j][4]);
        cloudmeshs[ptr++].vertex = sPosition(m_clouds[i][j]);
      }
      m_base->DrawEntry(GL_TRIANGLE_STRIP, cloudmeshs, ptr);
      ptr = 0;
    }
  }

  // draw sunset
  if (m_doSunset)
  {
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    m_base->BindTexture(GL_TEXTURE_2D, m_sunsettex);
    m_base->DrawEntry(GL_TRIANGLE_STRIP, m_sunsetlist, 18);
  }

  // draw moon's halo
  if (m_base->Settings().dMoonglow && m_base->Settings().dMoon)
  {
    float glow = float(m_base->Settings().dMoonglow) * 0.005f;  // half of max possible value
    glm::mat4& modelMatrix = m_base->ModelMatrix();
    glm::mat4 modelMatrixOld = modelMatrix;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    m_base->BindTexture(GL_TEXTURE_2D, m_moonglowtex);

    modelMatrix = glm::rotate(modelMatrix, glm::radians(m_moonRotation), glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(m_moonHeight), glm::vec3(1, 0, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, -20000.0f));
    m_moonglowlist[0].color = m_moonglowlist[1].color = m_moonglowlist[2].color = m_moonglowlist[3].color = sColor(1.0f, 1.0f, 1.0f, glow);
    m_base->DrawEntry(GL_TRIANGLE_STRIP, m_moonglowlist, 4);

    modelMatrix = glm::scale(modelMatrix, glm::vec3(6000.0f, 6000.0f, 6000.0f));
    //m_moonglowlist[0].color = m_moonglowlist[1].color = m_moonglowlist[2].color = m_moonglowlist[3].color = sColor(1.0f, 1.0f, 1.0f, glow * 0.7f);
    m_base->BindTexture(GL_TEXTURE_2D, m_base->Flare().Flares()[0].texture);
    m_base->DrawEntry(GL_TRIANGLE_STRIP, m_base->Flare().Flares()[0].light, 4);

    modelMatrix = modelMatrixOld;
  }

  // draw earth
  if (m_base->Settings().dEarth)
  {
    glm::mat4& modelMatrix = m_base->ModelMatrix();
    glm::mat4 modelMatrixOld = modelMatrix;

    glDisable(GL_BLEND);
    m_base->BindTexture(GL_TEXTURE_2D, m_earthneartex);
    m_base->DrawEntry(GL_TRIANGLE_STRIP, m_earthnearlist, 4);

    m_base->BindTexture(GL_TEXTURE_2D, m_earthfartex);
    for (unsigned int i = 0; i < 4; ++i)
      m_base->DrawEntry(GL_TRIANGLE_STRIP, m_earthfarlist[i], 4);

    if (m_base->Settings().dAmbient <= 25)
    {
      glEnable(GL_BLEND);
      glBlendFunc(GL_ONE, GL_ONE);
      m_base->BindTexture(GL_TEXTURE_2D, m_earthlighttex);
      for (unsigned int i = 0; i < 5; ++i)
        m_base->DrawEntry(GL_TRIANGLE_STRIP, m_earthlist[i], 4);
    }
    modelMatrix = modelMatrixOld;
  }
}
