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
#include "mipmap.h"
#include "flare.h"
#include "causticTextures.h"
#include "wavyNormalCubeMaps.h"
#include "splinePath.h"
#include "tunnel.h"
#include "goo.h"
#include "stretchedParticle.h"
#include "starBurst.h"
#include "nebulamap.h"

#include <chrono>
#include <kodi/gui/General.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <rsMath/rsMath.h>

bool CScreensaverHyperspace::Start()
{
  std::string fraqShader = kodi::addon::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/frag.glsl");
  std::string vertShader = kodi::addon::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
    return false;

  // Seed random number generator
  srand((unsigned)time(nullptr));

  m_settings.Load();

  glViewport(X(), Y(), Width(), Height());
  m_aspectRatio = float(Width()) / float(Height());
  m_viewport = glm::ivec4(X(), Y(), Width(), Height());

  // setup projection matrix
  m_projMat = glm::perspective(glm::radians(float(m_settings.dFov)), m_aspectRatio, 0.001f, 200.0f);

  // Limit memory consumption because the Windows previewer is just too darn slow
  if (m_doingPreview)
  {
    m_settings.dResolution = 6;
    if (m_settings.dDepth > 3)
      m_settings.dDepth = 3;
  };

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
//   glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

  m_flare.Init(this);

  m_thePath = new CSplinePath((m_settings.dDepth * 2) + 6);

  if (m_settings.dUseTunnels)
    m_theTunnel = new CTunnel(this, m_thePath, 20);

  // To avoid popping, depth, which will be used for fogging, is set to
  // m_settings.dDepth * goo grid size - size of one goo cubelet
  m_depth = float(m_settings.dDepth) * 2.0f - 2.0f / float(m_settings.dResolution);

  if (m_settings.dUseGoo)
    m_theGoo = new CGoo(this, m_settings.dResolution, m_depth);

  m_stars = new CStretchedParticle*[m_settings.dStars];
  for (int i = 0; i < m_settings.dStars; i++)
  {
    m_stars[i] = new CStretchedParticle(this);
    m_stars[i]->SetRadius(rsRandf(float(m_settings.dStarSize) * 0.0005f) + float(m_settings.dStarSize) * 0.0005f);
    if (i % 10)  // usually bland stars
      m_stars[i]->SetColor(0.8f + rsRandf(0.2f), 0.8f + rsRandf(0.2f), 0.8f + rsRandf(0.2f), false);
    else  // occasionally a colorful one
      m_stars[i]->SetColor(0.3f + rsRandf(0.7f), 0.3f + rsRandf(0.7f), 0.3f + rsRandf(0.7f), true);
    m_stars[i]->SetPosition(rsRandf(2.0f * m_depth) - m_depth, rsRandf(4.0f) - 2.0f, rsRandf(2.0f * m_depth) - m_depth);
    m_stars[i]->SetFov(float(m_settings.dFov));
  }

  m_sunStar = new CStretchedParticle(this);
  m_sunStar->SetRadius(float(m_settings.dStarSize) * 0.004f);
  m_sunStar->SetPosition(0.0f, 2.0f, 0.0f);
  m_sunStar->SetFov(float(m_settings.dFov));

  m_theStarBurst = new CStarBurst(this);
  for (int i = 0; i < SB_NUM_STARS; i++)
    m_theStarBurst->Stars()[i]->SetRadius(rsRandf(float(m_settings.dStarSize) * 0.001f) + float(m_settings.dStarSize) * 0.001f);

  glGenTextures(1, &m_nebulatex);

  m_numAnimTexFrames = 20;

  // Init light texture
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_nebulatex);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  Build2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_RGB, NEBULAMAPSIZE, NEBULAMAPSIZE, GL_RGB, GL_UNSIGNED_BYTE, nebulamap);
  Build2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_RGB, NEBULAMAPSIZE, NEBULAMAPSIZE, GL_RGB, GL_UNSIGNED_BYTE, nebulamap);
  Build2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_RGB, NEBULAMAPSIZE, NEBULAMAPSIZE, GL_RGB, GL_UNSIGNED_BYTE, nebulamap);
  Build2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_RGB, NEBULAMAPSIZE, NEBULAMAPSIZE, GL_RGB, GL_UNSIGNED_BYTE, nebulamap);
  Build2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_RGB, NEBULAMAPSIZE, NEBULAMAPSIZE, GL_RGB, GL_UNSIGNED_BYTE, nebulamap);
  Build2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_RGB, NEBULAMAPSIZE, NEBULAMAPSIZE, GL_RGB, GL_UNSIGNED_BYTE, nebulamap);

  glGenBuffers(1, &m_vertexVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
  glGenBuffers(1, &m_indexVBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);

  m_first = true;
  m_textureTime = 0.0f;
  m_starBurstTime = 300.0f;  // burst after 5 minutes
  m_lastTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  m_startOK = true;
  return true;
}

