/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2002 <hk@dgmr.nl>
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
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <rsMath/rsMath.h>

#define TRIANGLES 1
#define SPHERES 2
#define BIGSPHERES 3
#define POINTS 4

CScreensaverBiof::CScreensaverBiof()
{
  // Initialize pseudorandom number generator
  srand((unsigned)time(nullptr));

  m_geometry = kodi::addon::GetSettingInt("general.type");
  if (m_geometry == 0)
    m_geometry = rsRandi(4) + 1;

  int offAngle = kodi::addon::GetSettingInt("general.offangle");
  if (offAngle == 0)
    m_offAngle = rsRandi(2);
  else if (offAngle == 1)
    m_offAngle = false;
  else
    m_offAngle = true;

  m_linesQty = kodi::addon::GetSettingInt("general.lines");
  m_pointsQty = kodi::addon::GetSettingInt("general.points");

  switch (m_geometry)
  {
  case TRIANGLES:
  case SPHERES:
    if (m_linesQty == 0)
      m_linesQty = rsRandi(7) + 2;
    if (m_pointsQty == 0)
      m_pointsQty = rsRandi(96) + 32;
    break;

  case BIGSPHERES:
    if (m_linesQty == 0)
      m_linesQty = rsRandi(7) + 4;
    if (m_pointsQty == 0)
      m_pointsQty = rsRandi(32) + 4;
    break;

  case POINTS:
    if (m_linesQty == 0)
      m_linesQty = rsRandi(32) + 4;
    if (m_pointsQty == 0)
      m_pointsQty = rsRandi(64) + 64;
  }
}

