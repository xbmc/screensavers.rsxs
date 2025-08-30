/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2003 Holmes Futrell <holmes@neatosoftware.com>
 *  Ported to Kodi by Alwin Esch <alwinus@kodi.tv>
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

#include <chrono>
#include <kodi/gui/General.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <rsMath/rsMath.h>

bool CScreensaverSpiroGraphX::Start()
{
  m_timeInterval = static_cast<float>(kodi::addon::GetSettingInt("general.interval"));
  m_detail = kodi::addon::GetSettingInt("general.detail");

  std::string fraqShader = kodi::addon::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/frag.glsl");
  std::string vertShader = kodi::addon::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
    return false;

  // Initialize pseudorandom number generator
  srand((unsigned)time(nullptr));

  glGenVertexArrays(1, &m_vao);

  glGenBuffers(2, m_vertexVBO);

  m_content.blurWidth = kodi::addon::GetSettingInt("general.blurwidth");

  glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
  glClearDepthf(1.0f);
  glEnable(GL_BLEND);
#if !defined(HAS_GLES)
  //TODO: Bring in a way about in GLES 2.0 and above!
  glEnable(GL_LINE_SMOOTH);
#endif
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);

  ChangeSettings();
  m_contentOld = m_content;

  m_projMat = glm::perspective(glm::radians(kodi::addon::GetSettingBoolean("general.fitwidth") ? 45.0f : 90.0f), (GLfloat) Width() / (GLfloat) Height(), 0.1f, 100.0f);
  m_lastTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  m_startOK = true;
  return true;
}

void CScreensaverSpiroGraphX::Stop()
{
  m_startOK = false;

  glDeleteBuffers(2, m_vertexVBO);
  memset(m_vertexVBO, 0, sizeof(m_vertexVBO));

  glDeleteVertexArrays(1, &m_vao);

  // Kodi defaults
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#if !defined(HAS_GLES)
  //TODO: Bring in a way about in GLES 2.0 and above!
  glDisable(GL_LINE_SMOOTH);
#endif
}

void CScreensaverSpiroGraphX::Render()
{
  if (!m_startOK)
    return;

  double currentTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  float frameTime = static_cast<float>(currentTime - m_lastTime);
  m_lastTime = currentTime;

  if (m_lastSettingsChange == -1)
    m_lastSettingsChange = currentTime;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  GetAll(m_content);
  if (m_contentOldActive)
    GetAll(m_contentOld);

  m_modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));

  glBindVertexArray(m_vao);

  EnableShader();

  float width = sqrt((GLfloat) (Width() * Height()) / (500 * 400));
  glLineWidth(m_content.blurWidth * width);
  DrawAll(m_content, m_content.colorsBlur);

  glLineWidth(1);
  DrawAll(m_content, m_content.colorsLine);

  if (m_contentOldActive)
  {
    glLineWidth(m_contentOld.blurWidth * sqrt((GLfloat) (Width() * Height()) / (500 * 400)));
    DrawAll(m_contentOld, m_contentOld.colorsBlur);

    glLineWidth(1);
    DrawAll(m_contentOld, m_contentOld.colorsLine);
  }

  DisableShader();

  if (currentTime - m_lastSettingsChange > m_timeInterval)
  {
    m_lastSettingsChange = currentTime;
    m_contentOld = m_content;

    ChangeSettings();

    m_content.fade = 0.0f;
    m_contentOld.fade = 1.0f;
    m_contentOldActive = true;
  }
  else if (m_contentOldActive)
  {
    if (m_content.fade < 1.0f)
      m_content.fade += 0.05f;
    if (m_contentOld.fade > 0.0f)
      m_contentOld.fade -= 0.05f;
    else
      m_contentOldActive = false;

    m_contentOld.equationBase += m_contentOld.speed * (frameTime / (1.0f / 30.0f));
  }

  m_content.equationBase += m_content.speed * (frameTime / (1.0f / 30.0f));

  glBindVertexArray(0);

  glFlush();
}

