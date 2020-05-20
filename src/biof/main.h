/*
 *  Copyright (C) 2005-2020 Team Kodi (https://kodi.tv)
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
  void UpdateCubeRadius();
  void CreateCubeVerticesSmooth();

  bool m_lightEnabled = false;
  float m_lightPosition[4] = { 100.0f, 100.0f, 100.0f, 0.0f };
  float m_lightDiffuseColor[4] = { 1.0f, 0.8f, 0.4f, 1.0f };

  bool m_fogEnabled = false;
  float m_fogColor[4] = { 0.0f, 0.0f, 0.3f, 1.0f };
  float m_fogStart = 150.0f;
  float m_fogEnd = 250.0f;

  std::vector<glm::vec3> m_normal;
  std::vector<glm::vec3> m_vertex;
  std::vector<glm::vec4> m_color;
  std::vector<GLuint> m_index;

  float m_radius;
  int m_cubeSectorCount; // longitude, # of slices
  int m_cubeStackCount; // latitude, # of stacks

  glm::mat4 m_projMat;
  glm::mat4 m_modelMat;
  glm::mat3 m_normalMat;

  bool m_useLightning;
  bool m_useSphere;

  double m_startFrameTime;

  GLint m_projMatLoc = -1;
  GLint m_modelViewMatLoc = -1;
  GLint m_normalMatLoc = -1;
  GLint m_lightEnabledLoc = -1;
  GLint m_light0_ambientLoc = -1;
  GLint m_light0_diffuseLoc = -1;
  GLint m_light0_specularLoc = -1;
  GLint m_light0_positionLoc = -1;
#if defined(HAS_GLES)
  GLint m_pointSizeLoc = -1;
#endif
  GLint m_fogEnabledLoc = -1;
  GLint m_fogColorLoc = -1;
  GLint m_fogStartLoc = -1;
  GLint m_fogEndLoc = -1;
  GLint m_hNormal = -1;
  GLint m_hVertex = -1;
  GLint m_hColor = -1;

  bool m_startOK = false;

  GLuint m_vboHandle[4] = {0};

  int m_pointsQty, m_linesQty;
  float m_pointsSize;
  int m_geometry;
  int m_offAngle;
};
