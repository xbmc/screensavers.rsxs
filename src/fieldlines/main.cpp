/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
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
#include "ion.h"

#include <chrono>
#include <kodi/gui/General.h>
#include <string.h>
#include <math.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <rsMath/rsMath.h>

bool CScreensaverFieldLines::Start()
{
  kodi::CheckSettingInt("ions", m_ionsCnt);
  kodi::CheckSettingInt("seqsize", m_stepSize);
  kodi::CheckSettingInt("numlines", m_maxSteps);
  kodi::CheckSettingInt("width", m_width);
  kodi::CheckSettingInt("speed", m_speed);
  kodi::CheckSettingBoolean("constant", m_constwidth);
  kodi::CheckSettingBoolean("electric", m_electric);
  kodi::CheckSettingFloat("reduction", m_reduction);

  std::string fraqShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/frag.glsl");
  std::string vertShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
    return false;

  // Initialize pseudorandom number generator
  srand((unsigned)time(nullptr));

  glGenBuffers(1, &m_vertexVBO);

  // calculate boundaries
  if (Width() > Height())
  {
    m_usedHeight = m_usedDeep = 160.0f;
    m_usedWidth = m_usedHeight * Width() / Height();
  }
  else
  {
    m_usedWidth = m_usedDeep = 160.0f;
    m_usedHeight = m_usedWidth * Height() / Width();
  }

  if (m_ionsCnt < 1)
    m_ionsCnt = 1;
  if (m_ionsCnt > 50)
    m_ionsCnt = 50;
  for (int i = 0; i < m_ionsCnt; ++i)
    m_ions.push_back(this);

  m_projMat = glm::mat4(1.0f);
  m_modelMat = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f/m_usedDeep/m_reduction, 1.0f/m_usedDeep/m_reduction, 1.0f/m_usedDeep/m_reduction));

  m_packets = new PackedVertex[m_maxSteps * 4 + 2];

  m_startOK = true;
  m_lastTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  return true;
}

void CScreensaverFieldLines::Stop()
{
  if (!m_startOK)
    return;

  m_startOK = false;

  glDisable(GL_DEPTH_TEST);
#ifndef HAS_GLES
  glDisable(GL_LINE_SMOOTH);
#endif

  glDeleteBuffers(1, &m_vertexVBO);
  m_vertexVBO = 0;

  delete[] m_packets;
}

void CScreensaverFieldLines::Render()
{
  if (!m_startOK)
    return;

  /*
   * Following Extra work done here in render to prevent problems with controls
   * from Kodi and during window moving.
   * TODO: Maybe add a separate interface call to inform about?
   */
  //@{
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);

  glVertexAttribPointer(m_hPos,  3, GL_FLOAT, 0, sizeof(PackedVertex), BUFFER_OFFSET(offsetof(PackedVertex, x)));
  glEnableVertexAttribArray(m_hPos);

  glVertexAttribPointer(m_hCol, 3, GL_FLOAT, 0, sizeof(PackedVertex), BUFFER_OFFSET(offsetof(PackedVertex, r)));
  glEnableVertexAttribArray(m_hCol);
  //@}

  // Use our shader
  EnableShader();

  double currentTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  float frameTime = static_cast<float>(currentTime - m_lastTime);
  m_lastTime = currentTime;

  glEnable(GL_DEPTH_TEST);
#ifndef HAS_GLES
  glEnable(GL_LINE_SMOOTH);
#endif
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  if (m_constwidth)
    glLineWidth(float(m_width) * 0.1f);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  static float s = float(sqrt(float(m_stepSize) * float(m_stepSize) * 0.333f));
  for (auto& ion : m_ions)
  {
    ion.update(frameTime);

    drawfieldline(ion, s, s, s);
    drawfieldline(ion, s, s, -s);
    drawfieldline(ion, s, -s, s);
    drawfieldline(ion, s, -s, -s);
    drawfieldline(ion, -s, s, s);
    drawfieldline(ion, -s, s, -s);
    drawfieldline(ion, -s, -s, s);
    drawfieldline(ion, -s, -s, -s);
  }

  DisableShader();

  glDisableVertexAttribArray(m_hPos);
  glDisableVertexAttribArray(m_hCol);
}

