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

#include <chrono>
#include <rsMath/rsMath.h>
#include <kodi/gui/gl/Texture.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#define TEXTURE_RGB 1
#define TEXTURE_RGBA 2
#define TEXTURE_ALPHA 3

#define PI M_PI
#define PIx2 (M_PI * 2)
#define DEG2RAD (M_PI / 180)
#define RAD2DEG (180 / M_PI)

namespace {

  // Modulo function for picking the correct element of lattice array
int myMod(int x){
  while(x < 0)
    x += LATSIZE;
  return(x % LATSIZE);
}

// start point, start slope, end point, end slope, position (0.0 - 1.0)
// returns point somewhere along a smooth curve between the start point
// and end point
float interpolate(float a, float b, float c, float d, float where){
  float q = 2.0f * (a - c) + b + d;
  float r = 3.0f * (c - a) - 2.0f * b - d;
  return((where * where * where * q) + (where * where * r) + (where * b) + a);
}

}

////////////////////////////////////////////////////////////////////////////
// Kodi has loaded us into memory, we should set our core values
// here and load any settings we may have from our config file
//
CScreensaverLattice::CScreensaverLattice()
{
  setDefaults(kodi::GetSettingInt("general.type"));
}

bool CScreensaverLattice::Start()
{
  int i, j, k;

  // Initialize pseudorandom number generator
  srand((unsigned)time(nullptr));

  std::string fraqShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/frag.glsl");
  std::string vertShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
    return false;

  // Window initialization
  glViewport(X(), Y(), Width()-X(), Height()-Y());

  if (m_settings.dTexture == 9)  // Choose random texture
    m_settings.dTexture = rsRandi(9);
  if (m_settings.dTexture != 2 && m_settings.dTexture != 6)  // No z-buffering for crystal or ghostly
    glEnable(GL_DEPTH_TEST);

  glFrontFace(GL_CCW);
  glEnable(GL_CULL_FACE);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  float x1 = cosf(float(m_settings.dFov) * 0.5f * DEG2RAD) / sinf(float(m_settings.dFov) * 0.5f * DEG2RAD);
  m_projMat = glm::mat4(x1, 0.0f, 0.0f, 0.0f,
                        0.0f, x1 * (float(Width())/float(Height())), 0.0f, 0.0f,
                        0.0f, 0.0f, -1.0f - 0.02f / float(m_settings.dDepth), -1.0f,
                        0.0f, 0.0f, -(0.02f + 0.0002f / float(m_settings.dDepth)), 0.0f);


  m_camera.init(glm::value_ptr(m_projMat), float(m_settings.dDepth));

   // Environment mapping for crystal, chrome, brass, shiny, and ghostly
  m_useSphere = (m_settings.dTexture == 2 || m_settings.dTexture == 3 ||
                 m_settings.dTexture == 4 || m_settings.dTexture == 5 ||
                 m_settings.dTexture == 6);

  // No lighting for chrome, brass, or ghostly
  m_useLighting = (m_settings.dTexture != 3 && m_settings.dTexture != 4 &&
                   m_settings.dTexture != 6);
  if (m_useLighting)
  {
    m_ambient[0]  = 0.0f;   m_ambient[1]  = 0.0f;   m_ambient[2]  = 0.0f;   m_ambient[3] = 0.0f;
    m_diffuse[0]  = 1.0f;   m_diffuse[1]  = 1.0f;   m_diffuse[2]  = 1.0f;   m_diffuse[3] = 0.0f;
    m_specular[0] = 1.0f;   m_specular[1] = 1.0f;   m_specular[2] = 1.0f;   m_specular[3] = 0.0f;
    m_position[0] = 400.0f; m_position[1] = 300.0f; m_position[2] = 500.0f; m_position[3] = 0.0f;
  }

  if (m_settings.dTexture == 0 || m_settings.dTexture == 5 || m_settings.dTexture == 7)
    m_shininess = 50.0f;
  else if (m_settings.dTexture == 2)
    m_shininess = 10.0f;
  else
    m_shininess = 0.0f;

  if (m_settings.dFog)
  {
    m_fogUseLinear = 1;
    m_fogStart = float(m_settings.dDepth) * 0.3f;
    m_fogEnd = float(m_settings.dDepth) - 0.1f;
  }
  else
  {
    m_fogUseLinear = 0;
    m_fogStart = 1.0;
    m_fogEnd = 1.0;
  }

  if (m_settings.dTexture == 2 || m_settings.dTexture == 6)  // Use blending for crystal and ghostly
  {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_BLEND);
  }
  else if (m_settings.dTexture == 7)  // Use blending for circuits
  {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
  }
  else
  {
    glDisable(GL_BLEND);
  }

  if (m_settings.dTexture)
    initTextures();

  // Initialize lattice objects and their positions in the lattice array
  m_segmentList.resize(NUMOBJECTS);
  makeLatticeObjects(m_segmentList);
  for (i = 0; i < LATSIZE; i++)
  {
    for (j = 0; j < LATSIZE; j++)
    {
      for (k=0; k<LATSIZE; k++)
      {
        m_lattice[i][j][k] = rsRandi(NUMOBJECTS);
      }
    }
  }

  // Initialize border points
  // horizontal border points
  for (i = 0; i < 6; i++)
  {
    for (j = 0; j < 6; j++)
    {
      m_bPnt[i][j] = 0.0f;
    }
  }
  m_bPnt[0][0] = 0.5f;    m_bPnt[0][1] = -0.25f;  m_bPnt[0][2] = 0.25f;  m_bPnt[0][3] = 1.0f; // right
  m_bPnt[1][0] = 0.5f;    m_bPnt[1][1] = 0.25f;   m_bPnt[1][2] = -0.25f; m_bPnt[1][3] = 1.0f; // right
  m_bPnt[2][0] = -0.25f;  m_bPnt[2][1] = 0.5f;    m_bPnt[2][2] = 0.25f;  m_bPnt[2][4] = 1.0f; // top
  m_bPnt[3][0] = 0.25f;   m_bPnt[3][1] = 0.5f;    m_bPnt[3][2] = -0.25f; m_bPnt[3][4] = 1.0f; // top
  m_bPnt[4][0] = -0.25f;  m_bPnt[4][1] = -0.25f;  m_bPnt[4][2] = 0.5f;   m_bPnt[4][5] = 1.0f; // front
  m_bPnt[5][0] = 0.25f;   m_bPnt[5][1] = 0.25f;   m_bPnt[5][2] = 0.5f;   m_bPnt[5][5] = 1.0f; // front

  // diagonal border points
  m_bPnt[6][0] = 0.5f;    m_bPnt[6][1] = -0.5f;   m_bPnt[6][2] = -0.5f;
  m_bPnt[6][3] = 1.0f;    m_bPnt[6][4] = -1.0f;   m_bPnt[6][5] = -1.0f;
  m_bPnt[7][0] = 0.5f;    m_bPnt[7][1] = 0.5f;    m_bPnt[7][2] = -0.5f;
  m_bPnt[7][3] = 1.0f;    m_bPnt[7][4] = 1.0f;    m_bPnt[7][5] = -1.0f;
  m_bPnt[8][0] = 0.5f;    m_bPnt[8][1] = -0.5f;   m_bPnt[8][2] = 0.5f;
  m_bPnt[8][3] = 1.0f;    m_bPnt[8][4] = -1.0f;   m_bPnt[8][5] = 1.0f;
  m_bPnt[9][0] = 0.5f;    m_bPnt[9][1] = 0.5f;    m_bPnt[9][2] = 0.5f;
  m_bPnt[9][3] = 1.0f;    m_bPnt[9][4] = 1.0f;    m_bPnt[9][5] = 1.0f;

  m_globalxyz[0] = 0;
  m_globalxyz[1] = 0;
  m_globalxyz[2] = 0;

  // Set up first path section
  m_path[0][0] = 0.0f;
  m_path[0][1] = 0.0f;
  m_path[0][2] = 0.0f;
  m_path[0][3] = 0.0f;
  m_path[0][4] = 0.0f;
  m_path[0][5] = 0.0f;

  j = rsRandi(12);
  k = j % 6;
  for (i = 0; i < 6; i++)
    m_path[1][i] = m_bPnt[k][i];
  if (j > 5) // If we want to head in a negative direction
  {
    i = k / 2;  // then we need to flip along the appropriate axis
    m_path[1][i] *= -1.0f;
    m_path[1][i+3] *= -1.0f;
  }
  m_lastBorder = k;
  m_segments = 1;

  glGenBuffers(1, &m_vertexVBO);
  m_lastTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  m_startOK = true;
  return true;
}

