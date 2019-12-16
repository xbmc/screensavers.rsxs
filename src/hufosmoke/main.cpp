/*
 *  Copyright (C) 2005-2019 Team Kodi
 *  Copyright (C) 2000 Jeremie Allard (Hufo / N.A.A.)
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
#include "FMotion.h"

#include <chrono>
#include <kodi/gui/General.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace {

enum ColorType
{
  ColorAuto = 0,
  ColorWhite = 1,
  ColorManual = 2
};

float nrnd(double d) // normal distribution randomizer
{
  double r, v1, v2;

  do
  {
    v1 = rsRandf(2.0) - 1.0;
    v2 = rsRandf(2.0) - 1.0;
    r = v1 * v1 + v2 * v2;
  } while (r >= 1.0 || r == 0.0);

  r = sqrt (-2.0 * log (r) / r);

  return static_cast<float>(v1 * d * r);
}

struct sHufoSmokeSettings
{
  sHufoSmokeSettings()
  {
    setDefaults(ColorAuto);
  }

  void Load()
  {
    int type = ColorAuto;
    kodi::CheckSettingInt("color.type", type);
    setDefaults(type);
  }

  void setDefaults(int preset)
  {
    if (preset == ColorAuto)
    {
      do
      {
        frontRed = rsRandf(256) / 256.0f;
        frontGreen = rsRandf(256) / 256.0f;
        frontBlue = rsRandf(256) / 256.0f;

        backRed = rsRandi(256) / 256.0f;
        backGreen = rsRandi(256) / 256.0f;
        backBlue = rsRandi(256) / 256.0f;
      } while ((frontRed + frontGreen + frontBlue < 1) && (backRed + backGreen + backBlue < 1));
    }
    else if (preset == ColorWhite)
    {
      frontRed = frontGreen = frontBlue = 1.0f;
      backRed = backGreen = backBlue = 1.0f;
    }
    else
    {
      frontRed = kodi::GetSettingInt("color.foreground-red") / 256.0f;
      frontGreen = kodi::GetSettingInt("color.foreground-green") / 256.0f;
      frontBlue = kodi::GetSettingInt("color.foreground-blue") / 256.0f;

      backRed = kodi::GetSettingInt("color.background-red") / 256.0f;
      backGreen = kodi::GetSettingInt("color.background-green") / 256.0f;
      backBlue = kodi::GetSettingInt("color.background-blue") / 256.0f;
    }
  }

  int mode;

  float frontRed;
  float frontGreen;
  float frontBlue;

  float backRed;
  float backGreen;
  float backBlue;
} gSettings;
}

bool CScreensaverHufoSmoke::Start()
{
  std::string fraqShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/frag.glsl");
  std::string vertShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
    return false;

  gSettings.Load();

  float x = (float)Width() / (float)Height();  // Correct the viewing ratio of the window in the X axis.

  // Window initialization
  glViewport(X(), Y(), Width(), Height());

  if (x > XSTD)
    m_modelProjMat = glm::ortho(-x, x, -1.0f, 1.0f);  // Reset to a 2D screen space.
  else
    m_modelProjMat = glm::ortho(-XSTD, XSTD, -XSTD / x, XSTD / x);  // Reset to a 2D screen space.

  glCullFace(GL_FRONT);  // reject fliped faces
  glEnable(GL_CULL_FACE);
  glDepthFunc(GL_LESS);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);

  glGenBuffers(1, &m_vertexVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);

  m_tFire = 0.0;
  FireInit();    // initialise fire

  m_lastTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  m_startOK = true;
  return true;
}

void CScreensaverHufoSmoke::Stop()
{
  if (!m_startOK)
    return;

  m_startOK = false;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &m_vertexVBO);
  m_vertexVBO = 0;

  // Kodi defaults
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glCullFace(GL_BACK);
  glDisable(GL_CULL_FACE);
}

void CScreensaverHufoSmoke::Render()
{
  if (!m_startOK)
    return;

  double currentTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  float frameTime = static_cast<float>(currentTime - m_lastTime);
  m_lastTime = currentTime;

  /*
   * Following Extra work done here in render to prevent problems with controls
   * from Kodi and during window moving.
   * TODO: Maybe add a separate interface call to inform about?
   */
  //@{
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
  glVertexAttribPointer(m_hVertex, 3, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, vertex)));
  glEnableVertexAttribArray(m_hVertex);

  glVertexAttribPointer(m_hColor, 4, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, color)));
  glEnableVertexAttribArray(m_hColor);
  //@}

  m_tFire += frameTime;
  CalcFire (m_tFire, frameTime);  // animate the fire

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  int n;

  glEnable(GL_BLEND);
  Particle *p = m_tblP;

  n = m_np;
  while (n)
  {
    if (p->dx)
      AffParticle (p->ex, p->ey, p->dx, p->dy, p->a);

    ++p;
    --n;
  }

  glDisable(GL_BLEND);

  if (m_affGrid)
    AffFMotion(this, m_fireSrc, m_fireM, m_fireO);

  glDisableVertexAttribArray(m_hVertex);
  glDisableVertexAttribArray(m_hColor);
}

void CScreensaverHufoSmoke::FireInit()
{
  //NoiseInit();
  m_np = 0;
  m_lastPartTime = 0.0f;
  m_fireAnim = true;
  m_fireRotate = true;
  m_fireRot = glm::pi<float>() / 5.0f;
  m_fireAng = 0.0f;
  m_fireStop = false;
  m_fireRecalc = true;
  FMotionInit();
/*  np=1;
  TblP[0].p=FireSrc;
  TblP[0].v.Zero();
  TblP[0].a=1.0;;
  TblP[0].s.x=2.0;
  TblP[0].s.y=2.0;
  TblP[0].s.z=2.0;
  TblP[0].t=-1;*/
}

