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

#include "wavyNormalCubeMaps.h"
#include "mipmap.h"

#include <kodi/gui/gl/Texture.h>
#include <rsMath/rsMath.h>
#include <math.h>

CWavyNormalCubeMaps::CWavyNormalCubeMaps(int frames, int size)
  : m_numFrames(frames),
    m_texSize(size)
{
  int g, i, j;

  GLubyte* map = new GLubyte[m_texSize * m_texSize * 3];

  // allocate memory for pointers to texture objects
  m_texture = new GLuint[m_numFrames];
  glGenTextures(m_numFrames, m_texture);

  // calculate normal cube maps
  glm::vec3 vec;
  glm::vec3 norm;
  float offset = -0.5f * float(m_texSize) + 0.5f;
  for (g = 0; g < m_numFrames; g++)
  {
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture[g]);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_phase = RS_PIx2 * float(g) / float(m_numFrames);

    // left
    {
      for (i = 0; i < m_texSize; i++)
      {
        for (j = 0; j < m_texSize; j++)
        {
          vec[0] = -0.5f;
          vec[1] = -(float(j) + offset) / float(m_texSize);
          vec[2] = (float(i) + offset) / float(m_texSize);
          vec = glm::normalize(vec);
          WavyFunc(vec, norm);
          map[(i + j * m_texSize) * 3] = GLubyte(norm[0] * 127.999f + 128.0f);
          map[(i + j * m_texSize) * 3 + 1] = GLubyte(norm[1] * 127.999f + 128.0f);
          map[(i + j * m_texSize) * 3 + 2] = GLubyte(norm[2] * -127.999f + 128.0f);
        }
      }
      Build2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_RGB, m_texSize, m_texSize, GL_RGB, GL_UNSIGNED_BYTE, map);
    }

    // right
    for (i = 0; i < m_texSize; i++)
    {
      for (j = 0; j < m_texSize; j++)
      {
        vec[0] = 0.5f;
        vec[1] = -(float(j) + offset) / float(m_texSize);
        vec[2] = -(float(i) + offset) / float(m_texSize);
        vec = glm::normalize(vec);
        WavyFunc(vec, norm);
        map[(i + j * m_texSize) * 3] = GLubyte(norm[0] * 127.999f + 128.0f);
        map[(i + j * m_texSize) * 3 + 1] = GLubyte(norm[1] * 127.999f + 128.0f);
        map[(i + j * m_texSize) * 3 + 2] = GLubyte(norm[2] * -127.999f + 128.0f);
      }
    }
    Build2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_RGB, m_texSize, m_texSize, GL_RGB, GL_UNSIGNED_BYTE, map);

    // back
    for (i = 0; i < m_texSize; i++)
    {
      for (j = 0; j < m_texSize; j++)
      {
        vec[0] = -(float(i) + offset) / float(m_texSize);
        vec[1] = -(float(j) + offset) / float(m_texSize);
        vec[2] = -0.5f;
        vec = glm::normalize(vec);
        WavyFunc(vec, norm);
        map[(i + j * m_texSize) * 3] = GLubyte(norm[0] * 127.999f + 128.0f);
        map[(i + j * m_texSize) * 3 + 1] = GLubyte(norm[1] * 127.999f + 128.0f);
        map[(i + j * m_texSize) * 3 + 2] = GLubyte(norm[2] * -127.999f + 128.0f);
      }
    }
    Build2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_RGB, m_texSize, m_texSize, GL_RGB, GL_UNSIGNED_BYTE, map);

    // front
    for (i = 0; i < m_texSize; i++)
    {
      for (j = 0; j < m_texSize; j++)
      {
        vec[0] = (float(i) + offset) / float(m_texSize);
        vec[1] = -(float(j) + offset) / float(m_texSize);
        vec[2] = 0.5f;
        vec = glm::normalize(vec);
        WavyFunc(vec, norm);
        map[(i + j * m_texSize) * 3] = GLubyte(norm[0] * 127.999f + 128.0f);
        map[(i + j * m_texSize) * 3 + 1] = GLubyte(norm[1] * 127.999f + 128.0f);
        map[(i + j * m_texSize) * 3 + 2] = GLubyte(norm[2] * -127.999f + 128.0f);
      }
    }
    Build2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_RGB, m_texSize, m_texSize, GL_RGB, GL_UNSIGNED_BYTE, map);

    // bottom
    for (i = 0; i < m_texSize; i++)
    {
      for (j = 0; j < m_texSize; j++)
      {
        vec[0] = (float(i) + offset) / float(m_texSize);
        vec[1] = -0.5f;
        vec[2] = -(float(j) + offset) / float(m_texSize);
        vec = glm::normalize(vec);
        WavyFunc(vec, norm);
        map[(i + j * m_texSize) * 3] = GLubyte(norm[0] * 127.999f + 128.0f);
        map[(i + j * m_texSize) * 3 + 1] = GLubyte(norm[1] * 127.999f + 128.0f);
        map[(i + j * m_texSize) * 3 + 2] = GLubyte(norm[2] * -127.999f + 128.0f);
      }
    }
    Build2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_RGB, m_texSize, m_texSize, GL_RGB, GL_UNSIGNED_BYTE, map);

    // top
    for (i = 0; i < m_texSize; i++)
    {
      for (j = 0; j < m_texSize; j++)
      {
        vec[0] = (float(i) + offset) / float(m_texSize);
        vec[1] = 0.5f;
        vec[2] = (float(j) + offset) / float(m_texSize);
        vec = glm::normalize(vec);
        WavyFunc(vec, norm);
        map[(i + j * m_texSize) * 3] = GLubyte(norm[0] * 127.999f + 128.0f);
        map[(i + j * m_texSize) * 3 + 1] = GLubyte(norm[1] * 127.999f + 128.0f);
        map[(i + j * m_texSize) * 3 + 2] = GLubyte(norm[2] * -127.999f + 128.0f);
      }
    }
    Build2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_RGB, m_texSize, m_texSize, GL_RGB, GL_UNSIGNED_BYTE, map);
  }

  delete[] map;
}

CWavyNormalCubeMaps::~CWavyNormalCubeMaps()
{
  glDeleteTextures(m_numFrames, m_texture);
  delete m_texture;
}

void CWavyNormalCubeMaps::WavyFunc(const glm::vec3& point, glm::vec3& normal)
{
  normal = point;

  normal[0] += 0.2f * rsCosf((1.0f * point[0] + 4.0f  * point[1]) * M_PI + m_phase)
             + 0.1f * rsCosf((3.0f * point[1] + 13.0f * point[2]) * M_PI - m_phase);
  normal[1] += 0.2f * rsCosf((2.0f * point[1] - 5.0f  * point[2]) * M_PI + m_phase)
             + 0.1f * rsCosf((2.0f * point[2] + 12.0f * point[0]) * M_PI - m_phase);
  normal[2] += 0.2f * rsCosf((1.0f * point[2] + 6.0f  * point[0]) * M_PI + m_phase)
             + 0.1f * rsCosf((1.0f * point[0] - 11.0f * point[1]) * M_PI - m_phase);

  normal = glm::normalize(normal);
}
