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

struct sLight
{
  glm::vec3 vertex;
  glm::vec3 normal;
  glm::vec4 color;
  glm::vec2 coord;
};

class CBug;

class ATTR_DLL_LOCAL CScreensaverFlocks
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
  glm::vec4 m_uniformColor;

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