// Kodi tells us to stop the screensaver
// we should free any memory and release
// any resources we have created.
void CScreensaverLattice::Stop()
{
  if (!m_startOK)
    return;

  m_startOK = false;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  glDisable(GL_CULL_FACE);
  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);

  glDeleteBuffers(1, &m_vertexVBO);
  m_vertexVBO = 0;
}

void CScreensaverLattice::Render()
{
  if (!m_startOK)
    return;

  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);

  glVertexAttribPointer(m_aNormalLoc,  3, GL_FLOAT, GL_TRUE, sizeof(sLatticeSegmentEntry), BUFFER_OFFSET(offsetof(sLatticeSegmentEntry, normal)));
  glEnableVertexAttribArray(m_aNormalLoc);

  glVertexAttribPointer(m_aVertexLoc,  3, GL_FLOAT, GL_TRUE, sizeof(sLatticeSegmentEntry), BUFFER_OFFSET(offsetof(sLatticeSegmentEntry, vertex)));
  glEnableVertexAttribArray(m_aVertexLoc);

  glVertexAttribPointer(m_aColorLoc,  3, GL_FLOAT, GL_TRUE, sizeof(sLatticeSegmentEntry), BUFFER_OFFSET(offsetof(sLatticeSegmentEntry, color)));
  glEnableVertexAttribArray(m_aColorLoc);

  glVertexAttribPointer(m_aCoordLoc, 2, GL_FLOAT, GL_TRUE, sizeof(sLatticeSegmentEntry), BUFFER_OFFSET(offsetof(sLatticeSegmentEntry, coord)));
  glEnableVertexAttribArray(m_aCoordLoc);

  double currentTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  m_frameTime = static_cast<float>(currentTime - m_lastTime);
  m_lastTime = currentTime;

  if (m_settings.dTexture != 2 && m_settings.dTexture != 6)  // No z-buffering for crystal or ghostly
    glEnable(GL_DEPTH_TEST);

  glEnable(GL_CULL_FACE);

  if (m_settings.dTexture == 2 || m_settings.dTexture == 6)  // Use blending for crystal and ghostly
  {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_BLEND);
  }
  else if (m_settings.dTexture == 7)  // Use blending for circuits
  {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
  }

  int i, j, k;
  int indexx, indexy, indexz;
  rsVec xyz, dir, angvel, tempVec;
  static rsVec oldxyz(0.0f, 0.0f, 0.0f);
  static rsVec oldDir(0.0f, 0.0f, -1.0f);
  static rsVec oldAngvel(0.0f, 0.0f, 0.0f);
  float angle, distance;
  float rotMat[16];
  rsQuat newQuat;
  static rsQuat quat;
  static int flymode = 1;
  static float flymodeChange = 20.0f;
  static int seg = 0;  // Section of m_path
  static float where = 0.0f;  // Position on m_path
  static float rollVel = 0.0f, rollAcc = 0.0f;
  int drawDepth = m_settings.dDepth + 2;

  where += float(m_settings.dSpeed) * 0.05f * m_frameTime;
  if (where >= 1.0f)
  {
    where -= 1.0f;
    seg++;
  }
  if (seg >= m_segments)
  {
    seg = 0;
    reconfigure();
  }

  // Calculate position
  xyz[0] = interpolate(m_path[seg][0], m_path[seg][3], m_path[seg+1][0], m_path[seg+1][3], where);
  xyz[1] = interpolate(m_path[seg][1], m_path[seg][4], m_path[seg+1][1], m_path[seg+1][4], where);
  xyz[2] = interpolate(m_path[seg][2], m_path[seg][5], m_path[seg+1][2], m_path[seg+1][5], where);

  // Do rotation stuff
  dir = xyz - oldxyz;  // Direction of motion
  dir.normalize();
  angvel.cross(dir, oldDir);  // Desired axis of rotation
  float dot = oldDir.dot(dir);
  if (dot < -1.0f)
    dot = -1.0f;
  if (dot > 1.0f)
    dot = 1.0f;

  const float maxSpin(0.25f * float(m_settings.dSpeed) * m_frameTime);
  angle = acosf(dot);  // Desired turn angle
  if (angle > maxSpin)  // Cap the spin
    angle = maxSpin;
  if (angle < -maxSpin)
    angle = -maxSpin;

  angvel.scale(angle);  // Desired angular velocity
  tempVec = angvel - oldAngvel;  // Change in angular velocity
  distance = tempVec.length();  // Don't let angular velocity change too much
  const float rotationInertia(0.007f * float(m_settings.dSpeed) * m_frameTime);
  if (distance > rotationInertia)
  {
    tempVec.scale(rotationInertia / distance);
    angvel = oldAngvel + tempVec;
  }
  // transition from one fly mode to the other?
  flymodeChange -= m_frameTime;
  if (flymodeChange <= 1.0f)  // prepare to transition
    angvel.scale(flymodeChange);
  if (flymodeChange <= 0.0f)  // transition from one fly mode to the other?
  {
    flymode = rsRandi(4);
    flymodeChange = rsRandf(float(150 - m_settings.dSpeed)) + 5.0f;
  }

  // Recompute desired rotation
  tempVec = angvel;
  angle = tempVec.normalize();
  newQuat.make(angle, tempVec[0], tempVec[1], tempVec[2]);  // Use rotation

  if (flymode)  // fly normal (straight)
    quat.preMult(newQuat);
  else  // don't fly normal (go backwards and stuff)
    quat.postMult(newQuat);

  // Roll
  static float rollChange = rsRandf (10.0f) + 2.0f;
  rollChange -= m_frameTime;
  if (rollChange <= 0.0f)
  {
    rollAcc = rsRandf(0.02f * float(m_settings.dSpeed)) - (0.01f * float(m_settings.dSpeed));
    rollChange = rsRandf(10.0f) + 2.0f;
  }

  rollVel += rollAcc * m_frameTime;
  if (rollVel > (0.04f * float(m_settings.dSpeed)) && rollAcc > 0.0f)
    rollAcc = 0.0f;
  if (rollVel < (-0.04f * float(m_settings.dSpeed)) && rollAcc < 0.0f)
    rollAcc = 0.0f;

  newQuat.make(rollVel * m_frameTime, oldDir[0], oldDir[1], oldDir[2]);
  quat.preMult(newQuat);
  quat.toMat(rotMat);

  // Save old stuff
  oldxyz = xyz;
  oldDir[0] = -rotMat[2];
  oldDir[1] = -rotMat[6];
  oldDir[2] = -rotMat[10];
  oldAngvel = angvel;

  // Apply transformations
  glm::mat4 modelMat = glm::translate(glm::mat4(rotMat[0],  rotMat[1],  rotMat[2],  rotMat[3],
                                                rotMat[4],  rotMat[5],  rotMat[6],  rotMat[7],
                                                rotMat[8],  rotMat[9],  rotMat[10], rotMat[11],
                                                rotMat[12], rotMat[13], rotMat[14], rotMat[15]),
                                      glm::vec3(-xyz[0], -xyz[1], -xyz[2]));

  // Render everything
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindTexture(GL_TEXTURE_2D, m_texture_id[0]);
  for (i = m_globalxyz[0]-drawDepth; i <= m_globalxyz[0]+drawDepth; i++)
  {
    for (j = m_globalxyz[1]-drawDepth; j <= m_globalxyz[1]+drawDepth; j++)
    {
      for (k = m_globalxyz[2]-drawDepth; k <= m_globalxyz[2]+drawDepth; k++)
      {
        tempVec[0] = float(i) - xyz[0];
        tempVec[1] = float(j) - xyz[1];
        tempVec[2] = float(k) - xyz[2];

        float tpos[3];  // transformed position
        tpos[0] = tempVec[0] * rotMat[0] + tempVec[1] * rotMat[4] + tempVec[2] * rotMat[8];
        tpos[1] = tempVec[0] * rotMat[1] + tempVec[1] * rotMat[5] + tempVec[2] * rotMat[9];
        tpos[2] = tempVec[0] * rotMat[2] + tempVec[1] * rotMat[6] + tempVec[2] * rotMat[10];
        if (m_camera.inViewVolume(tpos, 0.9f))
        {
          indexx = myMod(i);
          indexy = myMod(j);
          indexz = myMod(k);
          glm::mat4 modelLattice = glm::translate(modelMat, glm::vec3(float(i), float(j), float(k)));

          // draw it
          for (const auto& entries : m_segmentList[m_lattice[indexx][indexy][indexz]])
          {
            EnableShader();
            m_modelMat = modelLattice * entries.matrix;

            if (m_settings.dTexture == 1 || m_settings.dTexture == 9)
              glBindTexture(GL_TEXTURE_2D, entries.texture);

            m_normalMat = glm::transpose(glm::inverse(glm::mat3(m_modelMat)));
            for (const auto& entry : entries.entries)
            {
              glBufferData(GL_ARRAY_BUFFER, sizeof(sLatticeSegmentEntry)*entry.size(), &entry[0], GL_DYNAMIC_DRAW);
              glDrawArrays(GL_TRIANGLE_STRIP, 0, entry.size());
            }

            if (m_settings.dTexture == 1 || m_settings.dTexture == 9)
              glBindTexture(GL_TEXTURE_2D, 0);
            DisableShader();
          }
        }
      }
    }
  }

  glDisableVertexAttribArray(m_aNormalLoc);
  glDisableVertexAttribArray(m_aVertexLoc);
  glDisableVertexAttribArray(m_aColorLoc);
  glDisableVertexAttribArray(m_aCoordLoc);

  glDisable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ZERO);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
}

