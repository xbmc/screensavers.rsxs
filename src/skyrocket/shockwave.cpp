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

#include <kodi/gui/gl/GL.h>
#include <math.h>

#include "shockwave.h"
#include "main.h"

void CShockwave::Init()
{
  m_shockwavegeom[0][0][0] = 1.0f;
  m_shockwavegeom[0][0][1] = 0.0f;
  m_shockwavegeom[0][0][2] = 0.0f;
  m_shockwavegeom[1][0][0] = 0.985f;
  m_shockwavegeom[1][0][1] = 0.035f;
  m_shockwavegeom[1][0][2] = 0.0f;
  m_shockwavegeom[2][0][0] = 0.95f;
  m_shockwavegeom[2][0][1] = 0.05f;
  m_shockwavegeom[2][0][2] = 0.0f;
  m_shockwavegeom[3][0][0] = 0.85f;
  m_shockwavegeom[3][0][1] = 0.05f;
  m_shockwavegeom[3][0][2] = 0.0f;
  m_shockwavegeom[4][0][0] = 0.75f;
  m_shockwavegeom[4][0][1] = 0.035f;
  m_shockwavegeom[4][0][2] = 0.0f;
  m_shockwavegeom[5][0][0] = 0.65f;
  m_shockwavegeom[5][0][1] = 0.01f;
  m_shockwavegeom[5][0][2] = 0.0f;
  m_shockwavegeom[6][0][0] = 0.5f;
  m_shockwavegeom[6][0][1] = 0.0f;
  m_shockwavegeom[6][0][2] = 0.0f;

  float ch, sh;
  for (int i = 1; i <= WAVESTEPS; i++)
  {
    ch = cosf(6.28318530718f * (float(i) / float(WAVESTEPS)));
    sh = sinf(6.28318530718f * (float(i) / float(WAVESTEPS)));
    for (int j = 0; j <= 6; j++)
    {
      m_shockwavegeom[j][i][0] = ch * m_shockwavegeom[j][0][0];
      m_shockwavegeom[j][i][1] = m_shockwavegeom[j][0][1];
      m_shockwavegeom[j][i][2] = sh * m_shockwavegeom[j][0][0];
    }
  }
}

// temp influences color intensity (0.0 - 1.0)
// texmove is amount to advance the texture coordinates
void CShockwave::Draw(float temperature, float texmove)
{
  int i, j;
  float u, v1, v2;

  // setup diminishing alpha values in color array
  float temp = temperature * temperature;
  m_colors[0][3] = temp;
  m_colors[1][3] = temp * 0.9f;
  m_colors[2][3] = temp * 0.8f;
  m_colors[3][3] = temp * 0.7f;
  m_colors[4][3] = temp * 0.5f;
  m_colors[5][3] = temp * 0.3f;
  m_colors[6][3] = 0.0f;
  // setup rgb values in color array
  for (i = 0; i <= 5; i++)
  {
    m_colors[i][0] = 1.0f;
    m_colors[i][1] = (temperature + 1.0f) * 0.5f;
    m_colors[i][2] = temperature;
  }

  glDisable(GL_CULL_FACE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);
  m_base->BindTexture(GL_TEXTURE_2D, m_base->World().CloudTex());

  // draw bottom of shockwave
  unsigned int ptr = 0;
  for (i = 0; i < 6; i++)
  {
    v1 = float(i+1) * 0.07f - texmove;
    v2 = float(i) * 0.07f - texmove;
    for (j = 0; j <= WAVESTEPS; j++)
    {
      u = (float(j) / float(WAVESTEPS)) * 10.0f;
      m_wavesteps[ptr  ].color = sColor(m_colors[i+1]);
      m_wavesteps[ptr  ].coord = sCoord(u, v1);
      m_wavesteps[ptr++].vertex = sPosition(m_shockwavegeom[i+1][j][0], -m_shockwavegeom[i+1][j][1], m_shockwavegeom[i+1][j][2]);
      m_wavesteps[ptr  ].color = sColor(m_colors[i]);
      m_wavesteps[ptr  ].coord = sCoord(u, v2);
      m_wavesteps[ptr++].vertex = sPosition(m_shockwavegeom[i][j][0], -m_shockwavegeom[i][j][1], m_shockwavegeom[i][j][2]);
    }

    m_base->DrawEntry(GL_TRIANGLE_STRIP, m_wavesteps, ptr);
    ptr = 0;
  }

  // keep colors a little warmer on top (more green)
  for (i = 1; i <= 6; i++)
    m_colors[i][1] = (temperature + 2.0f) * 0.333333f;

  // draw top of shockwave
  for (i = 0; i < 6; i++)
  {
    v1 = float(i) * 0.07f - texmove;
    v2 = float(i+1) * 0.07f - texmove;
    for (j = 0; j <= WAVESTEPS; j++)
    {
      u = (float(j) / float(WAVESTEPS)) * 10.0f;
      m_wavesteps[ptr  ].color = sColor(m_colors[i]);
      m_wavesteps[ptr  ].coord = sCoord(u, v1);
      m_wavesteps[ptr++].vertex = sPosition(m_shockwavegeom[i][j][0], m_shockwavegeom[i][j][1], m_shockwavegeom[i][j][2]);
      m_wavesteps[ptr  ].color = sColor(m_colors[i+1]);
      m_wavesteps[ptr  ].coord = sCoord(u, v2);
      m_wavesteps[ptr++].vertex = sPosition(m_shockwavegeom[i+1][j][0], m_shockwavegeom[i+1][j][1], m_shockwavegeom[i+1][j][2]);
    }

    m_base->DrawEntry(GL_TRIANGLE_STRIP, m_wavesteps, ptr);
    ptr = 0;
  }

  glEnable(GL_CULL_FACE);
}