bool CScreensaverBiof::Start()
{
  std::string fraqShader = kodi::addon::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/frag.glsl");
  std::string vertShader = kodi::addon::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
    return false;

  glGenBuffers(4, m_vboHandle);

  m_normal.clear();
  m_vertex.clear();
  m_color.clear();
  m_index.clear();
  if ((m_geometry == SPHERES) || (m_geometry == BIGSPHERES))
  {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    m_fogEnabled = false;
    m_lightEnabled = true;

    if (m_geometry == BIGSPHERES)
    {
      m_cubeSectorCount = 20;
      m_cubeStackCount = 10;
    }
    else
    {
      m_cubeSectorCount = 6;
      m_cubeStackCount = 4;
    }

    CreateCubeVerticesSmooth();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboHandle[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*m_index.size(), &m_index[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }
  else if (m_geometry == TRIANGLES)
  {
    m_normal.push_back(std::move(glm::vec3(0.0f, 0.0f, 0.0f)));
    m_vertex.push_back(std::move(glm::vec3(2.0f, 2.0f, 0.0f)));
    m_color.push_back(std::move(glm::vec4(0.9f, 0.8f, 0.5f, 1.0f)));
    m_index.push_back(0);

    m_normal.push_back(std::move(glm::vec3(0.0f, 0.0f, 0.0f)));
    m_vertex.push_back(std::move(glm::vec3(0.0f, 2.0f, 0.0f)));
    m_color.push_back(std::move(glm::vec4(0.9f, 0.8f, 0.5f, 0.0f)));
    m_index.push_back(1);

    m_normal.push_back(std::move(glm::vec3(0.0f, 0.0f, 0.0f)));
    m_vertex.push_back(std::move(glm::vec3(2.0f, 0.0f, 0.0f)));
    m_color.push_back(std::move(glm::vec4(0.9f, 0.8f, 0.5f, 0.0f)));
    m_index.push_back(2);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboHandle[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*m_index.size(), &m_index[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    m_fogEnabled = false;
    m_lightEnabled = false;
  }
  else if (m_geometry == POINTS)
  {
    m_pointsSize = 3.0f;
#if !defined(HAS_GLES) && !defined(TARGET_DARWIN)
    // TODO: Bring in a way about in GLES 2.0 and above!
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glPointSize(m_pointsSize);
    glEnable(GL_POINT_SMOOTH);
#endif
    m_normal.push_back(std::move(glm::vec3(0.0f, 0.0f, 0.0f)));
    m_vertex.push_back(std::move(glm::vec3(0.0f, 0.0f, 0.0f)));
    m_color.push_back(std::move(glm::vec4(0.9f, 0.8f, 0.5f, 1.0f)));

    m_fogEnabled = true;
    m_lightEnabled = false;
  }

  glViewport(X(), Y(), (GLsizei)Width(), (GLsizei)Height());
  m_projMat = glm::perspective(glm::radians(30.0f), (float)Width() / (float)Height(), 100.0f, 300.0f);

  glBindBuffer(GL_ARRAY_BUFFER, m_vboHandle[0]);
  glBufferData(GL_ARRAY_BUFFER, m_normal.size() * sizeof(glm::vec3), &m_normal[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, m_vboHandle[1]);
  glBufferData(GL_ARRAY_BUFFER, m_vertex.size() * sizeof(glm::vec3), &m_vertex[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, m_vboHandle[2]);
  glBufferData(GL_ARRAY_BUFFER, m_color.size() * sizeof(glm::vec4), &m_color[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  m_startFrameTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  m_startOK = true;
  return true;
}

void CScreensaverBiof::Stop()
{
  if (!m_startOK)
    return;

  m_startOK = false;

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glDeleteBuffers(4, m_vboHandle);
  memset(m_vboHandle, 0, sizeof(m_vboHandle));

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
}

void CScreensaverBiof::Render()
{
  if (!m_startOK)
    return;

  if ((m_geometry == SPHERES) || (m_geometry == BIGSPHERES))
    glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glBindBuffer(GL_ARRAY_BUFFER, m_vboHandle[0]);
  glVertexAttribPointer(m_hNormal, 3, GL_FLOAT, GL_TRUE, sizeof(glm::vec3), ((GLubyte *)nullptr + (0)));
  glEnableVertexAttribArray(m_hNormal);

  glBindBuffer(GL_ARRAY_BUFFER, m_vboHandle[1]);
  glVertexAttribPointer(m_hVertex, 3, GL_FLOAT, GL_TRUE, sizeof(glm::vec3), ((GLubyte *)nullptr + (0)));
  glEnableVertexAttribArray(m_hVertex);

  glBindBuffer(GL_ARRAY_BUFFER, m_vboHandle[2]);
  glVertexAttribPointer(m_hColor, 4, GL_FLOAT, GL_TRUE, sizeof(glm::vec4), ((GLubyte *)nullptr + (0)));
  glEnableVertexAttribArray(m_hColor);

  float frameTime = static_cast<float>(std::chrono::duration<double>(
                      std::chrono::system_clock::now().time_since_epoch()).count() - m_startFrameTime);

  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glm::mat4 modelMat;
  modelMat = glm::lookAt(glm::vec3(200.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  modelMat = glm::rotate(modelMat, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  modelMat = glm::rotate(modelMat, glm::radians(float(10.0f * frameTime)), glm::vec3(0.0f, 0.0f, 1.0f));
  if (m_offAngle)
    modelMat = glm::rotate(modelMat, glm::radians(45.0f), glm::vec3(1.0f, 1.0f, 0.0f));

  EnableShader();

  float bend       =        315.0f * sinf(frameTime * 0.237f);
  float stack      = 50.0f + 20.0f * sinf(frameTime * 0.133f);
  float twist      =       1500.0f * sinf(frameTime * 0.213f);
  float twisttrans =  7.5f +  2.5f * sinf(frameTime * 0.173f);
  float grow       =  0.6f +  0.1f * sinf(frameTime * 0.317f);

  glm::mat4 entryModelMat;
  for (int i = 0; i < m_linesQty; i++)
  {
    entryModelMat = glm::rotate(modelMat, glm::radians(360.0f * i / m_linesQty), glm::vec3(0.0f, 0.0f, 1.0f));
    entryModelMat = glm::rotate(entryModelMat, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    entryModelMat = glm::translate(entryModelMat, glm::vec3(0.0f, 0.0f, 2.0f));
    for (int i = 0; i < m_pointsQty; i++)
    {
      m_modelMat = glm::rotate(entryModelMat, glm::radians(i * bend / m_pointsQty), glm::vec3(1.0f, 0.0f, 0.0f));
      m_modelMat = glm::translate(m_modelMat, glm::vec3(0.0f, 0.0f, i * stack / m_pointsQty));
      m_modelMat = glm::translate(m_modelMat, glm::vec3(twisttrans, 0.0f, 0.0f));
      m_modelMat = glm::rotate(m_modelMat, glm::radians(i * twist / m_pointsQty), glm::vec3(0.0f, 0.0f, 1.0f));
      m_modelMat = glm::translate(m_modelMat, glm::vec3(-twisttrans, 0.0f, 0.0f));

      glUniformMatrix4fv(m_modelViewMatLoc, 1, GL_FALSE, glm::value_ptr(m_modelMat));

      m_normalMat = glm::transpose(glm::inverse(glm::mat3(m_modelMat)));
      glUniformMatrix3fv(m_normalMatLoc, 1, GL_FALSE, glm::value_ptr(m_normalMat));

      switch (m_geometry)
      {
        case TRIANGLES:
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboHandle[3]);
          glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_index.size()), GL_UNSIGNED_INT, 0);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
          break;

        case POINTS:
          glDrawArrays(GL_POINTS, 0, 1);
          break;

        case BIGSPHERES:
          m_radius = 7 * exp (i / m_pointsQty * log (grow));
          UpdateCubeRadius();

          glBindBuffer(GL_ARRAY_BUFFER, m_vboHandle[1]);
          glBufferData(GL_ARRAY_BUFFER, m_vertex.size() * sizeof(glm::vec3), &m_vertex[0], GL_STATIC_DRAW);
          glBindBuffer(GL_ARRAY_BUFFER, 0);

          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboHandle[3]);
          glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_index.size()), GL_UNSIGNED_INT, 0);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
          break;

        case SPHERES:
          m_radius = exp (i / m_pointsQty * log (grow));
          UpdateCubeRadius();

          glBindBuffer(GL_ARRAY_BUFFER, m_vboHandle[1]);
          glBufferData(GL_ARRAY_BUFFER, m_vertex.size() * sizeof(glm::vec3), &m_vertex[0], GL_STATIC_DRAW);
          glBindBuffer(GL_ARRAY_BUFFER, 0);

          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboHandle[3]);
          glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_index.size()), GL_UNSIGNED_INT, 0);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
          break;
      }
    }
  }

  DisableShader();

  glDisableVertexAttribArray(m_hNormal);
  glDisableVertexAttribArray(m_hVertex);
  glDisableVertexAttribArray(m_hColor);

  if ((m_geometry == SPHERES) || (m_geometry == BIGSPHERES))
    glDisable(GL_CULL_FACE);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glDisable(GL_DEPTH_TEST);
}

void CScreensaverBiof::OnCompiledAndLinked()
{
  // Variables passed directly to the Vertex shader
  m_projMatLoc = glGetUniformLocation(ProgramHandle(), "u_projectionMatrix");
  m_modelViewMatLoc = glGetUniformLocation(ProgramHandle(), "u_modelViewMatrix");
  m_normalMatLoc = glGetUniformLocation(ProgramHandle(), "u_transposeAdjointModelViewMatrix");

  m_lightEnabledLoc = glGetUniformLocation(ProgramHandle(), "u_lightEnabled");
  m_light0_ambientLoc = glGetUniformLocation(ProgramHandle(), "u_light0.ambient");
  m_light0_diffuseLoc = glGetUniformLocation(ProgramHandle(), "u_light0.diffuse");
  m_light0_specularLoc = glGetUniformLocation(ProgramHandle(), "u_light0.specular");
  m_light0_positionLoc = glGetUniformLocation(ProgramHandle(), "u_light0.position");

#if defined(HAS_GLES)
  m_pointSizeLoc = glGetUniformLocation(ProgramHandle(), "u_pointSize");
#endif

  m_fogEnabledLoc = glGetUniformLocation(ProgramHandle(), "u_fogEnabled");
  m_fogColorLoc = glGetUniformLocation(ProgramHandle(), "u_fogColor");
  m_fogStartLoc = glGetUniformLocation(ProgramHandle(), "u_fogStart");
  m_fogEndLoc = glGetUniformLocation(ProgramHandle(), "u_fogEnd");

  m_hNormal = glGetAttribLocation(ProgramHandle(), "a_normal");
  m_hVertex = glGetAttribLocation(ProgramHandle(), "a_position");
  m_hColor = glGetAttribLocation(ProgramHandle(), "a_color");
}

bool CScreensaverBiof::OnEnabled()
{
  // This is called after glUseProgram()
  glUniformMatrix4fv(m_projMatLoc, 1, GL_FALSE, glm::value_ptr(m_projMat));
  glUniformMatrix4fv(m_modelViewMatLoc, 1, GL_FALSE, glm::value_ptr(m_modelMat));
  glUniformMatrix3fv(m_normalMatLoc, 1, GL_FALSE, glm::value_ptr(m_normalMat));

  glUniform1i(m_lightEnabledLoc, m_lightEnabled);
  glUniform4f(m_light0_ambientLoc, 0.0f, 0.0f, 0.0f, 1.0f); /* default value */
  glUniform4f(m_light0_diffuseLoc, m_lightDiffuseColor[0], m_lightDiffuseColor[1], m_lightDiffuseColor[2], m_lightDiffuseColor[3]); /* NOT default value */
  glUniform4f(m_light0_specularLoc, 1.0f, 1.0f, 1.0f, 1.0f); /* default value */
  glUniform4f(m_light0_positionLoc, m_lightPosition[0], m_lightPosition[1], m_lightPosition[2], m_lightPosition[3]); /* NOT default value */

#if defined(HAS_GLES)
  glUniform1f(m_pointSizeLoc, m_pointsSize);
#endif

  glUniform1f(m_fogEnabledLoc, m_fogEnabled);
  glUniform3f(m_fogColorLoc, m_fogColor[0], m_fogColor[1], m_fogColor[2]);
  glUniform1f(m_fogStartLoc, m_fogStart);
  glUniform1f(m_fogEndLoc, m_fogEnd);

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// update vertex positions only
///////////////////////////////////////////////////////////////////////////////
void CScreensaverBiof::UpdateCubeRadius()
{
  float scale = sqrtf(m_radius * m_radius / (m_vertex[0].x * m_vertex[0].x +
                                             m_vertex[0].y * m_vertex[0].y +
                                             m_vertex[0].z * m_vertex[0].z));
  for (auto& vtx : m_vertex)
    vtx *= scale;
}

///////////////////////////////////////////////////////////////////////////////
// build vertices of sphere with smooth shading using parametric equation
// x = r * cos(u) * cos(v)
// y = r * cos(u) * sin(v)
// z = r * sin(u)
// where u: stack(latitude) angle (-90 <= u <= 90)
//       v: sector(longitude) angle (0 <= v <= 360)
///////////////////////////////////////////////////////////////////////////////
void CScreensaverBiof::CreateCubeVerticesSmooth()
{
  static const float PI = 3.1415926f;

  float xy;                              // vertex position
  glm::vec3 position;
  glm::vec3 normal;
  float lengthInv = 1.0f / m_radius;    // normal

  float sectorStep = 2.0f * PI / float(m_cubeSectorCount);
  float stackStep = PI / float(m_cubeStackCount);
  float sectorAngle, stackAngle;

  for(int i = 0; i <= m_cubeStackCount; ++i)
  {
    stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
    xy = m_radius * cosf(stackAngle);             // r * cos(u)
    position.z = m_radius * sinf(stackAngle);              // r * sin(u)

    // add (m_cubeSectorCount+1) vertices per stack
    // the first and last vertices have same position and normal, but different tex coords
    for(int j = 0; j <= m_cubeSectorCount; ++j)
    {
      sectorAngle = j * sectorStep;           // starting from 0 to 2pi

      // vertex position
      position.x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
      position.y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
      m_vertex.push_back(position);

      // normalized vertex normal
      normal.x = position.x * lengthInv;
      normal.y = position.y * lengthInv;
      normal.z = position.z * lengthInv;
      m_normal.push_back(normal);

      m_color.push_back(std::move(glm::vec4(0.9f, 0.8f, 0.5f, 1.0f)));
    }
  }

  // m_index
  //  k1--k1+1
  //  |  / |
  //  | /  |
  //  k2--k2+1
  unsigned int k1, k2;
  for(int i = 0; i < m_cubeStackCount; ++i)
  {
    k1 = i * (m_cubeSectorCount + 1);     // beginning of current stack
    k2 = k1 + m_cubeSectorCount + 1;      // beginning of next stack

    for(int j = 0; j < m_cubeSectorCount; ++j, ++k1, ++k2)
    {
      // 2 triangles per sector excluding 1st and last stacks
      if(i != 0)
      {
        m_index.push_back(k1);
        m_index.push_back(k2);
        m_index.push_back(k1+1); // k1---k2---k1+1
      }

      if(i != (m_cubeStackCount-1))
      {
        m_index.push_back(k1+1);
        m_index.push_back(k2);
        m_index.push_back(k2+1); // k1+1---k2---k2+1
      }
    }
  }
}

ADDONCREATOR(CScreensaverBiof);