void CScreensaverLattice::OnCompiledAndLinked()
{
  // Variables passed directly to the Vertex shader
  m_uProjMatLoc = glGetUniformLocation(ProgramHandle(), "u_projectionMatrix");
  m_uModelViewMatLoc = glGetUniformLocation(ProgramHandle(), "u_modelViewMatrix");
  m_uNormalMatLoc = glGetUniformLocation(ProgramHandle(), "u_transposeAdjointModelViewMatrix");

  m_uUseSphereLoc = glGetUniformLocation(ProgramHandle(), "u_useSphere");
  m_uUseLightingLoc = glGetUniformLocation(ProgramHandle(), "u_useLighting");
  m_uLightSource_ambientLoc = glGetUniformLocation(ProgramHandle(), "u_light.ambient");
  m_uLightSource_diffuseLoc = glGetUniformLocation(ProgramHandle(), "u_light.diffuse");
  m_uLightSource_specularLoc = glGetUniformLocation(ProgramHandle(), "u_light.specular");
  m_uLightSource_positionLoc = glGetUniformLocation(ProgramHandle(), "u_light.position");
  m_uLightSource_shininessLoc = glGetUniformLocation(ProgramHandle(), "u_light.shininess");

  m_uFog_useLoc = glGetUniformLocation(ProgramHandle(), "u_fog.use");
  m_uFog_startLoc = glGetUniformLocation(ProgramHandle(), "u_fog.start");
  m_uFog_endLoc = glGetUniformLocation(ProgramHandle(), "u_fog.end");

  // Smooth shading?
  m_uUseFlatLoc = glGetUniformLocation(ProgramHandle(), "u_useFlat");
  m_uTextureIdLoc = glGetUniformLocation(ProgramHandle(), "u_textureId");
  m_uMixTextureLoc = glGetUniformLocation(ProgramHandle(), "u_mixTexture");
  m_uTextureStyleLoc = glGetUniformLocation(ProgramHandle(), "u_textureStyle");

  m_aNormalLoc = glGetAttribLocation(ProgramHandle(), "a_normal");
  m_aVertexLoc = glGetAttribLocation(ProgramHandle(), "a_position");
  m_aColorLoc = glGetAttribLocation(ProgramHandle(), "a_color");
  m_aCoordLoc = glGetAttribLocation(ProgramHandle(), "a_coord");
}

