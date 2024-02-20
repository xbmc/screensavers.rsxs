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

struct sLight
{
  glm::vec3 vertex;
  glm::vec2 coord;
  glm::vec4 color;
};

class ATTR_DLL_LOCAL CScreensaverSolarWinds
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

  GLuint m_vao = 0;
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
