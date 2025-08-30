/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2009 Tugrul Galatali <tugrul@galatali.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

/*
 * Code is based on:
 *   http://rss-glx.sourceforge.net/
 * and reworked to GL 4.0.
 */

#include "main.h"

#include <algorithm>
#include <chrono>
#include <kodi/gui/General.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <rsMath/rsMath.h>
#include <Rgbhsl/Rgbhsl.h>

#define BUFFER_OFFSET(i) ((char *)nullptr + (i))

namespace
{
struct sFeedbackSettings
{
  void init()
  {
    kodi::addon::CheckSettingBoolean("general.grey", dGrey);
    kodi::addon::CheckSettingFloat("general.saturation", dSaturation);
    kodi::addon::CheckSettingFloat("general.lightness", dLightness);
    kodi::addon::CheckSettingBoolean("general.grid", dGrid);
    kodi::addon::CheckSettingInt("general.period", dPeriod);
    kodi::addon::CheckSettingInt("general.texsize", dTexSize);
    kodi::addon::CheckSettingFloat("general.speed", dSpeed);

    int cells;
    if (kodi::addon::CheckSettingInt("general.cells", cells))
      cwidth = cheight = 1 << cells;
  }

  bool dGrey = false;
  float dSaturation = 1.0f;
  float dLightness = 1.0f;
  bool dGrid = false;
  int dPeriod = 5;
  int dTexSize = 8;
  float dSpeed = 1;
  unsigned int cwidth = 8, cheight = 8;
} gSettings;
} /* namespace */

bool CScreensaverFeedback::Start()
{
  gSettings.init();

  std::string fraqShader = kodi::addon::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/frag.glsl");
  std::string vertShader = kodi::addon::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
    return false;

  // Initialize pseudorandom number generator
  srand((unsigned)time(nullptr));

  {
    m_width = m_height = 1 << gSettings.dTexSize;
    int newTexSize = gSettings.dTexSize;
    while ((m_width > Width()) || (m_height > Height()))
    {
      --newTexSize;
      m_width = m_width >> 1;
      m_height = m_height >> 1;
    }

    if (newTexSize != gSettings.dTexSize)
    {
      kodi::Log(ADDON_LOG_INFO, "Texture size reduced to %d from %d to fit display", newTexSize, gSettings.dTexSize);
      gSettings.dTexSize = newTexSize;
    }

    uint8_t *pixels = new uint8_t[m_width * m_height * 3];
    for (int hh = 0, ii = 0; hh < m_height; ++hh)
    {
      for (int ww = 0; ww < m_width; ++ww)
      {
        float r, g, b;

        if (gSettings.dGrey)
          hsl2rgb(0.0f, 0.0f, hh * ww / float(m_height * m_width), r, g, b);
        else
          hsl2rgb(hh / float(m_height), gSettings.dSaturation, gSettings.dLightness, r, g, b);

        pixels[ii++] = static_cast<uint8_t>(r * 255);
        pixels[ii++] = static_cast<uint8_t>(g * 255);
        pixels[ii++] = static_cast<uint8_t>(b * 255);
      }
    }

    glGenTextures(1, &m_texture);
    BindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    delete [] pixels;
  }

  m_displacements = new rsVec[gSettings.cwidth * gSettings.cheight];
  m_velocities = new rsVec[gSettings.cwidth * gSettings.cheight];
  m_accelerations = new rsVec[gSettings.cwidth * gSettings.cheight];
  m_framedTextures = new sLight[gSettings.cwidth * gSettings.cheight * 10];

  for (unsigned int hh = 0, ii = 0; hh < gSettings.cheight; ++hh)
  {
    for (unsigned int ww = 0; ww < gSettings.cwidth; ++ww)
    {
      m_displacements[ii][0] = rsRandf(0.5f) - 0.25f;
      m_displacements[ii][1] = rsRandf(0.5f) - 0.25f;
      m_displacements[ii][2] = 0.0f;
      m_velocities[ii] = rsVec(0.0f, 0.0f, 0.0f);
      m_accelerations[ii] = rsVec(0.0f, 0.0f, 0.0f);
      ++ii;
    }
  }

  // Window initialization
  glViewport(X(), Y(), Width(), Height());

  glGenVertexArrays(1, &m_vao);

  glGenBuffers(1, &m_vertexVBO);
  glGenBuffers(1, &m_indexVBO);

  m_rotatingColor[0].color = glm::vec3(1.0f, 1.0f, 1.0f);
  m_rotatingColor[0].vertex = glm::vec3(0.0f, 1.0f, 0.0f);
  m_rotatingColor[0].coord = glm::vec2(0.0f, 1.0f);
  m_rotatingColor[1].color = glm::vec3(1.0f, 1.0f, 1.0f);
  m_rotatingColor[1].vertex = glm::vec3(1.0f, 1.0f, 0.0f);
  m_rotatingColor[1].coord = glm::vec2(1.0f, 1.0f);
  m_rotatingColor[2].color = glm::vec3(1.0f, 1.0f, 1.0f);
  m_rotatingColor[2].vertex = glm::vec3(1.0f, 0.0f, 0.0f);
  m_rotatingColor[2].coord = glm::vec2(1.0f, 0.0f);
  m_rotatingColor[3].color = glm::vec3(1.0f, 1.0f, 1.0f);
  m_rotatingColor[3].vertex = glm::vec3(0.0f, 0.0f, 0.0f);
  m_rotatingColor[3].coord = glm::vec2(0.0f, 0.0f);

  m_lastTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  m_startOK = true;
  return true;
}

