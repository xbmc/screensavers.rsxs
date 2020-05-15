/*
 *  Copyright (C) 2005-2020 Team Kodi (https://kodi.tv)
 *  Copyright (C) 1999-2010 Terence M. Welsh
 *  Ported to Kodi by Alwin Esch <alwinus@kodi.tv>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

/*
 * Code is based on:
 *   https://github.com/reallyslickscreensavers/reallyslickscreensavers
 *   http://rss-glx.sourceforge.net/
 * and reworked to GL 4.0.
 */

#include "main.h"

#include <string.h>
#include <math.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <rsMath/rsMath.h>

#define PIx2 6.28318530718f

bool CScreensaverPlasma::Start()
{
  int speed = kodi::GetSettingInt("speed");
  m_zoom = kodi::GetSettingInt("zoom");
  m_focus = float(kodi::GetSettingInt("focus")) / 50.0f + 0.3f;
  m_maxdiff = 0.004f * float(speed);
  m_resolution = kodi::GetSettingInt("resolution");
  m_aspectRatio = float(Width()) / float(Height());

  std::string fraqShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/frag.glsl");
  std::string vertShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
    return false;

  glGenBuffers(1, &m_vertexVBO);

  // Initialize pseudorandom number generator
  srand((unsigned)time(nullptr));

  // Initialize constants
  for (int i = 0; i < NUMCONSTS; i++)
  {
    m_ct[i] = rsRandf(PIx2);
    m_cv[i] = rsRandf(0.005f * float(speed)) + 0.0001f;
  }

  SetPlasmaSize();

  return true;
}

void CScreensaverPlasma::Stop()
{
  glDeleteBuffers(1, &m_vertexVBO);
  m_vertexVBO = 0;
}

