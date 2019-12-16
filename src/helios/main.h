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
#include <rsMath/rsMath.h>
#include <glm/ext.hpp>
#include <glm/gtc/type_ptr.hpp>

class impCubeVolume;
class impSurface;
class impSphere;
class emitter;
class attracter;
class ion;

struct sLight
{
  glm::vec3 vertex;
  glm::vec3 normal;
  glm::vec2 coord;
  glm::vec4 color;
};

class ATTRIBUTE_HIDDEN CScreensaverHelios
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver,
    public kodi::gui::gl::CShaderProgram
{
public:
  CScreensaverHelios() = default;

  bool Start() override;
  void Stop() override;
  void Render() override;

  void OnCompiledAndLinked() override;
  bool OnEnabled() override;

private:
  void setTargets(int whichTarget);
  static float surfaceFunction(void* base, float* position);

  double m_lastTime;;
  float m_frameTime = 0.0f;
  bool m_startOK = false;

  GLuint m_vertexVBO = 0;
  GLuint m_indexVBO = 0;
  GLuint m_texture_id[2] = {0};

  glm::mat4 m_projMat;
  glm::mat4 m_modelMat;
  glm::mat3 m_normalMat;

  GLint m_hProj = -1;
  GLint m_hModel = -1;
  GLint m_normalMatLoc = -1;
  GLint m_hNormal = -1;
  GLint m_hVertex = -1;
  GLint m_hColor = -1;
  GLint m_hCoord = -1;
  GLint m_hType = -1;

  int m_firstRender = 2;

  glm::vec3 m_newRgb;
  float m_billboardMat[16];
  impCubeVolume* m_volume = nullptr;
  impSurface* m_surface = nullptr;
  impSphere* m_spheres = nullptr;

  emitter *m_elist = nullptr;
  attracter *m_alist = nullptr;
  ion *m_ilist = nullptr;

  int m_ionsReleased = 0;
  float m_releaseTime = 0.0f;

  int m_newTarget = 0, m_lastTarget;

  rsVec m_oldHsl, m_newHsl = rsVec(rsRandf(1.0f), 1.0f, 1.0f), m_targetHsl;
  float m_colorInterp = 1.0f, m_colorChange;

  float m_valuetrig = 0.0f;

  float m_wait = 0.0f;
  float m_preinterp = glm::pi<float>(), m_interp;
  float m_interpconst = 0.001f;

  float m_oldCameraDistance;
  float m_cameraDistance;
  float m_targetCameraDistance = -1000.0f;
  float m_preCameraInterp = glm::pi<float>();

  std::vector<sLight> m_surfaceData;

  rsVec m_radialVel = rsVec(0.0f, 0.0f, 0.0f);
  rsVec m_targetRadialVel = m_radialVel;
  rsQuat m_rotQuat = rsQuat(0.0f, 0.0f, 0.0f, 1.0f);
};
