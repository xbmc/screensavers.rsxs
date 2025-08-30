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
#include <math.h>
#include <kodi/gui/gl/GL.h>
#include <kodi/gui/gl/Shader.h>
#include <glm/glm.hpp>

#define TEXSIZE 1024
#define NUMCONSTS 18

// Find absolute value and truncate to 1.0
inline float fabstrunc(float f)
{
  if(f >= 0.0f)
    return(f <= 1.0f ? f : 1.0f);
  else
    return(f >= -1.0f ? -f : 1.0f);
}

class CGUIShader;

class ATTR_DLL_LOCAL CScreensaverPlasma
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver,
    public kodi::gui::gl::CShaderProgram
{
public:
  CScreensaverPlasma() = default;

  bool Start() override;
  void Stop() override;
  void Render() override;

  void OnCompiledAndLinked() override;
  bool OnEnabled() override { return true; }

private:
  void SetPlasmaSize();

  GLint m_hPos = -1;
  GLint m_hCord = -1;

  GLuint m_vao = 0;
  GLuint m_vertexVBO = 0;
  GLuint m_tex = 0;

  int m_plasmasize = 64;
  float m_aspectRatio = 16.0f / 9.0f;
  float m_focus = 30.0f / 50.0f + 0.3f;
  int m_zoom = 10;
  float m_maxdiff = 0.004f * 20.0f;
  int m_resolution = 25;

  float m_width;
  float m_height;
  float m_position[TEXSIZE][TEXSIZE][2];
  float m_plasma[TEXSIZE][TEXSIZE][3];
  float m_plasmamap[TEXSIZE * TEXSIZE * 3];
  float m_c[NUMCONSTS];  // constant
  float m_ct[NUMCONSTS];  // temporary value of constant
  float m_cv[NUMCONSTS];  // velocity of constant
};
