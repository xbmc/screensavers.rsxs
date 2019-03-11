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

#pragma once

#include <kodi/addon-instance/Screensaver.h>
#include <kodi/gui/gl/GL.h>
#include <kodi/gui/gl/Shader.h>

#include <rsMath/rsMath.h>
#include <glm/gtc/type_ptr.hpp>

#define HVCtrX 0.0
#define HVCtrY 0.0
#define FireFoc 2.0f

#define FireFocX FireFoc
#define FireFocY FireFoc

#define ProjEX(p) (HVCtrX+FireFocX*(p).v[0]/(p).v[1])
#define ProjEY(p) (HVCtrY+FireFocY*(p).v[2]/(p).v[1])
#define ProjEZ(ez,p) (ez)=-((p).v[1]*(float)(1.0/20.0)-1.0)

#define NBPARTMAX 2048
#define PARTLIFE 2.0
#define PARTINTERV (PARTLIFE/NBPARTMAX)
#define FIRESIZE 1.0f
#define FIREDS 0.7
#define FIREALPHA 0.15f
#define FIREDA 0.4f
#define FIRECYLR 0.2f
#define XSTD (4.0/3.0)

struct sPosition
{
  sPosition() : x(0.0f), y(0.0f), z(0.0f), u(1.0f) {}
  sPosition(float* d) : x(d[0]), y(d[1]), z(d[2]), u(1.0f) {}
  sPosition(float x, float y, float z = 0.0f) : x(x), y(y), z(z), u(1.0f) {}
  float x,y,z,u;
};

struct sColor
{
  sColor() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
  sColor(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
  sColor& operator=(float* rhs)
  {
    r = rhs[0];
    g = rhs[1];
    b = rhs[2];
    return *this;
  }
  float r,g,b,a;
};

struct sLight
{
  sPosition vertex;
  sColor color;
};

class ATTRIBUTE_HIDDEN CScreensaverHufoSmoke
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver,
    public kodi::gui::gl::CShaderProgram
{
public:
  CScreensaverHufoSmoke() = default;

  bool Start() override;
  void Stop() override;
  void Render() override;

  void OnCompiledAndLinked() override;
  bool OnEnabled() override;

  void DrawEntry(int primitive, const sLight* data, unsigned int size);

private:
  void FireInit();
  void AffParticle(float ex, float ey, float dx, float dy, float a);
  void CalcFire(float t, float dt);

  rsVec m_fireSrc = rsVec(0.0f, 25.0f, -13.0f);
  rsVec m_fireDS1 = rsVec(6.0f, 0.0f, 0.0f);
  rsVec m_fireDS2 = rsVec(0.0f, 2.0f, 0.0f);
  rsVec m_fireDir = rsVec(0.0f, 0.0f, 10.0f);
  rsVec m_fPartA = rsVec(0.0f, 0.0f, 4.0f);

  rsMatrix m_fireM;
  rsVec m_fireO;

  struct Particle {
    rsVec p;    // position
    rsVec v;    // dp/dt
    rsVec s;    // size
    float a;    // alpha
    float t;    // time to death
    float ex, ey, ez;  // screen pos
    float dx, dy;    // screen size
  } m_tblP[NBPARTMAX];

  int m_np;
  float m_lastPartTime;
  bool m_fireAnim, m_fireRotate, m_fireRecalc;
  bool m_fireStop;
  float m_fireRot;
  float m_fireAng;

  bool m_affGrid = false;
  float m_tFire; // fire time

  sLight m_light[8];

  glm::mat4 m_modelProjMat;

  GLint m_modelViewProjectionMatrixLoc = -1;
  GLint m_hVertex = -1;
  GLint m_hColor = -1;

  GLuint m_vertexVBO = 0;

  bool m_startOK = false;
  double m_lastTime;
};
