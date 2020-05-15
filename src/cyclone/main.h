/*
 *  Copyright (C) 2005-2020 Team Kodi (https://kodi.tv)
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

struct sLight
{
  glm::vec3 vertex;
  glm::vec3 normal;
  glm::vec4 color;
};

class CCyclone;
class CParticle;

class ATTRIBUTE_HIDDEN CScreensaverCyclone
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver,
    public kodi::gui::gl::CShaderProgram
{
public:
  CScreensaverCyclone() = default;

  bool Start() override;
  void Stop() override;
  void Render() override;

  void OnCompiledAndLinked() override;
  bool OnEnabled() override;

  void DrawEntry(int primitive, const sLight* data, unsigned int size);
  void DrawSphere(const glm::vec4& color);
  inline float FrameTime() const { return m_frameTime; }

  GLint m_lightingEnabled = 0;
  glm::vec4 m_uniformColor;
  glm::mat4 m_projMat;
  glm::mat4 m_modelMat;
  glm::mat4 m_modelProjMat;
  glm::mat3 m_normalMat;
  float m_fact[13];

private:
  void Sphere(GLfloat radius, GLint slices, GLint stacks);

  std::vector<sLight> m_sphereTriangleFan1;
  std::vector<sLight> m_sphereTriangleFan2;
  std::vector<std::vector<sLight>> m_sphereQuadStrip;

  GLint m_projMatLoc = -1;
  GLint m_modelViewMatLoc = -1;
  GLint m_modelViewProjectionMatrixLoc = -1;
  GLint m_transposeAdjointModelViewMatrixLoc = -1;
  GLint m_lightingLoc = -1;
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

  GLuint m_vertexVBO = 0;

  CCyclone **m_cyclones;
  CParticle **m_particles;

  float m_frameTime = 0.0f;
  bool m_startOK = false;
  double m_lastTime;
};