#define fSQRT_3_2 0.8660254038f
void CScreensaverHufoSmoke::AffParticle(float ex, float ey, float dx, float dy, float a)
{
  float hdx = 0.5f * dx;
  float s32_dy = fSQRT_3_2 * dy;

  m_light[0].color = glm::vec4(gSettings.frontRed, gSettings.frontGreen, gSettings.frontBlue, a);
  m_light[0].vertex = glm::vec3(ex, ey, 0.0f);
  m_light[1].color = m_light[2].color = m_light[3].color = m_light[4].color =
  m_light[5].color = m_light[6].color = m_light[7].color = glm::vec4(gSettings.backRed, gSettings.backGreen, gSettings.backBlue, 0.0f);
  m_light[1].vertex = glm::vec3(ex - dx, ey, 0.0f);
  m_light[2].vertex = glm::vec3(ex - hdx, ey + s32_dy, 0.0f);
  m_light[3].vertex = glm::vec3(ex + hdx, ey + s32_dy, 0.0f);
  m_light[4].vertex = glm::vec3(ex + dx, ey, 0.0f);
  m_light[5].vertex = glm::vec3(ex + hdx, ey - s32_dy, 0.0f);
  m_light[6].vertex = glm::vec3(ex - hdx, ey - s32_dy, 0.0f);
  m_light[7].vertex = glm::vec3(ex - dx, ey, 0.0f);

  EnableShader();
  glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*8, m_light, GL_DYNAMIC_DRAW);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
  DisableShader();
}


void CScreensaverHufoSmoke::CalcFire(float t, float dt)
{
  int n;
  Particle *p = m_tblP;  //+1;

  n = 0;
  float da = pow (FIREDA, dt);
  //float ds = pow (FIREDS, dt);
  //rsVec v;

  if (m_fireAnim)
  {
    FMotionAnimate (dt);
    while (n < m_np)
    {
      if ((p->t -= dt) <= 0)   // kill it
      {
        *p = m_tblP[--m_np];
      }
      else
      {  // animate it
        //a=p->p; a.z-=5*t;
        //a=FPartA; //+Noise(a)*(p->p.z-FireSrc.z)*1.0; //a.x*=4.0f; a.y*=4.0f;
        p->v += m_fPartA * dt;
        //GetFMotion(p->p,a); p->v=a+FPartA;
        p->p += p->v * dt;
        //p->p+=FireDir*dt;
        //p->p.z+=dt;
        FMotionWarp (p->p, dt);
        //p->s*=ds;
        //p->s.x=0.67*FIRESIZE+0.4*FIRESIZE*sin(PI*(p->t)/PARTLIFE);
        //p->s.z=0.05*FIRESIZE+0.768*FIRESIZE*(1.0-p->t/PARTLIFE);
        p->a *= da;
        ++n;
        ++p;
      }
    }
    if (!m_fireStop)
      while (m_np < NBPARTMAX && t - m_lastPartTime >= PARTINTERV)
      {
        m_lastPartTime += (float)PARTINTERV;
        p = m_tblP + (m_np++);
        p->p = m_fireSrc + m_fireDS1 * nrnd(0.25f) + m_fireDS2 * nrnd(0.25f);
        p->v = m_fireDir;
        FMotionWarp (p->p, (t - m_lastPartTime));
        p->t = PARTLIFE + m_lastPartTime - t;
        float size = FIRESIZE * (0.5f + nrnd(0.5f));
        float alpha = FIREALPHA * (float)pow (size / FIRESIZE, 0.5f);

        p->s.v[0] = size;
        p->s.v[1] = size;
        p->s.v[2] = size;
        p->a = alpha * pow (FIREDA, (t - m_lastPartTime));
        p->s *= 0.5f + nrnd (0.5f);
      }
  }
  else
    m_lastPartTime += dt;

  n = 0;
  p = m_tblP;
  rsVec v;

  if (m_fireRotate)
  {
    m_fireAng += m_fireRot * dt;
    m_fireRecalc = true;
  }

  m_fireM.makeRotate(m_fireAng, 0.0, 0.0, 1.0);
  m_fireO = m_fireSrc;
  m_fireO.transVec(m_fireM);
  m_fireO = m_fireSrc - m_fireO;
  while (n < m_np)
  {
    v = p->p;
    v.transVec(m_fireM);
    v += m_fireO;
    v = p->p;
    if (v.v[1] > 1.0f)
    {
      p->ex = ProjEX (v);
      p->ey = ProjEY (v);
      ProjEZ (p->ez, v);
      p->dx = FireFocX * p->s.v[0] / v.v[1];
      p->dy = FireFocY * p->s.v[2] / v.v[1];
    }
    else
      p->dx = 0;
    ++n;
    ++p;
  }
}

void CScreensaverHufoSmoke::DrawEntry(int primitive, const sLight* data, unsigned int size)
{
  EnableShader();
  glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*size, data, GL_DYNAMIC_DRAW);
  glDrawArrays(primitive, 0, size);
  DisableShader();
}

void CScreensaverHufoSmoke::OnCompiledAndLinked()
{
  // Variables passed directly to the Vertex shader
  m_modelViewProjectionMatrixLoc = glGetUniformLocation(ProgramHandle(), "u_modelViewProjectionMatrix");
  m_hVertex = glGetAttribLocation(ProgramHandle(), "a_position");
  m_hColor = glGetAttribLocation(ProgramHandle(), "a_color");
}

bool CScreensaverHufoSmoke::OnEnabled()
{
  // This is called after glUseProgram()
  glUniformMatrix4fv(m_modelViewProjectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(m_modelProjMat));
  return true;
}

ADDONCREATOR(CScreensaverHufoSmoke);