void CScreensaverHyperspace::Stop()
{
  if (!m_startOK)
    return;
  m_startOK = false;

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glDeleteBuffers(1, &m_vertexVBO);
  m_vertexVBO = 0;
  glDeleteBuffers(1, &m_indexVBO);
  m_indexVBO = 0;

  delete m_sunStar;
  m_sunStar = nullptr;

  for (int i = 0; i < m_settings.dStars; i++)
    delete m_stars[i];
  delete m_stars;
  m_stars = nullptr;

  delete m_theStarBurst;
  m_theStarBurst = nullptr;

  // Free memory
  if (m_settings.dUseGoo)
    delete m_theGoo;
  if (m_settings.dUseTunnels)
  {
    delete m_theTunnel;
    delete m_theCausticTextures;
  }
  delete m_thePath;
  delete m_theWNCM;
}

void CScreensaverHyperspace::Render()
{
  if (!m_startOK)
    return;

  double currentTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  m_frameTime = static_cast<float>(currentTime - m_lastTime);
  m_lastTime = currentTime;

  /*
   * Following Extra work done here in render to prevent problems with controls
   * from Kodi and during window moving.
   * TODO: Maybe add a separate interface call to inform about?
   */
  //@{
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);

  glVertexAttribPointer(m_aPosition, 4, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, vertex)));
  glEnableVertexAttribArray(m_aPosition);

  glVertexAttribPointer(m_aNormal, 4, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, normal)));
  glEnableVertexAttribArray(m_aNormal);

  glVertexAttribPointer(m_aCoord, 2, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, coord)));
  glEnableVertexAttribArray(m_aCoord);

  glVertexAttribPointer(m_aColor, 4, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, color)));
  glEnableVertexAttribArray(m_aColor);
  //@}

  if (m_first)
  {
    if (m_settings.dUseTunnels)  // only tunnels use caustic textures
    {
      m_fogUsed = 0;
      // Caustic textures can only be created after rendering context has been created
      // because they have to be drawn and then read back from the framebuffer.
      if (m_doingPreview) // super fast for Windows previewer
        m_theCausticTextures = new CCausticTextures(this, 8, m_numAnimTexFrames, 32, 32, 1.0f, 0.01f, 10.0f);
      else  // normal
        m_theCausticTextures = new CCausticTextures(this, 8, m_numAnimTexFrames, 100, 256, 1.0f, 0.01f, 20.0f);
      m_fogUsed = 1;
    }

    if (m_doingPreview) // super fast for Windows previewer
      m_theWNCM = new CWavyNormalCubeMaps(m_numAnimTexFrames, 32);
    else  // normal
      m_theWNCM = new CWavyNormalCubeMaps(m_numAnimTexFrames, 128);

    glViewport(m_viewport[0], m_viewport[1], m_viewport[2], m_viewport[3]);
    m_first = false;
  }

  // Camera movements
  m_camHeadingChangeTime[1] += m_frameTime;
  if (m_camHeadingChangeTime[1] >= m_camHeadingChangeTime[0]){  // Choose new direction
    m_camHeadingChangeTime[0] = rsRandf(15.0f) + 5.0f;
    m_camHeadingChangeTime[1] = 0.0f;
    m_camHeading[2] = m_camHeading[1];  // last = target
    if (m_changeCamHeading)
    {
      // face forward most of the time
      if (rsRandi(6))
        m_camHeading[1] = 0.0f;
      // face backward the rest of the time
      else if (rsRandi(2))
        m_camHeading[1] = M_PI;
      else
        m_camHeading[1] = -M_PI;
      m_changeCamHeading = false;
    }
    else
      m_changeCamHeading = true;
  }
  float t = m_camHeadingChangeTime[1] / m_camHeadingChangeTime[0];
  t = 0.5f * (1.0f - cosf(M_PI * t));
  m_camHeading[0] = m_camHeading[1] * t + m_camHeading[2] * (1.0f - t);
  m_camRollChangeTime[1] += m_frameTime;
  if (m_camRollChangeTime[1] >= m_camRollChangeTime[0])  // Choose new roll angle
  {
    m_camRollChangeTime[0] = rsRandf(5.0f) + 10.0f;
    m_camRollChangeTime[1] = 0.0f;
    m_camRoll[2] = m_camRoll[1];  // last = target
    if (m_changeCamRoll)
    {
      m_camRoll[1] = rsRandf(RS_PIx2*2) - RS_PIx2;
      m_changeCamRoll = false;
    }
    else
      m_changeCamRoll = true;
  }
  t = m_camRollChangeTime[1] / m_camRollChangeTime[0];
  t = 0.5f * (1.0f - cosf(M_PI * t));
  m_camRoll[0] = m_camRoll[1] * t + m_camRoll[2] * (1.0f - t);

  m_thePath->MoveAlongPath(float(m_settings.dSpeed) * m_frameTime * 0.04f);
  m_thePath->Update(m_frameTime);
  m_thePath->GetPoint(m_settings.dDepth + 2, m_thePath->Step(), m_camPos);
  m_thePath->GetBaseDirection(m_settings.dDepth + 2, m_thePath->Step(), m_pathDir);
  float pathAngle = atan2f(-m_pathDir[0], -m_pathDir[2]);
  m_billboardMat = glm::rotate(glm::mat4(1.0f), glm::radians((pathAngle + m_camHeading[0]) * RS_RAD2DEG), glm::vec3(0, 1, 0));
  m_billboardMat = glm::rotate(m_billboardMat, glm::radians(m_camRoll[0] * RS_RAD2DEG), glm::vec3(0, 0, 1));

  m_modelMat = glm::rotate(glm::mat4(1.0f), glm::radians(-m_camRoll[0] * RS_RAD2DEG), glm::vec3(0, 0, 1));
  m_modelMat = glm::rotate(m_modelMat, glm::radians((-pathAngle - m_camHeading[0]) * RS_RAD2DEG), glm::vec3(0, 1, 0));
  m_modelMat = glm::translate(m_modelMat, glm::vec3(-m_camPos[0], -m_camPos[1], -m_camPos[2]));

  m_unroll = m_camRoll[0] * RS_RAD2DEG;

  if (m_settings.dUseGoo)
  {
    // calculate diagonal fov
    float diagFov = 0.5f * float(m_settings.dFov) / RS_RAD2DEG;
    diagFov = tanf(diagFov);
    diagFov = sqrtf(diagFov * diagFov + (diagFov * m_aspectRatio * diagFov * m_aspectRatio));
    diagFov = 2.0f * atanf(diagFov);
    m_theGoo->update(m_camPos[0], m_camPos[2], pathAngle + m_camHeading[0], diagFov);
  }

  // clear
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  // draw stars
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_flare.FlareTex()[0]);

  float temppos[2];
  for (int i = 0; i < m_settings.dStars; i++)
  {
    temppos[0] = m_stars[i]->Position()[0] - m_camPos[0];
    temppos[1] = m_stars[i]->Position()[2] - m_camPos[2];
    if (temppos[0] > m_depth)
    {
      m_stars[i]->Position()[0] -= m_depth * 2.0f;
      m_stars[i]->LastPosition()[0] -= m_depth * 2.0f;
    }
    else if (temppos[0] < -m_depth)
    {
      m_stars[i]->Position()[0] += m_depth * 2.0f;
      m_stars[i]->LastPosition()[0] += m_depth * 2.0f;
    }
    if (temppos[1] > m_depth)
    {
      m_stars[i]->Position()[2] -= m_depth * 2.0f;
      m_stars[i]->LastPosition()[2] -= m_depth * 2.0f;
    }
    else if (temppos[1] < -m_depth)
    {
      m_stars[i]->Position()[2] += m_depth * 2.0f;
      m_stars[i]->LastPosition()[2] += m_depth * 2.0f;
    }
    m_stars[i]->Draw(glm::vec3(m_camPos[0], m_camPos[1], m_camPos[2]));
  }
  glDisable(GL_CULL_FACE);

  // pick animated texture frame
  m_textureTime += m_frameTime;
  // loop frames every 2 seconds
  const float texFrameTime(2.0f / float(m_numAnimTexFrames));
  while(m_textureTime > texFrameTime)
  {
    m_textureTime -= texFrameTime;
    m_whichTexture ++;
  }
  while(m_whichTexture >= m_numAnimTexFrames)
    m_whichTexture -= m_numAnimTexFrames;

  // alpha component gets normalmap lerp value
  const float lerp = m_textureTime / texFrameTime;

  // draw goo
  if (m_settings.dUseGoo)
  {
    // calculate color
    float goo_rgb[4];
    for (int i = 0; i < 3; i++)
    {
      m_goo_rgb_phase[i] += m_goo_rgb_speed[i] * m_frameTime;
      if (m_goo_rgb_phase[i] >= RS_PIx2)
        m_goo_rgb_phase[i] -= RS_PIx2;
      goo_rgb[i] = sinf(m_goo_rgb_phase[i]);
      if (goo_rgb[i] < 0.0f)
        goo_rgb[i] = 0.0f;
    }

    goo_rgb[3] = lerp;
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_nebulatex);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_theWNCM->Texture()[(m_whichTexture + 1) % m_numAnimTexFrames]);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_theWNCM->Texture()[m_whichTexture]);
    glActiveTexture(GL_TEXTURE0);

    // draw it
    ShaderProgram(SHADER_GOO);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_BLEND);
    m_theGoo->draw(goo_rgb);
    ShaderProgram(SHADER_NORMAL);
  }

  // update starburst
  m_starBurstTime -= m_frameTime;
  if (m_starBurstTime <= 0.0f)
  {
    float pos[] = {m_camPos[0] + (m_pathDir[0] * m_depth * (0.5f + rsRandf(0.5f))),
      rsRandf(2.0f) - 1.0f,
      m_camPos[2] + (m_pathDir[2] * m_depth * (0.5f + rsRandf(0.5f)))};
    m_theStarBurst->Restart(pos);  // it won't actually restart unless it's ready to
    m_starBurstTime = rsRandf(540.0f) + 60.0f;  // burst again within 1-5 minutes
  }
  m_theStarBurst->Draw(lerp);

  // draw tunnel
  if (m_settings.dUseTunnels)
  {
    m_theTunnel->Make(m_frameTime);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    ShaderProgram(SHADER_TUNNEL);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_theCausticTextures->CausticTex()[(m_whichTexture + 1) % m_numAnimTexFrames]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_theCausticTextures->CausticTex()[m_whichTexture]);
    m_theTunnel->Draw(lerp);
    ShaderProgram(SHADER_NORMAL);

    glDisable(GL_CULL_FACE);
  }

  // draw sun with lens flare
  m_fogUsed = 0;
  BindTexture(GL_TEXTURE_2D, m_flare.FlareTex()[0]);
  m_sunStar->Draw(m_camPos);

  glm::vec3 flarepos(0.0f, 2.0f, 0.0f);
  float diff[3] = {flarepos[0] - m_camPos[0], flarepos[1] - m_camPos[1], flarepos[2] - m_camPos[2]};
  float alpha = 0.5f - 0.005f * sqrtf(diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2]);
  if (alpha > 0.0f)
    m_flare.Draw(flarepos, 1.0f, 1.0f, 1.0f, alpha);
  m_fogUsed = 1;

  glDisableVertexAttribArray(m_aPosition);
  glDisableVertexAttribArray(m_aNormal);
  glDisableVertexAttribArray(m_aCoord);
  glDisableVertexAttribArray(m_aColor);
}

