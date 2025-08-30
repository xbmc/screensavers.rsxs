/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2002 Dirk Songuer <delax@sundancerinc.de>
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

bool CScreensaverSunDancer2::Start()
{
  // Initialize pseudorandom number generator
  srand((unsigned)time(nullptr));

  int colorMode = kodi::addon::GetSettingInt("color.type");
  if (colorMode == 0)
  {
    m_backRed = rsRandi(256);
    m_backGreen = rsRandi(256);
    m_backBlue = rsRandi(256);

    m_frontRed = rsRandi(256);
    m_frontGreen = rsRandi(256);
    m_frontBlue = rsRandi(256);
  }
  else if (colorMode == 1)
  {
    m_backRed = 255;
    m_backGreen = 0;
    m_backBlue = 0;

    m_frontRed = 255;
    m_frontGreen = 255;
    m_frontBlue = 0;
  }
  else if (colorMode == 2)
  {
    m_backRed = kodi::addon::GetSettingInt("color.background-red");
    m_backGreen = kodi::addon::GetSettingInt("color.background-green");
    m_backBlue = kodi::addon::GetSettingInt("color.background-blue");

    m_frontRed = kodi::addon::GetSettingInt("color.foreground-red");
    m_frontGreen = kodi::addon::GetSettingInt("color.foreground-green");
    m_frontBlue = kodi::addon::GetSettingInt("color.foreground-blue");
  }

  int reverse = kodi::addon::GetSettingInt("general.reverse");
  if (reverse == 0)
    m_reverse = rsRandi(2);
  else if (reverse == 1)
    m_reverse = false;
  else
    m_reverse = true;

  if (kodi::addon::GetSettingBoolean("general.automatictransparency"))
    m_transparencyValue = (rsRandf(100) + 1) / 100.0f;
  else
    m_transparencyValue = kodi::addon::GetSettingInt("general.transparency") / 100.0f;

  int quartcnt = kodi::addon::GetSettingInt("general.quartcnt");
  if (quartcnt == 0)
    m_quadCount = rsRandi(296) + 5;
  else
    m_quadCount = quartcnt;

  int speed = kodi::addon::GetSettingInt("general.speed");
  if (speed == 0)
    m_quadSpeedMax = rsRandf(50) / 400.0f;
  else
    m_quadSpeedMax = speed / 400.0f;
  m_quadSpeed = m_quadSpeedMax;

  std::string fraqShader = kodi::addon::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/frag.glsl");
  std::string vertShader = kodi::addon::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
    return false;

  glGenVertexArrays(1, &m_vao);

  glGenBuffers(1, &m_vertexVBO);
  glGenBuffers(1, &m_indexVBO);

  float Ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
  float Diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
  float temp_r, temp_g, temp_b;
  float step_r, step_g, step_b;
  int i;

  glClearColor(0.0, 0.0, 0.0, 0.0);

  if (m_transparencyValue == 1.0f)
  {
    glEnable(GL_DEPTH_TEST);
#if defined(HAS_GLES)
    glClearDepthf(1);
#else
    glClearDepth(1);
#endif
    glDepthFunc(GL_LESS);
  }
  else
  {
    glEnable(GL_BLEND);  // Turn Blending On
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  m_projMat = glm::ortho(-400.0f, 400.0f, -300.0f, 300.0f, -400.0f, 400.0f);

  // Init Quads
  m_quads = (quadsystem *) malloc (m_quadCount * sizeof (quadsystem));

  step_r = (m_frontRed - m_backRed) / (m_quadCount * 255.0f);
  step_g = (m_frontGreen - m_backGreen) / (m_quadCount * 255.0f);
  step_b = (m_frontBlue - m_backBlue) / (m_quadCount * 255.0f);

  temp_r = m_backRed / 255.0f;
  temp_g = m_backGreen / 255.0f;
  temp_b = m_backBlue / 255.0f;

  for (i = 0; i < m_quadCount; i++)
  {
    m_quads[i].PositionX = -100.0f;
    m_quads[i].PositionY = -100.0f;
    m_quads[i].PositionZ = i * 1.4f;

    m_quads[i].Laenge = 300 - i;
    m_quads[i].Hoehe = 300 - i;
    m_quads[i].Rotation = i / 100.0f;

    m_quads[i].ColorR = temp_r;
    m_quads[i].ColorG = temp_g;
    m_quads[i].ColorB = temp_b;

    temp_r += step_r;
    temp_g += step_g;
    temp_b += step_b;
  }

  // Init Light
  m_vlight = 0.0;
  m_hlight = 0.0;
  m_zlight = 0.0;
  m_vlightmul = -1;
  m_hlightmul = -1;
  m_zlightmul = -1;

  // Init Vertices
  m_vertexw1 = 1.0;
  m_vertexw2 = 1.0;
  m_vertexw3 = 1.0;
  m_vertexw4 = 1.0;

  m_vertexwmul1 = rsRandf(10) / 4096;
  m_vertexwmul2 = rsRandf(10) / 4096;
  m_vertexwmul3 = rsRandf(10) / 4096;
  m_vertexwmul4 = rsRandf(10) / 4096;

  m_startOK = true;
  return true;
}

void CScreensaverSunDancer2::Stop()
{
  m_startOK = false;

  glDeleteBuffers(1, &m_vertexVBO);
  m_vertexVBO = 0;
  glDeleteBuffers(1, &m_indexVBO);
  m_indexVBO = 0;

  glDeleteVertexArrays(1, &m_vao);

  // Kodi defaults
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
}

void CScreensaverSunDancer2::Render()
{
  struct sLatticeSegmentEntry
  {
    glm::vec4 vertex;
    glm::vec4 color;
  } segment[4];

  if (!m_startOK)
    return;

  glBindVertexArray(m_vao);

  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);

  glVertexAttribPointer(m_hPos,  4, GL_FLOAT, 0, sizeof(sLatticeSegmentEntry), BUFFER_OFFSET(offsetof(sLatticeSegmentEntry, vertex)));
  glEnableVertexAttribArray(m_hPos);

  glVertexAttribPointer(m_hCol, 4, GL_FLOAT, 0, sizeof(sLatticeSegmentEntry), BUFFER_OFFSET(offsetof(sLatticeSegmentEntry, color)));
  glEnableVertexAttribArray(m_hCol);

  if (m_transparencyValue == 1.0f)
  {
    glEnable(GL_DEPTH_TEST);
#if defined(HAS_GLES)
    glClearDepthf(1);
#else
    glClearDepth(1);
#endif
    glDepthFunc(GL_LESS);
  }
  else
  {
    glEnable(GL_BLEND);  // Turn Blending On
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  double currentTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();

  static int change_direction = 0;
  int i;

  if (m_quads_timer == -1)
    m_quads_timer = (int)currentTime + (rsRandi(10000) + 3);

  m_lightPosition[0] = m_hlight;  // set new light position - x
  m_lightPosition[1] = m_vlight;  // set new light position - y
  m_lightPosition[2] = m_zlight;  // set new light position - z

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -10.0f, 0.0f));

  m_modelMat = glm::mat4(1.0f);

  GLubyte idx[4] = {0, 1, 3, 2};
  for (i = 0; i < m_quadCount; i++)
  {
    segment[0].color.r = m_quads[i].ColorR;
    segment[0].color.g = m_quads[i].ColorG;
    segment[0].color.b = m_quads[i].ColorB;
    segment[0].color.a = m_transparencyValue;
    segment[0].vertex.x = m_quads[i].PositionX;
    segment[0].vertex.y = m_quads[i].PositionY;
    segment[0].vertex.z = m_quads[i].PositionZ;
    segment[0].vertex.w = m_vertexw1;

    segment[1].color.r = m_quads[i].ColorR;
    segment[1].color.g = m_quads[i].ColorG;
    segment[1].color.b = m_quads[i].ColorB;
    segment[1].color.a = m_transparencyValue;
    segment[1].vertex.x = m_quads[i].PositionX + m_quads[i].Laenge;
    segment[1].vertex.y = m_quads[i].PositionY;
    segment[1].vertex.z = m_quads[i].PositionZ;
    segment[1].vertex.w = m_vertexw2;

    segment[2].color.r = m_quads[i].ColorR;
    segment[2].color.g = m_quads[i].ColorG;
    segment[2].color.b = m_quads[i].ColorB;
    segment[2].color.a = m_transparencyValue;
    segment[2].vertex.x = m_quads[i].PositionX + m_quads[i].Laenge;
    segment[2].vertex.y = m_quads[i].PositionY + m_quads[i].Hoehe;
    segment[2].vertex.z = m_quads[i].PositionZ;
    segment[2].vertex.w = m_vertexw3;

    segment[3].color.r = m_quads[i].ColorR;
    segment[3].color.g = m_quads[i].ColorG;
    segment[3].color.b = m_quads[i].ColorB;
    segment[3].color.a = m_transparencyValue;
    segment[3].vertex.x = m_quads[i].PositionX;
    segment[3].vertex.y = m_quads[i].PositionY + m_quads[i].Hoehe;
    segment[3].vertex.z = m_quads[i].PositionZ;
    segment[3].vertex.w = m_vertexw4;

    m_modelMat = glm::rotate(m_modelMat, glm::radians(m_quads[i].Rotation), glm::vec3(0.0f, 0.0f, 1.0f));

    EnableShader();
    glBufferData(GL_ARRAY_BUFFER, sizeof(sLatticeSegmentEntry)*4, &segment[0], GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte)*4, idx, GL_STATIC_DRAW);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, 0);
    DisableShader();
  }

  // Update Quads
  if (m_reverse)    //rotation
  {
    if (m_quads_timer < (int)currentTime)
      change_direction = 1;

    if (change_direction)
      m_quadSpeed -= m_direction_add * m_quadCount;

    if ((m_quadSpeed > m_quadSpeedMax) || (m_quadSpeed < -m_quadSpeedMax))
    {
      if (m_quadSpeed > m_quadSpeedMax)
        m_quadSpeed = m_quadSpeedMax - 0.0001f;
      else
        m_quadSpeed = -m_quadSpeedMax + 0.0001f;

      change_direction = 0;
      m_direction_add *= -1;
      m_quads_timer = (int)currentTime + (rsRandi(30) + 3);
    }
  }

  for (i = 0; i < m_quadCount; i++)
  {
    m_quads[i].Rotation += m_quadSpeed;
  }

  // Update Light
  if ((m_vlight > 300) || (m_vlight < -300))
    m_vlightmul *= -1;
  if ((m_hlight > 300) || (m_hlight < -300))
    m_hlightmul *= -1;
  if ((m_zlight > 400) || (m_zlight < 0))
    m_zlightmul *= -1;

  m_vlight += m_vlightmul;
  m_hlight += m_hlightmul;
  m_zlight += m_zlightmul;

  // Update Vertices
  if ((m_vertexw1 > 2.5f) || (m_vertexw1 < 0.7f))
    m_vertexwmul1 *= -1;
  if ((m_vertexw2 > 2.5f) || (m_vertexw2 < 0.7f))
    m_vertexwmul2 *= -1;
  if ((m_vertexw3 > 2.5f) || (m_vertexw3 < 0.7f))
    m_vertexwmul3 *= -1;
  if ((m_vertexw4 > 2.5f) || (m_vertexw4 < 0.7f))
    m_vertexwmul4 *= -1;

  m_vertexw1 += m_vertexwmul1;
  m_vertexw2 += m_vertexwmul2;
  m_vertexw3 += m_vertexwmul3;
  m_vertexw4 += m_vertexwmul4;

  glDisableVertexAttribArray(m_hPos);
  glDisableVertexAttribArray(m_hCol);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindVertexArray(0);
}