bool CScreensaverLattice::OnEnabled()
{
  // This is called after glUseProgram()
  glUniformMatrix4fv(m_uProjMatLoc, 1, GL_FALSE, glm::value_ptr(m_projMat));
  glUniformMatrix4fv(m_uModelViewMatLoc, 1, GL_FALSE, glm::value_ptr(m_modelMat));
  glUniformMatrix3fv(m_uNormalMatLoc, 1, GL_FALSE, glm::value_ptr(m_normalMat));

  glUniform1i(m_uUseSphereLoc, m_useSphere);
  glUniform1i(m_uUseLightingLoc, m_useLighting);
  glUniform3f(m_uLightSource_ambientLoc, m_ambient[0], m_ambient[1], m_ambient[2]);
  glUniform3f(m_uLightSource_diffuseLoc, m_diffuse[0], m_diffuse[1], m_diffuse[2]);
  glUniform3f(m_uLightSource_specularLoc, m_specular[0], m_specular[1], m_specular[2]);
  glUniform3f(m_uLightSource_positionLoc, m_position[0], m_position[1], m_position[2]);
  glUniform1f(m_uLightSource_shininessLoc, m_shininess);

  glUniform1i(m_uFog_useLoc, m_fogUseLinear);
  glUniform1f(m_uFog_startLoc, m_fogStart);
  glUniform1f(m_uFog_endLoc, m_fogEnd);

  // Smooth shading?
  glUniform1f(m_uUseFlatLoc, m_settings.dSmooth ? 0.0f : 1.0f);
  glUniform1i(m_uTextureIdLoc, m_settings.dTexture);
  glUniform1i(m_uMixTextureLoc, m_settings.dTexture == 5);
  glUniform1i(m_uTextureStyleLoc, m_textureStyle);

  return true;
}