void CScreensaverHyperspace::Draw(int primitive, const sLight* data, unsigned int size)
{
  m_uniformColorUsed = 0;
  EnableShader();
  glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*size, data, GL_DYNAMIC_DRAW);
  glDrawArrays(primitive, 0, size);
  DisableShader();
}

void CScreensaverHyperspace::Draw(const sColor& color, int primitive, const sLight* data, unsigned int size)
{
  m_uniformColorUsed = 1;
  m_uniformColor = color;
  EnableShader();
  glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*size, data, GL_DYNAMIC_DRAW);
  glDrawArrays(primitive, 0, size);
  DisableShader();
}

void CScreensaverHyperspace::Draw(const sColor& color, const float* vertices, unsigned int vertex_offset, const unsigned int* indices, unsigned int index_offset)
{
  int length = vertex_offset/6;
  m_surface.resize(length);
  for (int i = 0; i < length; ++i)
  {
    m_surface[i].normal.x = vertices[i*6+0];
    m_surface[i].normal.y = vertices[i*6+1];
    m_surface[i].normal.z = vertices[i*6+2];

    m_surface[i].vertex.x = vertices[i*6+3];
    m_surface[i].vertex.y = vertices[i*6+4];
    m_surface[i].vertex.z = vertices[i*6+5];
  }

  m_uniformColorUsed = 1;
  m_uniformColor = color;
  EnableShader();
  glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*length, m_surface.data(), GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_offset * sizeof(GLuint), &(indices[0]), GL_DYNAMIC_DRAW);
  glDrawElements(GL_TRIANGLES, index_offset, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  DisableShader();
}

