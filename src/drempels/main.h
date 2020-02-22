/*
 *  Copyright (C) 2005-2019 Team Kodi
 *  Copyright (C) 2001 Ryan M. Geiss <guava at geissworks dot com>
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

#include "TexMgr.h"

#include <kodi/addon-instance/Screensaver.h>
#include <kodi/gui/gl/GL.h>
#include <kodi/gui/gl/Shader.h>
#include <glm/gtc/type_ptr.hpp>

struct sLight
{
  glm::vec3 vertex;
  glm::vec4 color;
  glm::vec2 coord;
};

class td_cellcornerinfo;

class ATTRIBUTE_HIDDEN CScreensaverDrempels
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver,
    public kodi::gui::gl::CShaderProgram
{
public:
  CScreensaverDrempels() = default;

  bool Start() override;
  void Stop() override;
  void Render() override;

  void OnCompiledAndLinked() override;
  bool OnEnabled() override;

private:
  void RandomizeStartValues();
  void DrawQuads(const glm::vec4& color);

  TexMgr m_textureManager;

  int m_cells;
  int m_cellResolution;

  bool m_fadeComplete = false;
  uint32_t *m_fadeBuf = nullptr;

  float m_animTime = 0;

  float m_fRandStart1;
  float m_fRandStart2;
  float m_fRandStart3;
  float m_fRandStart4;
  float m_warp_w[4];
  float m_warp_uscale[4];
  float m_warp_vscale[4];
  float m_warp_phase[4];

  double m_lastTexChange = 0;

  td_cellcornerinfo *m_cell = nullptr;
  unsigned short *m_buf = nullptr;

  glm::mat4 m_projMat;
  glm::mat4 m_modelMat;

  GLuint m_tex, m_ptex, m_ctex, m_uvtex, m_btex;

  GLint m_projMatLoc = -1;
  GLint m_modelViewMatLoc = -1;
  GLint m_hVertex = -1;
  GLint m_hCoord = -1;
  GLint m_hColor = -1;

  GLuint m_vertexVBO = 0;
  GLuint m_indexVBO = 0;

  sLight m_quad[4];
  GLubyte m_idx[4] = {0, 1, 3, 2};

  bool m_startOK = false;
  double m_lastTime;
};