void CScreensaverSpiroGraphX::ChangeSettings()
{
  do {
    m_content.equationBase = rsRandf(10) - 5;
  } while (m_content.equationBase <= 2 && m_content.equationBase >= -2); // we don't want between 1 and -1

  m_content.blurColor[0] = rsRandi(100) / 50.0f;
  m_content.blurColor[1] = rsRandi(100) / 50.0f;
  m_content.blurColor[2] = rsRandi(100) / 50.0f;

  m_content.lineColor[0] = rsRandi(100) / 50.0f;
  m_content.lineColor[1] = rsRandi(100) / 50.0f;
  m_content.lineColor[2] = rsRandi(100) / 50.0f;

  m_content.subLoops = rsRandi(3) + 2;
  m_content.graphTo = rsRandi(16) + 15;
  m_content.speed = (rsRandi(225) + 75) / 1000000.0f;

  if (rsRandi(2) == 1)
    m_content.speed *= -1;
}

void CScreensaverSpiroGraphX::GetAll(renderContent& content)
{
  int m, n;

  float poweranswer[5];
  poweranswer[0] = 1;
  for (n = 1; n < content.subLoops; n++)
    poweranswer[n] = poweranswer[n - 1] * content.equationBase;

  content.numberOfPoints = 2 * M_PI * content.graphTo * m_detail;

  memset(content.points, 0, content.numberOfPoints * sizeof(glm::vec3));

  for (m = 0; m < content.numberOfPoints; m++)
  {
    const float moverd = (float)m / m_detail;
    for (n = 0; n < content.subLoops; n++)
    {
      const float pointpoweroverdetail = moverd * poweranswer[n];
      float sinppod = rsSinf(pointpoweroverdetail);
      float cosppod = rsCosf(pointpoweroverdetail);

      content.points[m].x += cosppod / poweranswer[n];
      content.points[m].y += sinppod / poweranswer[n];
    }
    content.points[m].z = 1.0f;

    content.colorsBlur[m].r = content.blurColor[0];
    content.colorsBlur[m].g = content.blurColor[1];
    content.colorsBlur[m].b = content.blurColor[2];
    content.colorsBlur[m].a = m_blurAlpha * content.fade;

    content.colorsLine[m].r = content.lineColor[0];
    content.colorsLine[m].g = content.lineColor[1];
    content.colorsLine[m].b = content.lineColor[2];
    content.colorsLine[m].a = m_lineAlpha * content.fade;
  }
}

void CScreensaverSpiroGraphX::DrawAll(renderContent& content, glm::vec4* colors)
{
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*content.numberOfPoints, content.points, GL_STATIC_DRAW);
  glVertexAttribPointer(m_hPos,  3, GL_FLOAT, 0, sizeof(glm::vec3), BUFFER_OFFSET(offsetof(glm::vec3, x)));
  glEnableVertexAttribArray(m_hPos);

  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*content.numberOfPoints, colors, GL_STATIC_DRAW);
  glVertexAttribPointer(m_hCol, 4, GL_FLOAT, 0, sizeof(glm::vec4), BUFFER_OFFSET(offsetof(glm::vec4, r)));
  glEnableVertexAttribArray(m_hCol);

  glDrawArrays(GL_LINE_STRIP, 0, content.numberOfPoints);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CScreensaverSpiroGraphX::OnCompiledAndLinked()
{
  // Variables passed directly to the Vertex shader
  m_hProj = glGetUniformLocation(ProgramHandle(), "u_projectionMatrix");
  m_hModel = glGetUniformLocation(ProgramHandle(), "u_modelViewMatrix");
  m_hPos = glGetAttribLocation(ProgramHandle(), "a_position");
  m_hCol = glGetAttribLocation(ProgramHandle(), "a_color");

  // It's okay to do this only one time. Textures units never change.
  glUseProgram(ProgramHandle());
  glUseProgram(0);
}

bool CScreensaverSpiroGraphX::OnEnabled()
{
  // This is called after glUseProgram()
  glUniformMatrix4fv(m_hProj, 1, GL_FALSE, glm::value_ptr(m_projMat));
  glUniformMatrix4fv(m_hModel, 1, GL_FALSE, glm::value_ptr(m_modelMat));

  return true;
}

ADDONCREATOR(CScreensaverSpiroGraphX);