void CScreensaverHyperspace::OnCompiledAndLinked()
{
  // Variables passed directly to the Vertex shader
  m_hProj = glGetUniformLocation(ProgramHandle(), "u_projectionMatrix");
  m_hModel = glGetUniformLocation(ProgramHandle(), "u_modelViewMatrix");

  u_uUniformColor = glGetUniformLocation(ProgramHandle(), "u_uniformColor");
  m_uUniformColorUsed = glGetUniformLocation(ProgramHandle(), "u_uniformColorUsed");
  m_uTextureUsed = glGetUniformLocation(ProgramHandle(), "u_textureUsed");
  m_uTexUnit0 = glGetUniformLocation(ProgramHandle(), "u_texUnit0");
  m_uTexUnit1 = glGetUniformLocation(ProgramHandle(), "u_texUnit1");
  m_uTexUnit2 = glGetUniformLocation(ProgramHandle(), "u_texUnit2");
  m_uNormaltex0 = glGetUniformLocation(ProgramHandle(), "u_normaltex0");
  m_uNormaltex1 = glGetUniformLocation(ProgramHandle(), "u_normaltex1");
  m_uTex = glGetUniformLocation(ProgramHandle(), "u_tex");

  m_uFogUsed = glGetUniformLocation(ProgramHandle(), "u_fogEnabled");
  m_uFogColor = glGetUniformLocation(ProgramHandle(), "u_fogColor");
  m_ufogStart = glGetUniformLocation(ProgramHandle(), "u_fogStart");
  m_ufogEnd = glGetUniformLocation(ProgramHandle(), "u_fogEnd");

  m_uActiveShader = glGetUniformLocation(ProgramHandle(), "u_activeShader");

  m_aPosition = glGetAttribLocation(ProgramHandle(), "a_position");
  m_aNormal = glGetAttribLocation(ProgramHandle(), "a_normal");
  m_aCoord = glGetAttribLocation(ProgramHandle(), "a_coord");
  m_aColor = glGetAttribLocation(ProgramHandle(), "a_color");
}

