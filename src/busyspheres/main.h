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

#pragma once

#include <kodi/addon-instance/Screensaver.h>
#include <kodi/gui/gl/GL.h>
#include <kodi/gui/gl/Shader.h>
#include <glm/gtc/type_ptr.hpp>

#define DEFAULT_NRPOINTS 100
#define MAX_NRPOINTS     200

#define EFFECT_RANDOM    0
#define EFFECT_CRESCENT  1
#define EFFECT_DOT       2
#define EFFECT_RING      3
#define EFFECT_LONGITUDE 4

struct sPosition
{
  sPosition() : x(0.0f), y(0.0f), z(0.0f), u(1.0f) {}
  sPosition(float x, float y, float z = 0.0f) : x(x), y(y), z(z), u(1.0f) {}
  float x,y,z,u;
};

struct sCoord
{
  sCoord() : s(0.0f), t(0.0f) {}
  sCoord(float s, float t) : s(s), t(t) {}
  float s,t;
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
  sCoord coord;
};

class ATTRIBUTE_HIDDEN CScreensaverBusySpheres
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver,
    public kodi::gui::gl::CShaderProgram
{
public:
  CScreensaverBusySpheres() = default;

  // kodi::addon::CInstanceScreensaver
  bool Start() override;
  void Stop() override;
  void Render() override;

  // kodi::gui::gl::CShaderProgram
  void OnCompiledAndLinked() override;
  bool OnEnabled() override;

private:
  void CalcPoints(float currentTime);

  bool m_startOK = false;
  int m_oldMode = 0, m_newMode;
  float m_convertTime = 0;
  float m_points[MAX_NRPOINTS][4];
  int m_pointsCnt = DEFAULT_NRPOINTS;
  int m_zoom = 2;

  GLuint m_texture_id = 0;

  glm::mat4 m_projMat;
  glm::mat4 m_modelMat;
  glm::mat4 m_entryMat;

  GLint m_projMatLoc = -1;
  GLint m_modelViewMatLoc = -1;
  GLint m_aPosition = -1;
  GLint m_aCoord = -1;
  GLint m_aColor = -1;

  GLuint m_vertexVBO = 0;
  GLuint m_indexVBO = 0;

  GLubyte m_idx[4] = {0, 1, 3, 2};

  glm::vec3 m_fb_buffer[MAX_NRPOINTS*37];
};
