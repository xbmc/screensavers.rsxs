
/*
 *  Copyright (C) 2005-2019 Team Kodi
 *  Copyright (C) 1999 Andreas Gustafsson
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

#define NR_WAVES 24
#define TEXSIZE 256

struct sLight
{
  glm::vec3 vertex;
  glm::vec4 color;
  glm::vec2 coord;
};

class ATTRIBUTE_HIDDEN CScreensaverColorFire
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver,
    public kodi::gui::gl::CShaderProgram
{
public:
  CScreensaverColorFire() = default;

  bool Start() override;
  void Stop() override;
  void Render() override;

  void OnCompiledAndLinked() override;
  bool OnEnabled() override;

private:
  void InitWave(int nr);
  void DrawWave(int nr, float fDeltaTime);

  glm::mat4 m_projMat;
  glm::mat4 m_modelMat;

  GLint m_projMatLoc = -1;
  GLint m_modelViewMatLoc = -1;
  GLint m_textureTypeLoc = -1;

  GLint m_hVertex = -1;
  GLint m_hCoord = -1;
  GLint m_hColor = -1;

  GLuint m_vertexVBO = 0;
  GLuint m_indexVBO = 0;
  GLuint m_texture = 0;

  int m_textureType = 1;
  bool m_startOK = false;
  double m_lastTime;

  GLubyte m_idx[4] = {0, 1, 3, 2};
  sLight m_light[4];

  float m_wrot[NR_WAVES], m_wtime[NR_WAVES], m_wr[NR_WAVES], m_wg[NR_WAVES], m_wb[NR_WAVES], m_wspd[NR_WAVES], m_wmax[NR_WAVES];
  float m_v1 = 0, m_v2 = 0, m_v3 = 0;
};