bool CScreensaverHyperspace::OnEnabled()
{
  // This is called after glUseProgram()
  glUniformMatrix4fv(m_hProj, 1, GL_FALSE, glm::value_ptr(m_projMat));
  glUniformMatrix4fv(m_hModel, 1, GL_FALSE, glm::value_ptr(m_modelMat));

  glUniform1i(m_uFogUsed, m_fogUsed);
  glUniform4f(m_uFogColor, 0.0f, 0.0f, 0.0f, 1.0f);
  glUniform1f(m_ufogStart, m_depth * 0.7f);
  glUniform1f(m_ufogEnd, m_depth);
  glUniform4f(u_uUniformColor, m_uniformColor.r, m_uniformColor.g, m_uniformColor.b, m_uniformColor.a);
  glUniform1i(m_uUniformColorUsed, m_uniformColorUsed);
  glUniform1i(m_uTextureUsed, m_textureUsed);
  glUniform1i(m_uTexUnit0, 0);
  glUniform1i(m_uTexUnit1, 1);
  glUniform1i(m_uTexUnit2, 2);
  glUniform1i(m_uNormaltex0, 3);
  glUniform1i(m_uNormaltex1, 4);
  glUniform1i(m_uTex, 5);
  glUniform1i(m_uActiveShader, m_activeShader);

  return true;
}

ADDONCREATOR(CScreensaverHyperspace);