void CScreensaverPlasma::Render()
{
  glDisable(GL_BLEND);

  int i, j;
  float rgb[3];
  float temp;
  int index;

  //Update constants
  for (i = 0; i < NUMCONSTS; i++)
  {
    m_ct[i] += m_cv[i];
    if(m_ct[i] > PIx2)
      m_ct[i] -= PIx2;
    m_c[i] = sinf(m_ct[i]) * m_focus;
  }

  // Update colors
  for(i = 0; i < m_plasmasize; i++)
  {
    for(j = 0; j < int(float(m_plasmasize) / m_aspectRatio); j++)
    {
      // Calculate vertex colors
      rgb[0] = m_plasma[i][j][0];
      rgb[1] = m_plasma[i][j][1];
      rgb[2] = m_plasma[i][j][2];
      m_plasma[i][j][0] = 0.7f
              * (m_c[0] * m_position[i][j][0] + m_c[1] * m_position[i][j][1]
              + m_c[2] * (m_position[i][j][0] * m_position[i][j][0] + 1.0f)
              + m_c[3] * m_position[i][j][0] * m_position[i][j][1]
              + m_c[4] * rgb[1] + m_c[5] * rgb[2]);
      m_plasma[i][j][1] = 0.7f
              * (m_c[6] * m_position[i][j][0] + m_c[7] * m_position[i][j][1]
              + m_c[8] * m_position[i][j][0] * m_position[i][j][0]
              + m_c[9] * (m_position[i][j][1] * m_position[i][j][1] - 1.0f)
              + m_c[10] * rgb[0] + m_c[11] * rgb[2]);
      m_plasma[i][j][2] = 0.7f
              * (m_c[12] * m_position[i][j][0] + m_c[13] * m_position[i][j][1]
              + m_c[14] * (1.0f - m_position[i][j][0] * m_position[i][j][1])
              + m_c[15] * m_position[i][j][1] * m_position[i][j][1]
              + m_c[16] * rgb[0] + m_c[17] * rgb[1]);

      // Don't let the colors change too much
      temp = m_plasma[i][j][0] - rgb[0];
      if(temp > m_maxdiff)
        m_plasma[i][j][0] = rgb[0] + m_maxdiff;
      if(temp < -m_maxdiff)
        m_plasma[i][j][0] = rgb[0] - m_maxdiff;
      temp = m_plasma[i][j][1] - rgb[1];
      if(temp > m_maxdiff)
        m_plasma[i][j][1] = rgb[1] + m_maxdiff;
      if(temp < -m_maxdiff)
        m_plasma[i][j][1] = rgb[1] - m_maxdiff;
      temp = m_plasma[i][j][2] - rgb[2];
      if(temp > m_maxdiff)
        m_plasma[i][j][2] = rgb[2] + m_maxdiff;
      if(temp < -m_maxdiff)
        m_plasma[i][j][2] = rgb[2] - m_maxdiff;

      // Put colors into texture
      index = (i * TEXSIZE + j) * 3;
      m_plasmamap[index] = fabstrunc(m_plasma[i][j][0]);
      m_plasmamap[index+1] = fabstrunc(m_plasma[i][j][1]);
      m_plasmamap[index+2] = fabstrunc(m_plasma[i][j][2]);
    }
  }

  // Update texture
  if (!glIsTexture(m_tex))
  {
    glGenTextures(1, &m_tex);
    glBindTexture(GL_TEXTURE_2D, m_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, TEXSIZE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXSIZE, TEXSIZE, 0, GL_RGB, GL_FLOAT, m_plasmamap);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  }
  else
  {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, TEXSIZE);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, int(float(m_plasmasize) / m_aspectRatio), m_plasmasize, GL_RGB, GL_FLOAT, m_plasmamap);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  }

  // Draw it
  // The "- 1" cuts off right and top edges to get rid of blending to black
  float texright = float(m_plasmasize - 1) / float(TEXSIZE);
  float textop = float(int(float(m_plasmasize) / m_aspectRatio) - 1) / float(TEXSIZE);

  struct PackedVertex
  {
    float x, y, z;
    float u1, v1;
  } packets[4];

  packets[0].x = -1.0f;
  packets[0].y = -1.0f;
  packets[0].z = 0.0f;
  packets[0].u1 = 0.0f;
  packets[0].v1 = 0.0f;

  packets[1].x = 1.0f;
  packets[1].y = -1.0f;
  packets[1].z = 0.0f;
  packets[1].u1 = 0.0f;
  packets[1].v1 = texright;

  packets[2].x = -1.0f;
  packets[2].y = 1.0f;
  packets[2].z = 0.0f;
  packets[2].u1 = textop;
  packets[2].v1 = 0.0f;

  packets[3].x = 1.0f;
  packets[3].y = 1.0f;
  packets[3].z = 0.0f;
  packets[3].u1 = textop;
  packets[3].v1 = texright;

  EnableShader();

  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(PackedVertex)*4, &packets[0], GL_STATIC_DRAW);

  glVertexAttribPointer(m_hPos,  3, GL_FLOAT, 0, sizeof(PackedVertex), BUFFER_OFFSET(offsetof(PackedVertex, x)));
  glEnableVertexAttribArray(m_hPos);

  glVertexAttribPointer(m_hCord, 2, GL_FLOAT, 0, sizeof(PackedVertex), BUFFER_OFFSET(offsetof(PackedVertex, u1)));
  glEnableVertexAttribArray(m_hCord);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glDisableVertexAttribArray(m_hPos);
  glDisableVertexAttribArray(m_hCord);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  DisableShader();

  glBindTexture(GL_TEXTURE_2D, 0);
}

void CScreensaverPlasma::SetPlasmaSize()
{
  if (m_aspectRatio >= 1.0f)
  {
    m_width = 30.0f / float(m_zoom);
    m_height = m_width / m_aspectRatio;
  }
  else
  {
    m_height = 30.0f / float(m_zoom);
    m_width = m_height * m_aspectRatio;
  }

  // Set resolution of plasma
  if (m_aspectRatio >= 1.0f)
    m_plasmasize = int(float(m_resolution * TEXSIZE) * 0.01f);
  else
    m_plasmasize = int(float(m_resolution * TEXSIZE) * m_aspectRatio * 0.01f);

  for (int i=0; i<m_plasmasize; i++)
  {
    for (int j=0; j<int(float(m_plasmasize) / m_aspectRatio); j++)
    {
      m_plasma[i][j][0] = 0.0f;
      m_plasma[i][j][1] = 0.0f;
      m_plasma[i][j][2] = 0.0f;
      m_position[i][j][0] = float(i * m_width) / float(m_plasmasize - 1) - (m_width * 0.5f);
      m_position[i][j][1] = float(j * m_height) / (float(m_plasmasize) / m_aspectRatio - 1.0f) - (m_height * 0.5f);
    }
  }
}

void CScreensaverPlasma::OnCompiledAndLinked()
{
  // Variables passed directly to the Vertex shader
  m_hPos = glGetAttribLocation(ProgramHandle(), "m_attrpos");
  m_hCord = glGetAttribLocation(ProgramHandle(), "m_attrcord");
}

ADDONCREATOR(CScreensaverPlasma);
