/*
 *  Copyright (C) 2005-2019 Team Kodi
 *  Copyright (C) 2002 Dirk Songuer <delax@sundancerinc.de>
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
 *
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

class ATTRIBUTE_HIDDEN CScreensaverSunDancer2
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver,
    public kodi::gui::gl::CShaderProgram
{
public:
  CScreensaverSunDancer2() = default;

  bool Start() override;
  void Stop() override;
  void Render() override;

  void OnCompiledAndLinked() override;
  bool OnEnabled() override;

private:
  bool m_startOK = false;
  GLuint m_vertexVBO = 0;
  GLuint m_indexVBO = 0;

  glm::mat4 m_projMat;
  glm::mat4 m_modelMat;

  GLint m_hProj = -1;
  GLint m_hModel = -1;
  GLint m_hPos = -1;
  GLint m_hCol = -1;

  GLint m_light1_ambientLoc = -1;
  GLint m_light1_diffuseLoc = -1;
  GLint m_light1_directionLoc = -1;
  GLint m_light1_positionLoc = -1;

  GLfloat *m_proj = nullptr;
  GLfloat *m_model = nullptr;

  GLfloat m_lightPosition[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  int m_quads_timer = -1;
  float m_direction_add = 0.00001f;

  typedef struct quadsystem_struct
  {
    float PositionX, PositionY, PositionZ;
    float ColorR, ColorG, ColorB;
    float Rotation;
    int Laenge, Hoehe;
    int Status, Intensity;
  } quadsystem;

  quadsystem* m_quads;

  float m_vlight, m_hlight, m_zlight;
  float m_vlightmul, m_hlightmul, m_zlightmul;

  float m_vertexw1, m_vertexw2, m_vertexw3, m_vertexw4;
  float m_vertexwmul1, m_vertexwmul2, m_vertexwmul3, m_vertexwmul4;

  bool m_reverse = false;
  float m_transparencyValue = 0.25f;
  int m_quadCount = 150;
  #define QUAD_SPEED_MAX_DEFAULT 20.0f / 400.0f
  float m_quadSpeedMax = QUAD_SPEED_MAX_DEFAULT;
  float m_quadSpeed = QUAD_SPEED_MAX_DEFAULT;
  int m_backRed = 255, m_backGreen = 0, m_backBlue = 0;
  int m_frontRed = 255, m_frontGreen = 255, m_frontBlue = 0;
};