void CScreensaverSunDancer2::OnCompiledAndLinked()
{
  // Variables passed directly to the Vertex shader
  m_hProj = glGetUniformLocation(ProgramHandle(), "u_projectionMatrix");
  m_hModel = glGetUniformLocation(ProgramHandle(), "u_modelViewMatrix");
  m_hPos = glGetAttribLocation(ProgramHandle(), "a_position");
  m_hCol = glGetAttribLocation(ProgramHandle(), "a_color");

  m_light1_ambientLoc = glGetUniformLocation(ProgramHandle(), "u_light1.ambient");
  m_light1_diffuseLoc = glGetUniformLocation(ProgramHandle(), "u_light1.diffuse");
  m_light1_positionLoc = glGetUniformLocation(ProgramHandle(), "u_light1.position");
}

bool CScreensaverSunDancer2::OnEnabled()
{
  // This is called after glUseProgram()
  glUniformMatrix4fv(m_hProj, 1, GL_FALSE, glm::value_ptr(m_projMat));
  glUniformMatrix4fv(m_hModel, 1, GL_FALSE, glm::value_ptr(m_modelMat));

  glUniform4f(m_light1_ambientLoc, 0.1f, 0.1f, 0.1f, 1.0f);
  glUniform4f(m_light1_diffuseLoc, 1.0f, 1.0f, 1.0f, 1.0f);
  glUniform4f(m_light1_positionLoc, m_lightPosition[0], m_lightPosition[1], m_lightPosition[2], m_lightPosition[3]);

  return true;
}

ADDONCREATOR(CScreensaverSunDancer2);
