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
 */

/*
 * Code is based on:
 *   https://github.com/reallyslickscreensavers/reallyslickscreensavers
 *   http://rss-glx.sourceforge.net/
 * and reworked to GL 4.0.
 */

#include "tunnel.h"
#include "main.h"
#include "splinePath.h"

#include <algorithm>
#include <kodi/gui/gl/GL.h>
#include <rsMath/rsMath.h>
#include <Rgbhsl/Rgbhsl.h>

CTunnel::CTunnel(CScreensaverHyperspace* base, CSplinePath* sp, int res)
  : m_base(base),
    m_path(sp),
    m_resolution(res)
{
  m_radius = 0.1f;
  m_widthOffset = 0.0f;
  m_texSpin = 0.0f;

  m_numSections = m_path->NumPoints() - 5;
  m_section = 0;

  m_v = new float***[m_numSections];
  m_t = new float***[m_numSections];
  m_c = new float***[m_numSections];
  for (int i = 0; i < m_numSections; i++)
  {
    m_v[i] = new float**[m_resolution+1];
    m_t[i] = new float**[m_resolution+1];
    m_c[i] = new float**[m_resolution+1];
    for (int j = 0; j <= m_resolution; j++)
    {
      m_v[i][j] = new float*[m_resolution+1];
      m_t[i][j] = new float*[m_resolution+1];
      m_c[i][j] = new float*[m_resolution+1];
      for (int k=0; k<=m_resolution; k++)
      {
        m_v[i][j][k] = new float[3];
        m_t[i][j][k] = new float[2];
        m_c[i][j][k] = new float[3];
      }
    }
  }

  m_huelo = 0.0f;
  m_huehi = 0.0f;
  m_satlo = 0.0f;
  m_sathi = 0.0f;
  m_lumlo = 3.14f;  // tunnel will be invisible at first
  m_lumhi = 3.14f;
}

CTunnel::~CTunnel()
{
  for (int i = 0; i < m_numSections; i++)
  {
    for (int j = 0; j <= m_resolution; j++)
    {
      for (int k=0; k<=m_resolution; k++)
      {
        delete[] m_v[i][j][k];
        delete[] m_t[i][j][k];
        delete[] m_c[i][j][k];
      }
      delete[] m_v[i][j];
      delete[] m_t[i][j];
      delete[] m_c[i][j];
    }
    delete[] m_v[i];
    delete[] m_t[i];
    delete[] m_c[i];
  }

  delete[] m_v;
  delete[] m_t;
  delete[] m_c;
}