void CScreensaverLattice::setDefaults(int which)
{
  switch(which)
  {
  default:
  case 0:  // Regular
    m_settings.dLongitude = 16;
    m_settings.dLatitude = 8;
    m_settings.dThick = 50;
    m_settings.dDensity = 50;
    m_settings.dDepth = 4;
    m_settings.dFov = 90;
    m_settings.dPathrand = 7;
    m_settings.dSpeed = 10;
    m_settings.dTexture = 0;
    m_settings.dSmooth = false;
    m_settings.dFog = true;
    break;
  case 1:  // Chainmail
    m_settings.dLongitude = 24;
    m_settings.dLatitude = 12;
    m_settings.dThick = 50;
    m_settings.dDensity = 80;
    m_settings.dDepth = 3;
    m_settings.dFov = 90;
    m_settings.dPathrand = 7;
    m_settings.dSpeed = 10;
    m_settings.dTexture = 3;
    m_settings.dSmooth = true;
    m_settings.dFog = true;
    break;
  case 2:  // Brass Mesh
    m_settings.dLongitude = 4;
    m_settings.dLatitude = 4;
    m_settings.dThick = 40;
    m_settings.dDensity = 50;
    m_settings.dDepth = 4;
    m_settings.dFov = 90;
    m_settings.dPathrand = 7;
    m_settings.dSpeed = 10;
    m_settings.dTexture = 4;
    m_settings.dSmooth = false;
    m_settings.dFog = true;
    break;
  case 3:  // Computer
    m_settings.dLongitude = 4;
    m_settings.dLatitude = 6;
    m_settings.dThick = 70;
    m_settings.dDensity = 90;
    m_settings.dDepth = 4;
    m_settings.dFov = 90;
    m_settings.dPathrand = 7;
    m_settings.dSpeed = 10;
    m_settings.dTexture = 7;
    m_settings.dSmooth = false;
    m_settings.dFog = true;
    break;
  case 4:  // Slick
    m_settings.dLongitude = 24;
    m_settings.dLatitude = 12;
    m_settings.dThick = 100;
    m_settings.dDensity = 30;
    m_settings.dDepth = 4;
    m_settings.dFov = 90;
    m_settings.dPathrand = 7;
    m_settings.dSpeed = 10;
    m_settings.dTexture = 5;
    m_settings.dSmooth = true;
    m_settings.dFog = true;
    break;
  case 5:  // Tasty
    m_settings.dLongitude = 24;
    m_settings.dLatitude = 12;
    m_settings.dThick = 100;
    m_settings.dDensity = 25;
    m_settings.dDepth = 4;
    m_settings.dFov = 90;
    m_settings.dPathrand = 7;
    m_settings.dSpeed = 10;
    m_settings.dTexture = 8;
    m_settings.dSmooth = true;
    m_settings.dFog = true;
    break;
  case 6:  // Advanced settings
    m_settings.dLongitude = kodi::GetSettingInt("advanced.longitude");
    m_settings.dLatitude = kodi::GetSettingInt("advanced.latitude");
    m_settings.dThick = kodi::GetSettingInt("advanced.thickness");
    m_settings.dDensity = kodi::GetSettingInt("advanced.density");
    m_settings.dDepth = kodi::GetSettingInt("advanced.depth");
    m_settings.dFov = 90;
    m_settings.dPathrand = 7;
    m_settings.dSpeed = kodi::GetSettingInt("advanced.speed");
    m_settings.dTexture = kodi::GetSettingInt("advanced.texture");
    m_settings.dSmooth = kodi::GetSettingBoolean("advanced.smooth");
    m_settings.dFog = kodi::GetSettingBoolean("advanced.fog");
  }
}

