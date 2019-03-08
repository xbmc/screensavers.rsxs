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

struct sPosition
{
  sPosition() : x(0.0f), y(0.0f), z(0.0f), u(1.0f) {}
  sPosition(float* d) : x(d[0]), y(d[1]), z(d[2]), u(1.0f) {}
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
  sColor(float* c) : r(c[0]), g(c[1]), b(c[2]), a(c[3]) {}
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
  sPosition normal;
  sColor color;
  sCoord coord;
};

class CBug;

class ATTRIBUTE_HIDDEN CScreensaverFlocks
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver,
    public kodi::gui::gl::CShaderProgram
{
public:
  CScreensaverFlocks() = default;

  bool Start() override;
  void Stop() override;
  void Render() override;

  void OnCompiledAndLinked() override;
  bool OnEnabled() override;

  void DrawEntry(int primitive, const sLight* data, unsigned int size);
  void DrawSphere();

  glm::mat4 m_projMat;
  glm::mat4 m_modelMat;
  glm::mat4 m_modelProjMat;
  glm::mat3 m_normalMat;

  GLuint m_lightingEnabled = 1;
  GLuint m_uniformColorUsed = 0;
  sColor m_uniformColor;

private:
  void Sphere(GLfloat radius, GLint slices, GLint stacks);

  std::vector<sLight> m_sphereTriangleFan1;
  std::vector<sLight> m_sphereTriangleFan2;

  GLint m_projMatLoc = -1;
  GLint m_modelViewMatLoc = -1;
  GLint m_modelViewProjectionMatrixLoc = -1;
  GLint m_transposeAdjointModelViewMatrixLoc = -1;
  GLint m_textureUsedLoc = -1;
  GLint m_lightingLoc = -1;
  GLint m_uniformColorUsedLoc = -1;
  GLint m_uniformColorLoc = -1;
  GLint m_light0_ambientLoc = -1;
  GLint m_light0_diffuseLoc = -1;
  GLint m_light0_specularLoc = -1;
  GLint m_light0_positionLoc = -1;
  GLint m_light0_constantAttenuationLoc = -1;
  GLint m_light0_linearAttenuationLoc = -1;
  GLint m_light0_quadraticAttenuationLoc = -1;
  GLint m_light0_spotDirectionLoc = -1;
  GLint m_light0_spotExponentLoc = -1;
  GLint m_light0_spotCutoffAngleCosLoc = -1;
  GLint m_material_ambientLoc = -1;
  GLint m_material_diffuseLoc = -1;
  GLint m_material_specularLoc = -1;
  GLint m_material_emissionLoc = -1;
  GLint m_material_shininessLoc = -1;

  GLint m_hNormal = -1;
  GLint m_hVertex = -1;
  GLint m_hColor = -1;
  GLint m_hCoord = -1;

  GLuint m_vertexVBO = 0;
  GLuint m_indexVBO = 0;

  GLuint m_texture;
  GLuint m_textureUsed = 0;

  CBug* m_lBugs;
  CBug* m_fBugs;

  GLubyte m_idx[4] = {0, 1, 3, 2};
  sLight m_light[4];
  unsigned char m_lightTexture[LIGHTSIZE][LIGHTSIZE];

  float m_elapsedTime = 0.0f;
  float m_colorFade;
  int m_width;
  int m_height;
  int m_depth;

  bool m_startOK = false;
  int m_startClearCnt = 5;
  double m_lastTime;
};
