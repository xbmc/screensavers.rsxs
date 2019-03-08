/*
 *  Copyright (C) 2005-2019 Team Kodi
 *  Copyright (C) 2002 <hk@dgmr.nl>
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
 *   http://rss-glx.sourceforge.net/
 * and reworked to GL 4.0.
 */

#include "main.h"

#include <kodi/tools/Time.h>
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

  m_geometry = kodi::GetSettingInt("general.type");
  if (m_geometry == 0)
    m_geometry = rsRandi(4) + 1;

  int offAngle = kodi::GetSettingInt("general.offangle");
  if (offAngle == 0)
    m_offAngle = rsRandi(2);
  else if (offAngle == 1)
    m_offAngle = false;
  else
    m_offAngle = true;

  m_linesQty = kodi::GetSettingInt("general.lines");
  m_pointsQty = kodi::GetSettingInt("general.points");

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
  std::string fraqShader = kodi::GetAddonPath("resources/shaders/frag.glsl");
  std::string vertShader = kodi::GetAddonPath("resources/shaders/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
    return false;

  glGenBuffers(4, m_vboHandle);

  float glfLightPosition[4] = { 100.0, 100.0, 100.0, 0.0 };
  float glfFog[4] = { 0.0, 0.0, 0.3, 1.0 };
  float DiffuseLightColor[4] = { 1, 0.8, 0.4, 1.0 };
  int i, j;
  double x, y, r, d;
  unsigned char texbuf[64][64];

  glEnable(GL_DEPTH_TEST);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  std::vector<sPosition> normal;
  std::vector<sPosition> vertex;
  std::vector<sColor> color;
  std::vector<GLuint> index;
  if ((m_geometry == SPHERES) || (m_geometry == BIGSPHERES))
  {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    m_fogEnabled = false;
    m_lightEnabled = true;
  }
  else if (m_geometry == TRIANGLES)
  {
    normal.push_back(std::move(sPosition(0.0f, 0.0f, 0.0f)));
    vertex.push_back(std::move(sPosition(2.0f, 2.0f, 0.0f)));
    color.push_back(std::move(sColor(0.9f, 0.8f, 0.5f, 1.0f)));
    index.push_back(0);

    normal.push_back(std::move(sPosition(0.0f, 0.0f, 0.0f)));
    vertex.push_back(std::move(sPosition(0.0f, 2.0f, 0.0f)));
    color.push_back(std::move(sColor(0.9f, 0.8f, 0.5f, 0.0f)));
    index.push_back(1);

    normal.push_back(std::move(sPosition(0.0f, 0.0f, 0.0f)));
    vertex.push_back(std::move(sPosition(2.0f, 0.0f, 0.0f)));
    color.push_back(std::move(sColor(0.9f, 0.8f, 0.5f, 0.0f)));
    index.push_back(2);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboHandle[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*index.size(), &index[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    m_mode = GL_TRIANGLES;
    m_fogEnabled = false;
    m_lightEnabled = false;
  }
  else if (m_geometry == POINTS)
  {
#if !defined(HAS_GLES) && !defined(TARGET_DARWIN)
    // TODO: Bring in a way about in GLES 2.0 and above!
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glPointSize(3);
    glEnable(GL_POINT_SMOOTH);
#endif
    normal.push_back(std::move(sPosition(0.0f, 0.0f, 0.0f)));
    vertex.push_back(std::move(sPosition(0.0f, 0.0f, 0.0f)));
    color.push_back(std::move(sColor(0.9f, 0.8f, 0.5f, 1.0f)));
    index.push_back(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboHandle[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    m_mode = GL_POINTS;
    m_fogEnabled = true;
    m_lightEnabled = false;
  }

  m_nVerts = (GLuint)normal.size();

  glViewport(X(), Y(), (GLsizei)Width(), (GLsizei)Height());
  m_projMat = glm::perspective(glm::radians(30.0f), (float)Width() / (float)Height(), 100.0f, 300.0f);

  glBindBuffer(GL_ARRAY_BUFFER, m_vboHandle[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(sPosition)*normal.size(), &normal[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, m_vboHandle[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(sPosition)*vertex.size(), &vertex[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, m_vboHandle[2]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(sColor)*color.size(), &color[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

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
  glVertexAttribPointer(m_hNormal,  3, GL_FLOAT, GL_TRUE, sizeof(sPosition), ((GLubyte *)nullptr + (0)));
  glEnableVertexAttribArray(m_hNormal);

  glBindBuffer(GL_ARRAY_BUFFER, m_vboHandle[1]);
  glVertexAttribPointer(m_hVertex,  3, GL_FLOAT, GL_TRUE, sizeof(sPosition), ((GLubyte *)nullptr + (0)));
  glEnableVertexAttribArray(m_hVertex);

  glBindBuffer(GL_ARRAY_BUFFER, m_vboHandle[2]);
  glVertexAttribPointer(m_hColor,  4, GL_FLOAT, GL_TRUE, sizeof(sColor), ((GLubyte *)nullptr + (0)));
  glEnableVertexAttribArray(m_hColor);

  m_frameTime = kodi::time::GetTimeSec<double>();

  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glm::mat4 modelMat;
  modelMat = glm::lookAt(glm::vec3(200.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  modelMat = glm::rotate(modelMat, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  modelMat = glm::rotate(modelMat, glm::radians(float(10.0f * m_frameTime)), glm::vec3(0.0f, 0.0f, 1.0f));
  if (m_offAngle)
    modelMat = glm::rotate(modelMat, glm::radians(45.0f), glm::vec3(1.0f, 1.0f, 0.0f));

  EnableShader();

  float bend       =       315 * sin(m_frameTime * 0.237);
  float stack      =   50 + 20 * sin(m_frameTime * 0.133);
  float twist      =      1500 * sin(m_frameTime * 0.213);
  float twisttrans = 7.5 + 2.5 * sin(m_frameTime * 0.173);
  float grow       = 0.6 + 0.1 * sin(m_frameTime * 0.317);

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
        case POINTS:
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboHandle[3]);
          glDrawElements(m_mode, m_nVerts, GL_UNSIGNED_INT, 0);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
          break;

        case BIGSPHERES:
          Sphere(7 * exp (i / m_pointsQty * log (grow)), 20, 10);
          break;

        case SPHERES:
          Sphere(exp (i / m_pointsQty * log (grow)), 6, 4);
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

void CScreensaverBiof::Sphere(GLfloat radius, GLint nSlices, GLint nStacks)
{
  int nVerts = (nSlices+1) * (nStacks + 1);
  int elements = (nSlices * 2 * (nStacks-1) ) * 3;

  // Verts
  std::vector<GLfloat> p(4 * nVerts);
  // Normals
  std::vector<GLfloat> n(4 * nVerts);
  std::vector<GLfloat> c(4 * nVerts);
  // Elements
  std::vector<GLuint> el(elements);

  // Generate positions and normals
  GLfloat theta, phi;
  GLfloat thetaFac = glm::two_pi<float>() / nSlices;
  GLfloat phiFac = glm::pi<float>() / nStacks;
  GLfloat nx, ny, nz, s, t;
  GLuint idx = 0;
  for( GLuint i = 0; i <= nSlices; i++ )
  {
    theta = i * thetaFac;
        s = (GLfloat)i / nSlices;
    for( GLuint j = 0; j <= nStacks; j++ )
    {
      phi = j * phiFac;
        t = (GLfloat)j / nStacks;

      nx = sinf(phi) * cosf(theta);
      ny = sinf(phi) * sinf(theta);
      nz = cosf(phi);

      p[idx] = radius * nx;
      p[idx+1] = radius * ny;
      p[idx+2] = radius * nz;
      p[idx+3] = 1.0f;

      n[idx] = nx;
      n[idx+1] = ny;
      n[idx+2] = nz;
      n[idx+3] = 1.0f;

      c[idx] = 0.9f;
      c[idx+1] = 0.7f;
      c[idx+2] = 0.5f;
      c[idx+3] = 1.0f;
      idx += 4;
    }
  }

  // Generate the element list
  idx = 0;
  for( GLuint i = 0; i < nSlices; i++ )
  {
    GLuint stackStart = i * (nStacks + 1);
    GLuint nextStackStart = (i+1) * (nStacks+1);
    for( GLuint j = 0; j < nStacks; j++ )
    {
      if( j == 0 )
      {
        el[idx] = stackStart;
        el[idx+1] = stackStart + 1;
        el[idx+2] = nextStackStart + 1;
        idx += 3;
      }
      else if( j == nStacks - 1)
      {
        el[idx] = stackStart + j;
        el[idx+1] = stackStart + j + 1;
        el[idx+2] = nextStackStart + j;
        idx += 3;
      }
      else
      {
        el[idx] = stackStart + j;
        el[idx+1] = stackStart + j + 1;
        el[idx+2] = nextStackStart + j + 1;
        el[idx+3] = nextStackStart + j;
        el[idx+4] = stackStart + j;
        el[idx+5] = nextStackStart + j + 1;
        idx += 6;
      }
    }
  }

  glBindBuffer(GL_ARRAY_BUFFER, m_vboHandle[0]);
  glBufferData(GL_ARRAY_BUFFER, n.size() * sizeof(GLfloat), &n[0], GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, m_vboHandle[1]);
  glBufferData(GL_ARRAY_BUFFER, p.size() * sizeof(GLfloat), &p[0], GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, m_vboHandle[2]);
  glBufferData(GL_ARRAY_BUFFER, c.size() * sizeof(GLfloat), &c[0], GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboHandle[3]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, el.size() * sizeof(GLuint), &el[0], GL_DYNAMIC_DRAW);
  glDrawElements(GL_TRIANGLES, el.size(), GL_UNSIGNED_INT, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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

  glUniform1f(m_fogEnabledLoc, m_fogEnabled);
  glUniform3f(m_fogColorLoc, m_fogColor[0], m_fogColor[1], m_fogColor[2]);
  glUniform1f(m_fogStartLoc, m_fogStart);
  glUniform1f(m_fogEndLoc, m_fogEnd);

  return true;
}

ADDONCREATOR(CScreensaverBiof);