void CScreensaverLattice::initTextures()
{
  switch(m_settings.dTexture)
  {
    case 1:
    m_texture_id[0] = kodi::gui::gl::CreateTexture(kodi::GetAddonPath("/resources/industrial1.dds"));
    m_texture_id[1] = kodi::gui::gl::CreateTexture(kodi::GetAddonPath("/resources/industrial2.dds"));
    m_textureStyle = TEXTURE_RGB;
    break;
  case 2:
    m_texture_id[0] = kodi::gui::gl::CreateTexture(kodi::GetAddonPath("/resources/crystal.dds"));
    m_textureStyle = TEXTURE_RGB;
    break;
  case 3:
    m_texture_id[0] = kodi::gui::gl::CreateTexture(kodi::GetAddonPath("/resources/chrome.dds"));
    m_textureStyle = TEXTURE_RGB;
    break;
  case 4:
    m_texture_id[0] = kodi::gui::gl::CreateTexture(kodi::GetAddonPath("/resources/brass.dds"));
    m_textureStyle = TEXTURE_RGB;
    break;
  case 5:
    m_texture_id[0] = kodi::gui::gl::CreateTexture(kodi::GetAddonPath("/resources/shiny.dds"));
    m_textureStyle = TEXTURE_RGBA;
    break;
  case 6:
    m_texture_id[0] = kodi::gui::gl::CreateTexture(kodi::GetAddonPath("/resources/ghostly.dds"));
    m_textureStyle = TEXTURE_ALPHA;
    break;
  case 7:
    m_texture_id[0] = kodi::gui::gl::CreateTexture(kodi::GetAddonPath("/resources/circuits.dds"));
    m_textureStyle = TEXTURE_ALPHA;
    break;
  case 8:
    m_texture_id[0] = kodi::gui::gl::CreateTexture(kodi::GetAddonPath("/resources/doughnuts.dds"));
    m_textureStyle = TEXTURE_RGBA;
    break;
  default:
    break;
  }
}

void CScreensaverLattice::reconfigure()
{
  static int transitions[20][6] =
  {
    1, 2, 12, 4, 14, 8,
    0, 3, 15, 7, 7, 7,
    3, 4, 14, 0, 7, 16,
    2, 1, 15, 7, 7, 7,
    5, 10, 12, 17, 17, 17,
    4, 3, 13, 11, 9, 17,
    12, 4, 10, 17, 17, 17,
    2, 0, 14, 8, 16, 19,
    1, 3, 15, 7, 7, 7,
    4, 10, 12, 17, 17, 17,
    11, 4, 12, 17, 17, 17,
    10, 5, 15, 13, 17, 18,
    13, 10, 4, 17, 17, 17,
    12, 1, 11, 5, 6, 17,
    15, 2, 12, 0, 7, 19,
    14, 3, 1, 7, 7, 7,
    3, 1, 15, 7, 7, 7,
    5, 11, 13, 6, 9, 18,
    10, 4, 12, 17, 17, 17,
    15, 1, 3, 7, 7, 7
  };

  int i, j;
  int newBorder, positive;

  // End of old m_path = start of new m_path
  for (i = 0; i < 6; i++)
    m_path[0][i] = m_path[m_segments][i];

  // determine if direction of motion is positive or negative
  // update global position
  if (m_lastBorder < 6)
  {
    if ((m_path[0][3] + m_path[0][4] + m_path[0][5]) > 0.0f)
    {
      positive = 1;
      m_globalxyz[m_lastBorder / 2] ++;
    }
    else
    {
      positive = 0;
      m_globalxyz[m_lastBorder / 2] --;
    }
  }
  else
  {
    if (m_path[0][3] > 0.0f)
    {
      positive = 1;
      m_globalxyz[0] ++;
    }
    else
    {
      positive = 0;
      m_globalxyz[0] --;
    }
    if (m_path[0][4] > 0.0f)
      m_globalxyz[1] ++;
    else
      m_globalxyz[1] --;
    if (m_path[0][5] > 0.0f)
      m_globalxyz[2] ++;
    else
      m_globalxyz[2] --;
  }

  if (!rsRandi(11 - m_settings.dPathrand))  // Change directions
  {
    if (!positive)
      m_lastBorder += 10;
    newBorder = transitions[m_lastBorder][rsRandi(6)];
    positive = 0;
    if (newBorder < 10)
      positive = 1;
    else
      newBorder -= 10;
    for (i = 0; i < 6; i++)  // set the new border point
      m_path[1][i] = m_bPnt[newBorder][i];
    if (!positive)  // flip everything if direction is negative
    {
      if (newBorder < 6)
        m_path[1][newBorder/2] *= -1.0f;
      else
        for (i = 0; i < 3; i++)
          m_path[1][i] *= -1.0f;
      for (i=3; i < 6; i++)
        m_path[1][i] *= -1.0f;
    }
    for (i = 0; i < 3; i++)  // reposition the new border
      m_path[1][i] += m_globalxyz[i];
    m_lastBorder = newBorder;
    m_segments = 1;
  }
  else
  {  // Just keep going straight
    newBorder = m_lastBorder;
    for (i = 0; i < 6; i++)
      m_path[1][i] = m_bPnt[newBorder][i];
    i = newBorder / 2;
    if (!positive)
    {
      if (newBorder < 6)
        m_path[1][i] *= -1.0f;
      else
      {
        m_path[1][0] *= -1.0f;
        m_path[1][1] *= -1.0f;
        m_path[1][2] *= -1.0f;
      }
      m_path[1][3] *= -1.0f;
      m_path[1][4] *= -1.0f;
      m_path[1][5] *= -1.0f;
    }
    for (j = 0; j < 3; j++)
    {
      m_path[1][j] += m_globalxyz[j];
      if ((newBorder < 6) && (j != 1))
        m_path[1][j] += rsRandf(0.15f) - 0.075f;
    }
    if (newBorder >= 6)
      m_path[1][0] += rsRandf(0.1f) - 0.05f;
    m_segments = 1;
  }
}