void CScreensaverFieldLines::drawfieldline(CIon& ion, float x, float y, float z)
{
  float charge;
  float repulsion;
  float dist, distsquared, distrec;
  float xyz[3];
  float lastxyz[3];
  float dir[3];
  float end[3];
  float tempvec[3];
  float r, g, b;
  float lastr, lastg, lastb;
  static float brightness = 10000.0f;

  charge = ion.charge;
  lastxyz[0] = ion.xyz[0];
  lastxyz[1] = ion.xyz[1];
  lastxyz[2] = ion.xyz[2];
  dir[0] = x;
  dir[1] = y;
  dir[2] = z;

  // Do the first segment
  r = float(fabs(dir[2])) * brightness;
  g = float(fabs(dir[0])) * brightness;
  b = float(fabs(dir[1])) * brightness;
  if (r > 1.0f)
    r = 1.0f;
  if (g > 1.0f)
    g = 1.0f;
  if (b > 1.0f)
    b = 1.0f;
  lastr = r;
  lastg = g;
  lastb = b;
  xyz[0] = lastxyz[0] + dir[0];
  xyz[1] = lastxyz[1] + dir[1];
  xyz[2] = lastxyz[2] + dir[2];
  if (m_electric)
  {
    xyz[0] += rsRandf(float(m_stepSize) * 0.2f) - (float(m_stepSize) * 0.1f);
    xyz[1] += rsRandf(float(m_stepSize) * 0.2f) - (float(m_stepSize) * 0.1f);
    xyz[2] += rsRandf(float(m_stepSize) * 0.2f) - (float(m_stepSize) * 0.1f);
  }

  unsigned int ptr = 0;

  m_packets[ptr].x = lastxyz[0];
  m_packets[ptr].y = lastxyz[1];
  m_packets[ptr].z = lastxyz[2];
  m_packets[ptr].r = lastr;
  m_packets[ptr].g = lastg;
  m_packets[ptr].b = lastb;
  ptr++;
  m_packets[ptr].x = xyz[0];
  m_packets[ptr].y = xyz[1];
  m_packets[ptr].z = xyz[2];
  m_packets[ptr].r = r;
  m_packets[ptr].g = g;
  m_packets[ptr].b = b;
  ptr++;

  if (!m_constwidth)
  {
    glBufferData(GL_ARRAY_BUFFER, sizeof(PackedVertex)*ptr, &m_packets[0], GL_STATIC_DRAW);
    glLineWidth((xyz[2] + 300.0f) * 0.000333f * float(m_width));
    glDrawArrays(GL_LINE_STRIP, 0, ptr);
//     ptr = 0; // Unused to prevent wrong size in middle
  }

  int i;
  for (i = 0; i < m_maxSteps; i++)
  {
    dir[0] = 0.0f;
    dir[1] = 0.0f;
    dir[2] = 0.0f;
    for (int j = 0; j < m_ionsCnt; j++)
    {
      repulsion = charge * m_ions[j].charge;
      tempvec[0] = xyz[0] - m_ions[j].xyz[0];
      tempvec[1] = xyz[1] - m_ions[j].xyz[1];
      tempvec[2] = xyz[2] - m_ions[j].xyz[2];
      distsquared = tempvec[0] * tempvec[0] + tempvec[1] * tempvec[1] + tempvec[2] * tempvec[2];
      dist = float(sqrt(distsquared));
      if (dist < float(m_stepSize) && i > 2)
      {
        end[0] = m_ions[j].xyz[0];
        end[1] = m_ions[j].xyz[1];
        end[2] = m_ions[j].xyz[2];
        i = 10000;
      }
      tempvec[0] /= dist;
      tempvec[1] /= dist;
      tempvec[2] /= dist;
      if (distsquared < 1.0f)
        distsquared = 1.0f;
      dir[0] += tempvec[0] * repulsion / distsquared;
      dir[1] += tempvec[1] * repulsion / distsquared;
      dir[2] += tempvec[2] * repulsion / distsquared;
    }
    lastr = r;
    lastg = g;
    lastb = b;
    r = float(fabs(dir[2])) * brightness;
    g = float(fabs(dir[0])) * brightness;
    b = float(fabs(dir[1])) * brightness;
    if (m_electric)
    {
      r *= 10.0f;
      g *= 10.0f;
      b *= 10.0f;;
      if(r > b * 0.5f)
        r = b * 0.5f;
      if(g > b * 0.3f)
        g = b * 0.3f;
    }
    if (r > 1.0f)
      r = 1.0f;
    if (g > 1.0f)
      g = 1.0f;
    if (b > 1.0f)
      b = 1.0f;
    distsquared = dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2];
    distrec = float(m_stepSize) / float(sqrt(distsquared));
    dir[0] *= distrec;
    dir[1] *= distrec;
    dir[2] *= distrec;
    if (m_electric)
    {
      dir[0] += rsRandf(float(m_stepSize)) - (float(m_stepSize) * 0.5f);
      dir[1] += rsRandf(float(m_stepSize)) - (float(m_stepSize) * 0.5f);
      dir[2] += rsRandf(float(m_stepSize)) - (float(m_stepSize) * 0.5f);
    }
    lastxyz[0] = xyz[0];
    lastxyz[1] = xyz[1];
    lastxyz[2] = xyz[2];
    xyz[0] += dir[0];
    xyz[1] += dir[1];
    xyz[2] += dir[2];

    m_packets[ptr].r = lastr;
    m_packets[ptr].g = lastg;
    m_packets[ptr].b = lastb;
    m_packets[ptr].x = lastxyz[0];
    m_packets[ptr].y = lastxyz[1];
    m_packets[ptr].z = lastxyz[2];
    ptr++;

    if (i != 10000)
    {
      if (i == (m_maxSteps - 1))
      {
        m_packets[ptr].r = 0.0f;
        m_packets[ptr].g = 0.0f;
        m_packets[ptr].b = 0.0f;
      }
      else
      {
        m_packets[ptr].r = r;
        m_packets[ptr].g = g;
        m_packets[ptr].b = b;
      }
      m_packets[ptr].x = lastxyz[0];
      m_packets[ptr].y = lastxyz[1];
      m_packets[ptr].z = lastxyz[2];
      ptr++;
    }
  }

  if (i == 10001)
  {
    m_packets[ptr].r = r;
    m_packets[ptr].g = g;
    m_packets[ptr].b = b;
    m_packets[ptr].x = end[0];
    m_packets[ptr].y = end[1];
    m_packets[ptr].z = end[2];
    ptr++;
  }

  if (!m_constwidth)
    glLineWidth((xyz[2] + 300.0f) * 0.000333f * float(m_width));

  glBufferData(GL_ARRAY_BUFFER, sizeof(PackedVertex)*ptr, &m_packets[0], GL_STATIC_DRAW);
  glDrawArrays(GL_LINE_STRIP, 0, ptr);
}

void CScreensaverFieldLines::OnCompiledAndLinked()
{
  // Variables passed directly to the Vertex shader
  m_hProj = glGetUniformLocation(ProgramHandle(), "u_proj");
  m_hModel = glGetUniformLocation(ProgramHandle(), "u_model");
  m_hPos = glGetAttribLocation(ProgramHandle(), "a_pos");
  m_hCol = glGetAttribLocation(ProgramHandle(), "a_col");
}

bool CScreensaverFieldLines::OnEnabled()
{
  // This is called after glUseProgram()
  glUniformMatrix4fv(m_hProj, 1, GL_FALSE, glm::value_ptr(m_projMat));
  glUniformMatrix4fv(m_hModel, 1, GL_FALSE, glm::value_ptr(m_modelMat));
  return true;
}

ADDONCREATOR(CScreensaverFieldLines);