void CTunnel::Make(float frameTime)
{
  int i, j, k;

  m_widthOffset += frameTime * 1.5f;
  while(m_widthOffset >= RS_PIx2)
    m_widthOffset -= RS_PIx2;
  m_texSpin += frameTime * 0.1f;
  while(m_texSpin >= RS_PIx2)
    m_texSpin -= RS_PIx2;

  m_huelo += frameTime * 0.04f;
  m_huehi += frameTime * 0.15f;
  m_satlo += frameTime * 0.04f;
  m_sathi += frameTime;
  m_lumlo += frameTime * 0.04f;
  m_lumhi += frameTime * 0.5f;
  while(m_huelo >= RS_PIx2)
    m_huelo -= RS_PIx2;
  while(m_huehi >= RS_PIx2)
    m_huehi -= RS_PIx2;
  while(m_satlo >= RS_PIx2)
    m_satlo -= RS_PIx2;
  while(m_sathi >= RS_PIx2)
    m_sathi -= RS_PIx2;
  while(m_lumlo >= RS_PIx2)
    m_lumlo -= RS_PIx2;
  while(m_lumhi >= RS_PIx2)
    m_lumhi -= RS_PIx2;

  float angle;
  glm::vec3 pos;
  float dir[3];
  rsVec vert;
  rsMatrix rotMat;
  float hsl[3];
  const float texcoordmult = 1.0f;
  const float max_saturation = 0.1f ;
  for (k = 0; k < m_numSections; k++)
  {
    // create new vertex data for this section
    for (i = 0; i <= m_resolution; i++)
    {
      angle = 0.0f;
      const float whereOnPath(float(i) / float(m_resolution));
      m_path->GetPoint(k+2, whereOnPath, pos);
      m_path->GetDirection(k+2, whereOnPath, dir);
      // z component of rotation matrix points in direction of path
      rotMat.m[8] = -dir[0];
      rotMat.m[9] = -dir[1];
      rotMat.m[10] = -dir[2];
      // x component of rotation matrix = cross product of (0,1,0) and z component
      rotMat.m[0] = rotMat.m[10];
      rotMat.m[1] = 0.0f;
      rotMat.m[2] = -rotMat.m[8];
      // y component of rotation matrix = cross product of z component and x component
      rotMat.m[4] = rotMat.m[9] * rotMat.m[2] - rotMat.m[10] * rotMat.m[1];
      rotMat.m[5] = rotMat.m[10] * rotMat.m[0] - rotMat.m[8] * rotMat.m[2];
      rotMat.m[6] = rotMat.m[8] * rotMat.m[1] - rotMat.m[9] * rotMat.m[0];
      for (j = 0; j <= m_resolution; j++)
      {
        angle = float(j) * RS_PIx2 / float(m_resolution);
        vert[0] = (m_radius + m_radius * 0.5f * rsCosf(2.0f * pos.x + m_widthOffset)) * rsCosf(angle);
        vert[1] = (m_radius + m_radius * 0.5f * rsCosf(pos.z + m_widthOffset)) * rsSinf(angle);
        vert[2] = 0.0f;
        vert.transVec(rotMat);
        // set vertex coordinates
        m_v[k][i][j][0] = pos.x + vert[0];
        m_v[k][i][j][1] = pos.y + vert[1];
        m_v[k][i][j][2] = pos.z + vert[2];
        // set texture coordinates
        m_t[k][i][j][0] = texcoordmult * float(i) / float(m_resolution);
        m_t[k][i][j][1] = texcoordmult * float(j) / float(m_resolution) + rsCosf(m_texSpin);
        // set colors
        hsl[0] = 2.0f * rsCosf(0.1f * m_v[k][i][j][0] + m_huelo) - 1.0f;
        hsl[1] = 0.25f * (rsCosf(0.013f * m_v[k][i][j][1] - m_satlo)
          + rsCosf(m_v[k][i][j][2] + m_sathi) + 2.0f);
        hsl[2] = 2.0f * rsCosf(0.01f * m_v[k][i][j][2] + m_lumlo)
          + rsCosf(0.4f * m_v[k][i][j][0] - m_lumhi)
          + 0.3f * rsCosf(4.0f * (m_v[k][i][j][0] + m_v[k][i][j][1] + m_v[k][i][j][2]));
        while(hsl[0] < 0.0f)
          hsl[0] += 1.0f;
        while(hsl[0] > 1.0f)
          hsl[0] -= 1.0f;
        if(hsl[1] < 0.0f)
          hsl[1] = 0.0f;
        if(hsl[1] > max_saturation)
          hsl[1] = max_saturation;
        if(hsl[2] < 0.0f)
          hsl[2] = 0.0f;
        if(hsl[2] > 1.0f)
          hsl[2] = 1.0f;
        hsl2rgb(hsl[0], hsl[1], hsl[2], m_c[k][i][j][0], m_c[k][i][j][1], m_c[k][i][j][2]);
      }
    }
  }
}

// specialty drawing function for when using shaders.
// lerp goes into color alpha and determines blend between
// texture frames.
void CTunnel::Draw(float lerp)
{
  unsigned int ptr = 0;
  m_lights.resize((m_resolution+1)*2);

  for (int k = 0; k < m_numSections; k++)
  {
    for (int i = 0; i < m_resolution; i++)
    {
      for (int j = 0; j <= m_resolution; j++)
      {
        m_lights[ptr  ].color = sColor(m_c[k][i+1][j][0], m_c[k][i+1][j][1], m_c[k][i+1][j][2], lerp);
        m_lights[ptr  ].coord = sCoord(m_t[k][i+1][j]);
        m_lights[ptr++].vertex = sPosition(m_v[k][i+1][j]);
        m_lights[ptr  ].color = sColor(m_c[k][i][j][0], m_c[k][i][j][1], m_c[k][i][j][2], lerp);
        m_lights[ptr  ].coord = sCoord(m_t[k][i][j]);
        m_lights[ptr++].vertex = sPosition(m_v[k][i][j]);
      }
      m_base->Draw(GL_TRIANGLE_STRIP, m_lights.data(), ptr);
      ptr = 0;
    }
  }
}
