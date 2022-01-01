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

struct sLight
{
  glm::vec3 vertex;
  glm::vec4 color;
  glm::vec2 coord;
};

class ATTR_DLL_LOCAL CScreensaverBusySpheres
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
  double m_startFrameTime;
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