void CScreensaverLattice::setMaterialAttribs(sLatticeSegment& segment)
{
  if (m_settings.dTexture == 0 || m_settings.dTexture >= 5)
    segment.color = { rsRandf(1.0f), rsRandf(1.0f), rsRandf(1.0f) };
  else
    segment.color = { 1.0f, 1.0f, 1.0f };

  if (m_settings.dTexture == 1)
    segment.texture = m_texture_id[rsRandi(2)];
  else if (m_settings.dTexture >= 2)
    segment.texture = m_texture_id[0];
}

//  Build the lattice display lists
void CScreensaverLattice::makeLatticeObjects(std::vector<SEGMENT>& segments)
{
  int d = 0;
  float thick = float(m_settings.dThick) * 0.001f;

  for (int i = 0; i < NUMOBJECTS; i++)
  {
    if (d < m_settings.dDensity)
    {
      sLatticeSegment segment;
      segment.matrix = glm::mat4(1.0f);
      setMaterialAttribs(segment);
      segment.matrix = glm::translate(segment.matrix, glm::vec3(-0.25f, -0.25f, -0.25f));
      if (rsRandi(2))
        segment.matrix = glm::rotate(segment.matrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
      makeTorus(segment, m_settings.dLongitude, m_settings.dLatitude, 0.36f - thick, thick);
      segments[i].push_back(std::move(segment));
    }
    d = (d + 37) % 100;
    if (d < m_settings.dDensity)
    {
      sLatticeSegment segment;
      segment.matrix = glm::mat4(1.0f);
      setMaterialAttribs(segment);
      segment.matrix = glm::translate(segment.matrix, glm::vec3(0.25f, -0.25f, -0.25f));
      if (rsRandi(2))
        segment.matrix = glm::rotate(segment.matrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
      else
        segment.matrix = glm::rotate(segment.matrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
      makeTorus(segment, m_settings.dLongitude, m_settings.dLatitude, 0.36f - thick, thick);
      segments[i].push_back(std::move(segment));
    }
    d = (d + 37) % 100;
    if (d < m_settings.dDensity)
    {
      sLatticeSegment segment;
      segment.matrix = glm::mat4(1.0f);
      setMaterialAttribs(segment);
      segment.matrix = glm::translate(segment.matrix, glm::vec3(0.25f, -0.25f, 0.25f));
      if (rsRandi(2))
        segment.matrix = glm::rotate(segment.matrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
      else
        segment.matrix = glm::rotate(segment.matrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
      makeTorus(segment, m_settings.dLongitude, m_settings.dLatitude, 0.36f - thick, thick);
      segments[i].push_back(std::move(segment));
    }
    d = (d + 37) % 100;
    if (d < m_settings.dDensity)
    {
      sLatticeSegment segment;
      segment.matrix = glm::mat4(1.0f);
      setMaterialAttribs(segment);
      segment.matrix = glm::translate(segment.matrix, glm::vec3(0.25f, 0.25f, 0.25f));
      if (rsRandi(2))
        segment.matrix = glm::rotate(segment.matrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
      makeTorus(segment, m_settings.dLongitude, m_settings.dLatitude, 0.36f - thick, thick);
      segments[i].push_back(std::move(segment));
    }
    d = (d + 37) % 100;
    if (d < m_settings.dDensity)
    {
      sLatticeSegment segment;
      segment.matrix = glm::mat4(1.0f);
      setMaterialAttribs(segment);
      segment.matrix = glm::translate(segment.matrix, glm::vec3(-0.25f, 0.25f, 0.25f));
      if (rsRandi(2))
        segment.matrix = glm::rotate(segment.matrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
      else
        segment.matrix = glm::rotate(segment.matrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
      makeTorus(segment, m_settings.dLongitude, m_settings.dLatitude, 0.36f - thick, thick);
      segments[i].push_back(std::move(segment));
    }
    d = (d + 37) % 100;
    if (d < m_settings.dDensity)
    {
      sLatticeSegment segment;
      segment.matrix = glm::mat4(1.0f);
      setMaterialAttribs(segment);
      segment.matrix = glm::translate(segment.matrix, glm::vec3(-0.25f, 0.25f, -0.25f));
      if (rsRandi(2))
        segment.matrix = glm::rotate(segment.matrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
      else
        segment.matrix = glm::rotate(segment.matrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
      makeTorus(segment, m_settings.dLongitude, m_settings.dLatitude, 0.36f - thick, thick);
      segments[i].push_back(std::move(segment));
    }
    d = (d + 37) % 100;
  }
}

void CScreensaverLattice::makeTorus(sLatticeSegment& segment, int longitude, int latitude, float centerradius, float thickradius)
{
  float r, rr;  // Radius
  float z, zz;  // Depth
  float cosa, sina;  // Longitudinal positions
  float cosn, cosnn, sinn, sinnn;  // Normals for shading
  float ncosa, nsina;  // Longitudinal positions for shading
  float u, v1, v2;
  float temp;
  float oldcosa, oldsina, oldncosa, oldnsina, oldcosn, oldcosnn, oldsinn, oldsinnn;

  // Initialize texture stuff
  float vstep = 1.0f / static_cast<float>(latitude);
  float ustep = float(int((centerradius / thickradius) + 0.5f)) / float(longitude);
  v2 = 0.0f;

  sLatticeSegmentEntry entry;
  for (int i = 0; i < latitude; i++)
  {
    temp = PIx2 * float(i) / float(latitude);
    cosn = cosf(temp);
    sinn = sinf(temp);
    temp = PIx2 * float(i+1) / float(latitude);
    cosnn = cosf(temp);
    sinnn = sinf(temp);
    r = centerradius + thickradius * cosn;
    rr = centerradius + thickradius * cosnn;
    z = thickradius * sinn;
    zz = thickradius * sinnn;
    if (!m_settings.dSmooth)  // Redefine normals for flat shaded model
    {
      temp = PIx2 * (float(i) + 0.5f) / float(latitude);
      cosn = cosnn = cosf(temp);
      sinn = sinnn = sinf(temp);
    }
    v1 = v2;
    v2 += vstep;
    u = 0.0f;

    std::vector<sLatticeSegmentEntry> entries;
    for (int j = 0; j < longitude; j++)
    {
      temp = PIx2 * float(j) / float(longitude);
      cosa = cosf(temp);
      sina = sinf(temp);
      if (m_settings.dSmooth)
      {
        ncosa = cosa;
        nsina = sina;
      }
      else  // Redefine longitudinal component of normal for flat shading
      {
        temp = PIx2 * (float(j) - 0.5f) / float(longitude);
        ncosa = cosf(temp);
        nsina = sinf(temp);
      }

      if (j==0)  // Save first values for end of circular tri-strip
      {
        oldcosa = cosa;
        oldsina = sina;
        oldncosa = ncosa;
        oldnsina = nsina;
        oldcosn = cosn;
        oldcosnn = cosnn;
        oldsinn = sinn;
        oldsinnn = sinnn;
      }

      entry.color = segment.color;
      entry.normal.x = cosnn * ncosa;
      entry.normal.y = cosnn * nsina;
      entry.normal.z = sinnn;
      entry.coord.u = u;
      entry.coord.v = v2;
      entry.vertex.x = cosa * rr;
      entry.vertex.y = sina * rr;
      entry.vertex.z = zz;
      entries.push_back(entry);

      entry.color = segment.color;
      entry.normal.x = cosn * ncosa;
      entry.normal.y = cosn * nsina;
      entry.normal.z = sinn;
      entry.coord.u = u;
      entry.coord.v = v1;
      entry.vertex.x = cosa * r;
      entry.vertex.y = sina * r;
      entry.vertex.z = z;
      entries.push_back(entry);

      u += ustep;  // update u texture coordinate
    }
    //  Finish off circular tri-strip with saved first values
    entry.color = segment.color;
    entry.normal.x = oldcosnn * oldncosa;
    entry.normal.y = oldcosnn * oldnsina;
    entry.normal.z = oldsinnn;
    entry.coord.u = u;
    entry.coord.v = v2;
    entry.vertex.x = oldcosa * rr;
    entry.vertex.y = oldsina * rr;
    entry.vertex.z = zz;
    entries.push_back(entry);

    entry.color = segment.color;
    entry.normal.x = oldcosn * oldncosa;
    entry.normal.y = oldcosn * oldnsina;
    entry.normal.z = oldsinn;
    entry.coord.u = u;
    entry.coord.v = v1;
    entry.vertex.x = oldcosa * r;
    entry.vertex.y = oldsina * r;
    entry.vertex.z = z;
    entries.push_back(entry);

    segment.entries.push_back(entries);
  }
}

ADDONCREATOR(CScreensaverLattice);
