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

struct sPosition
{
  sPosition() : x(0.0f), y(0.0f), z(0.0f), u(0.0f) {}
  sPosition(float x, float y, float z) : x(x), y(y), z(z), u(0.0f) {}
  float x,y,z,u;
};

struct sColor
{
  sColor() : r(0.0f), g(0.0f), b(0.0f), a(0.0f) {}
  sColor(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
  float r,g,b,a;
};

class ATTRIBUTE_HIDDEN CScreensaverBiof
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver,
    public kodi::gui::gl::CShaderProgram
{
public:
  CScreensaverBiof();

  // kodi::addon::CInstanceScreensaver
  bool Start() override;
  void Stop() override;
  void Render() override;

  // kodi::gui::gl::CShaderProgram
  void OnCompiledAndLinked() override;
  bool OnEnabled() override;

private:
  void Sphere(GLfloat radius, GLint slices, GLint stacks);

  bool m_lightEnabled = false;
  float m_lightPosition[4] = { 100.0f, 100.0f, 100.0f, 0.0f };
  float m_lightDiffuseColor[4] = { 1.0f, 0.8f, 0.4f, 1.0f };

  bool m_fogEnabled = false;
  float m_fogColor[4] = { 0.0f, 0.0f, 0.3f, 1.0f };
  float m_fogStart = 150.0f;
  float m_fogEnd = 250.0f;

  glm::mat4 m_projMat;
  glm::mat4 m_modelMat;
  glm::mat3 m_normalMat;

  bool m_useLightning;
  bool m_useSphere;

  GLint m_projMatLoc = -1;
  GLint m_modelViewMatLoc = -1;
  GLint m_normalMatLoc = -1;
  GLint m_lightEnabledLoc = -1;
  GLint m_light0_ambientLoc = -1;
  GLint m_light0_diffuseLoc = -1;
  GLint m_light0_specularLoc = -1;
  GLint m_light0_positionLoc = -1;
  GLint m_fogEnabledLoc = -1;
  GLint m_fogColorLoc = -1;
  GLint m_fogStartLoc = -1;
  GLint m_fogEndLoc = -1;
  GLint m_hNormal = -1;
  GLint m_hVertex = -1;
  GLint m_hColor = -1;

  bool m_startOK = false;
  double m_frameTime = 0.0f;

  GLenum m_mode;
  GLuint m_nVerts;
  GLuint m_vboHandle[4] = {0};

  int m_pointsQty, m_linesQty;
  int m_geometry;
  int m_offAngle;
};
