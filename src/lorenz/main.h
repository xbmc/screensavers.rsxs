/*
 *  Copyright (C) 2005-2020 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2002, 2009 Sören Sonnenburg <sonne@debian.org>
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

#include <vector>

#include <glm/gtc/type_ptr.hpp>

#define num_satellites_default 25
#define num_points_default 100000
#define line_width_attractor_default 2
#define line_width_satellites_default 12
#define camera_speed_default 0.3f
#define linear_cutoff_default 0.2f
#define camera_angle_default 45.0f

struct sLatticeSegmentEntry
{
  glm::vec3 normal;
  glm::vec3 vertex;
  glm::vec4 color;
};

// Parameters edited in the dialog box
struct settings
{
  int line_width_satellites=line_width_satellites_default;
  int line_width_attractor=line_width_attractor_default;
  float camera_speed=camera_speed_default;
  float camera_angle=camera_angle_default;
  float linear_cutoff=linear_cutoff_default;

  int num_precomputed_points=num_points_default;
  int num_satellites=num_satellites_default;
};

class ATTRIBUTE_HIDDEN CScreensaverLorenz
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver,
    public kodi::gui::gl::CShaderProgram
{
public:
  CScreensaverLorenz();

  // kodi::addon::CInstanceScreensaver
  bool Start() override;
  void Stop() override;
  void Render() override;

  // kodi::gui::gl::CShaderProgram
  void OnCompiledAndLinked() override;
  bool OnEnabled() override;

private:
  void display(void);
  void set_camera();
  void reshape(int w, int h);
  void init_satellites();
  void precompute_lorenz_array();
  void init_line_strip(void);
  void reduce_points(int cutoff);

  settings m_settings;

  // For hack check, Kodi currently also calls Render() if Start() returned false!
  bool m_startOK = false;

  float m_camera_angle_anim[2] = {5.0f, 179.0f};
  float m_camera_angle_anim_speed = 0.1f;

  int m_num_points = 0;
  int m_num_points_max=-1;

  int m_width = 800;
  int m_height = 600;

  float* m_lorenz_coords = nullptr;
  float* m_lorenz_path = nullptr;
  float* m_satellite_times = nullptr;
  float* m_satellite_speeds = nullptr;

  glm::mat4 m_projMat;
  glm::mat4 m_modelMat;
  glm::mat3 m_normalMat;

  bool m_useLightning;
  bool m_useSphere;
  GLfloat m_position[4];
  GLfloat m_lightDir0[4];

  GLint m_projMatLoc = -1;
  GLint m_modelViewMatLoc = -1;
  GLint m_normalMatLoc = -1;

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

  double m_lastTime;
  float m_frameTime = 0.0f;
  std::vector<sLatticeSegmentEntry> m_stripEntries;
};
