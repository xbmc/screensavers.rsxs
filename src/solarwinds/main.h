/*
 *  Copyright (C) 2005-2019 Team Kodi
 *  Copyright (C) 1999-2010 Terence M. Welsh
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
 *
 */

/*
 * Code is based on:
 *   https://github.com/reallyslickscreensavers/reallyslickscreensavers
 *   http://rss-glx.sourceforge.net/
 * and reworked to GL 4.0.
 */

#pragma once

#include <kodi/addon-instance/Screensaver.h>
#include <kodi/gui/gl/GL.h>
#include <kodi/gui/gl/Shader.h>

#include <glm/gtc/type_ptr.hpp>

#define LIGHTSIZE 64

class CWind;

enum
{
  ADVANCED_MODE = -1,
  AUTOMATIC_MODE = 0,
  PRESET_REGULAR,
  PRESET_COSMIC_STRINGS,
  PRESET_COLD_PRICKLIES,
  PRESET_SPACE_FLURE,
  PRESET_JIGGLY,
  PRESET_UNDERTOW,
};

// Parameters edited in the dialog box
struct sSettings
{
  int dWinds;
  int dEmitters;
  int dParticles;
  int dGeometry;
  int dSize;
  int dParticlespeed;
  int dEmitterspeed;
  int dWindspeed;
  int dBlur;
};

struct sPosition
{
  sPosition() : x(0.0f), y(0.0f), z(0.0f), u(1.0f) {}
  sPosition(float x, float y, float z) : x(x), y(y), z(z), u(1.0f) {}
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
  sColor(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
  float r,g,b,a;
};

struct sLight
{
  sPosition vertex;
  sCoord coord;
  sColor color;
};

class ATTRIBUTE_HIDDEN CScreensaverSolarWinds
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver,
    public kodi::gui::gl::CShaderProgram
{
public:
  CScreensaverSolarWinds();

  bool Start() override;
  void Stop() override;
  void Render() override;

  void OnCompiledAndLinked() override;
  bool OnEnabled() override;

  glm::mat4 m_projMat;
  glm::mat4 m_modelMat;

  sLight m_light[4];
  sLight m_blur[4];

private:
  void SetDefaults(int type);

  bool m_startOK = false;
  int m_startClearCnt = 5;
  unsigned int m_vertexVBO[2] = {0};
  CWind *m_winds;

  unsigned char m_lightTexture[LIGHTSIZE][LIGHTSIZE];

  GLint m_hProj = -1;
  GLint m_hModel = -1;
  GLint m_hPos = -1;
  GLint m_hCol = -1;
  GLint m_hCoord = -1;
  GLint m_hGeometry = -1;

  GLuint m_hwTexture = -1;
};