void CScreensaverFeedback::Stop()
{
  if (!m_startOK)
    return;

  m_startOK = false;

  glDeleteBuffers(1, &m_vertexVBO);
  m_vertexVBO = 0;
  glDeleteBuffers(1, &m_indexVBO);
  m_indexVBO = 0;
  glDeleteTextures(1, &m_texture);
  m_texture = 0;

  delete[] m_displacements;
  delete[] m_velocities;
  delete[] m_accelerations;
  delete[] m_framedTextures;

  glDeleteVertexArrays(1, &m_vao);
}

void CScreensaverFeedback::Render()
{
  if (!m_startOK)
    return;

  /*
   * Following Extra work done here in render to prevent problems with controls
   * from Kodi and during window moving.
   * TODO: Maybe add a separate interface call to inform about?
   */
  //@{
  glBindVertexArray(m_vao);

  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);
  BindTexture(GL_TEXTURE_2D, m_texture);

  glVertexAttribPointer(m_hVertex, 3, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, vertex)));
  glEnableVertexAttribArray(m_hVertex);

  glVertexAttribPointer(m_hColor, 3, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, color)));
  glEnableVertexAttribArray(m_hColor);

  glVertexAttribPointer(m_hCoord, 2, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, coord)));
  glEnableVertexAttribArray(m_hCoord);
  //@}

  // ################################################################################
  // Frame texture with a rotating color

  double currentTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  float frameTime = static_cast<float>(currentTime - m_lastTime);
  m_lastTime = currentTime;

  glViewport(0, 0, m_width, m_height);
  m_projMat = glm::ortho(-0.125f, 1.125f, -0.125f, 1.125f);
  m_modelMat = glm::mat4(1.0f);

  {
    float h, s, l;
    if (gSettings.dGrey)
    {
      h = 0.0f;
      s = 0.0f;
      l = static_cast<float>(currentTime / gSettings.dPeriod - trunc(currentTime / gSettings.dPeriod));

      l = l * 2.0f;

      if (l > 1.0f)
        l = 2.0f - l;
    }
    else
    {
      h = static_cast<float>(currentTime / gSettings.dPeriod - trunc(currentTime / gSettings.dPeriod));
      s = gSettings.dSaturation;
      l = gSettings.dLightness;
    }

    float r, g, b;

    hsl2rgb(h, s, l, r, g, b);

    glClearColor(r, g, b, 1.0f);
  }

  glClear(GL_COLOR_BUFFER_BIT);

  EnableShader();
  glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*4, m_rotatingColor, GL_STATIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte)*4, m_rotatingColorIdx, GL_STATIC_DRAW);
  glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, 0);
  DisableShader();

  glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, m_width, m_height, 0);

  // ################################################################################
  // Warp framed texture

  glViewport(0, 0, m_width, m_height);

  m_projMat = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);
  m_modelMat = glm::mat4(1.0f);
  int ptr = 0;
  sLight framedTextures[4];
  for (unsigned int dh = 0; dh < gSettings.cheight; ++dh)
  {
    for (unsigned int dw = 0; dw < gSettings.cwidth; ++dw)
    {
      const rsVec da = rsVec(dw / float(gSettings.cwidth),       dh / float(gSettings.cheight),       0.0f);
      const rsVec db = rsVec(dw / float(gSettings.cwidth),       (dh + 1) / float(gSettings.cheight), 0.0f);
      const rsVec dc = rsVec((dw + 1) / float(gSettings.cwidth), (dh + 1) / float(gSettings.cheight), 0.0f);
      const rsVec dd = rsVec((dw + 1) / float(gSettings.cwidth), dh / float(gSettings.cheight),       0.0f);

      const unsigned nh = (dh + 1) & (gSettings.cheight - 1);
      const unsigned nw = (dw + 1) & (gSettings.cwidth - 1);
      const rsVec sa = m_displacements[dh * gSettings.cwidth + dw] + rsVec(float(dw),     float(dh), 0.0f);
      const rsVec sb = m_displacements[nh * gSettings.cwidth + dw] + rsVec(float(dw),     float(dh + 1), 0.0f);
      const rsVec sc = m_displacements[nh * gSettings.cwidth + nw] + rsVec(float(dw + 1), float(dh + 1), 0.0f);
      const rsVec sd = m_displacements[dh * gSettings.cwidth + nw] + rsVec(float(dw + 1), float(dh), 0.0f);

      framedTextures[0].color = glm::vec3(1.0f, 1.0f, 1.0f);
      framedTextures[0].vertex = glm::vec3(da[0], da[1], 0.0f);
      framedTextures[0].coord = glm::vec2(sa[0] / gSettings.cwidth * 0.8f + 0.1f, sa[1] / gSettings.cheight * 0.8f + 0.1f);

      framedTextures[1].color = glm::vec3(1.0f, 1.0f, 1.0f);
      framedTextures[1].vertex = glm::vec3(db[0], db[1], 0.0f);
      framedTextures[1].coord = glm::vec2(sb[0] / gSettings.cwidth * 0.8f + 0.1f, sb[1] / gSettings.cheight * 0.8f + 0.1f);

      framedTextures[2].color = glm::vec3(1.0f, 1.0f, 1.0f);
      framedTextures[2].vertex = glm::vec3(dc[0], dc[1], 0.0f);
      framedTextures[2].coord = glm::vec2(sc[0] / gSettings.cwidth * 0.8f + 0.1f, sc[1] / gSettings.cheight * 0.8f + 0.1f);

      framedTextures[3].color = glm::vec3(1.0f, 1.0f, 1.0f);
      framedTextures[3].vertex = glm::vec3(dd[0], dd[1], 0.0f);
      framedTextures[3].coord = glm::vec2(sd[0] / gSettings.cwidth * 0.8f + 0.1f, sd[1] / gSettings.cheight * 0.8f + 0.1f);

      EnableShader();
      glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*4, framedTextures, GL_DYNAMIC_DRAW);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte)*4, m_rotatingColorIdx, GL_STATIC_DRAW);
      glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, 0);
      DisableShader();
    }
  }

  glCopyTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, 0, 0, m_width, m_height, 0);

  // ################################################################################
  // Render warped texture to screen

  glViewport(X(), Y(), Width(), Height());

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  EnableShader();
  glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*4, m_rotatingColor, GL_STATIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte)*4, m_rotatingColorIdx, GL_STATIC_DRAW);
  glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, 0);
  DisableShader();

  // ################################################################################
  // Optionally display warping grid

  if (gSettings.dGrid)
  {
    int ptr = 0;

    BindTexture(GL_TEXTURE_2D, 0);

    for (unsigned int dh = 0; dh < gSettings.cheight; ++dh)
    {
      for (unsigned int dw = 0; dw < gSettings.cwidth; ++dw)
      {
        const unsigned nh = (dh + 1) & (gSettings.cheight - 1);
        const unsigned nw = (dw + 1) & (gSettings.cwidth - 1);
        const rsVec a = m_displacements[dh * gSettings.cwidth + dw] + rsVec(float(dw),     float(dh), 0.0f);
        const rsVec b = m_displacements[nh * gSettings.cwidth + dw] + rsVec(float(dw),     float(dh + 1), 0.0f);
        const rsVec c = m_displacements[nh * gSettings.cwidth + nw] + rsVec(float(dw + 1), float(dh + 1), 0.0f);
        const rsVec d = m_displacements[dh * gSettings.cwidth + nw] + rsVec(float(dw + 1), float(dh), 0.0f);

        m_framedTextures[ptr  ].color = glm::vec3(0.0f, 1.0f, 0.0f);;
        m_framedTextures[ptr++].vertex = glm::vec3(float(dw) / gSettings.cwidth, float(dh) / gSettings.cheight, 0.0f);
        m_framedTextures[ptr  ].color = glm::vec3(0.0f, 1.0f, 0.0f);;
        m_framedTextures[ptr++].vertex = glm::vec3(a[0] / gSettings.cwidth, a[1] / gSettings.cheight, 0.0f);

        m_framedTextures[ptr  ].color = glm::vec3(1.0f, 0.0f, 0.0f);;
        m_framedTextures[ptr++].vertex = glm::vec3(a[0] / gSettings.cwidth, a[1] / gSettings.cheight, 0.0f);
        m_framedTextures[ptr  ].color = glm::vec3(1.0f, 0.0f, 0.0f);;
        m_framedTextures[ptr++].vertex = glm::vec3(b[0] / gSettings.cwidth, b[1] / gSettings.cheight, 0.0f);

        m_framedTextures[ptr  ].color = glm::vec3(1.0f, 0.0f, 0.0f);;
        m_framedTextures[ptr++].vertex = glm::vec3(b[0] / gSettings.cwidth, b[1] / gSettings.cheight, 0.0f);
        m_framedTextures[ptr  ].color = glm::vec3(1.0f, 0.0f, 0.0f);;
        m_framedTextures[ptr++].vertex = glm::vec3(c[0] / gSettings.cwidth, c[1] / gSettings.cheight, 0.0f);

        m_framedTextures[ptr  ].color = glm::vec3(1.0f, 0.0f, 0.0f);;
        m_framedTextures[ptr++].vertex = glm::vec3(c[0] / gSettings.cwidth, c[1] / gSettings.cheight, 0.0f);
        m_framedTextures[ptr  ].color = glm::vec3(1.0f, 0.0f, 0.0f);;
        m_framedTextures[ptr++].vertex = glm::vec3(d[0] / gSettings.cwidth, d[1] / gSettings.cheight, 0.0f);

        m_framedTextures[ptr  ].color = glm::vec3(1.0f, 0.0f, 0.0f);;
        m_framedTextures[ptr++].vertex = glm::vec3(d[0] / gSettings.cwidth, d[1] / gSettings.cheight, 0.0f);
        m_framedTextures[ptr  ].color = glm::vec3(1.0f, 0.0f, 0.0f);;
        m_framedTextures[ptr++].vertex = glm::vec3(a[0] / gSettings.cwidth, a[1] / gSettings.cheight, 0.0f);
      }
    }

    EnableShader();
    glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*ptr, m_framedTextures, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINES, 0, ptr);
    DisableShader();
  }

  // ################################################################################
  // Jiggle grid

  // Only compute forces along leading edge to save duplication since the grid wraps...
  // xxL
  // xSL
  // xLL
  for (unsigned int dh = 0, ii = 0; dh < gSettings.cheight; ++dh)
  {
    for (unsigned int dw = 0; dw < gSettings.cwidth; ++dw)
    {
      const int offsets[4][2] = { { 1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 } };

      m_accelerations[ii] += m_displacements[ii] * -m_displacements[ii].length();

      for (int jj = 0; jj < 4; ++jj)
      {
        const int nh = (int)dh + offsets[jj][0];
        const int nw = (int)dw + offsets[jj][1];
        const int nii = (nh & (gSettings.cheight - 1)) * gSettings.cwidth + (nw & (gSettings.cwidth - 1));

        rsVec nn = m_displacements[nii] - m_displacements[ii] + rsVec(float(nh - (int)dh), float(nw - (int)dw), 0.0f);

        const float nominalDisplacements[3] = { 0.0f, 1.0f, float(M_SQRT2) };
        const float nominalDisplacement = nominalDisplacements[abs(offsets[jj][0]) + abs(offsets[jj][1])];

        rsVec ff = nn * (nn.length() - nominalDisplacement);
        m_accelerations[ii] += ff;
        m_accelerations[nii] -= ff;
      }

      ++ii;
    }
  }

  rsVec newTotalV(0, 0, 0);
  const float totalVScalar = m_totalV.length();
  const float stepSize = std::min(frameTime, 0.05f);
  for (unsigned int dh = 0, ii = 0; dh < gSettings.cheight; ++dh)
  {
    for (unsigned int dw = 0; dw < gSettings.cwidth; ++dw)
    {
      m_velocities[ii] += m_accelerations[ii] * stepSize;
      m_accelerations[ii] = rsVec(0.0f, 0.0f, 0.0f);

      // Don't let things get too fast
      if (totalVScalar > 20.0f)
      {
        m_velocities[ii] -= m_velocities[ii] / expf(totalVScalar - 20.0f);
      }

      newTotalV += rsVec(abs(m_velocities[ii][0]), abs(m_velocities[ii][1]), 0);

      m_displacements[ii] += m_velocities[ii] * stepSize * gSettings.dSpeed;

      // or displacements too large
      if (m_displacements[ii].length() > 1.0f)
      {
        m_displacements[ii] = m_displacements[ii] / m_displacements[ii].length();
      }

      ++ii;
    }
  }
  m_totalV = newTotalV;

  glDisableVertexAttribArray(m_hVertex);
  glDisableVertexAttribArray(m_hColor);
  glDisableVertexAttribArray(m_hCoord);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindVertexArray(0);
}

void CScreensaverFeedback::OnCompiledAndLinked()
{
  // Variables passed directly to the Vertex shader
  m_projMatLoc = glGetUniformLocation(ProgramHandle(), "u_projectionMatrix");
  m_modelViewMatLoc = glGetUniformLocation(ProgramHandle(), "u_modelViewMatrix");
  m_textureIdLoc = glGetUniformLocation(ProgramHandle(), "u_textureId");

  m_hVertex = glGetAttribLocation(ProgramHandle(), "a_vertex");
  m_hColor = glGetAttribLocation(ProgramHandle(), "a_color");
  m_hCoord = glGetAttribLocation(ProgramHandle(), "a_coord");
}

bool CScreensaverFeedback::OnEnabled()
{
  // This is called after glUseProgram()
  glUniformMatrix4fv(m_projMatLoc, 1, GL_FALSE, glm::value_ptr(m_projMat));
  glUniformMatrix4fv(m_modelViewMatLoc, 1, GL_FALSE, glm::value_ptr(m_modelMat));
  glUniform1i(m_textureIdLoc, m_textureUsed);

  return true;
}

ADDONCREATOR(CScreensaverFeedback);
