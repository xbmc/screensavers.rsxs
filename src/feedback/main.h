/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2009 Tugrul Galatali <tugrul@galatali.com>
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

#include <rsMath/rsVec.h>

#include <glm/gtc/type_ptr.hpp>

#include <vector>

struct sLight
{
  glm::vec3 vertex;
  glm::vec3 color;
  glm::vec2 coord;
};

class ATTR_DLL_LOCAL CScreensaverFeedback
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver,
    public kodi::gui::gl::CShaderProgram
{
public:
  CScreensaverFeedback() = default;

  bool Start() override;
  void Stop() override;
  void Render() override;

  void OnCompiledAndLinked() override;
  bool OnEnabled() override;

private:
  inline void BindTexture(int type, int id)
  {
    // Needed to give shader the presence of a texture
    m_textureUsed = id != 0;
    glBindTexture(GL_TEXTURE_2D, id);
  }

  int m_width = 256, m_height = 256;

  std::vector<rsVec> m_displacements;
  std::vector<rsVec> m_velocities;
  std::vector<rsVec> m_accelerations;

  rsVec m_totalV;

  glm::mat4 m_projMat;
  glm::mat4 m_modelMat;

  GLint m_projMatLoc = -1;
  GLint m_modelViewMatLoc = -1;
  GLint m_textureIdLoc = -1;
  GLint m_hVertex = -1;
  GLint m_hCoord = -1;
  GLint m_hColor = -1;

  GLuint m_vertexVBO = 0;
  GLuint m_indexVBO = 0;

  GLuint m_texture;

  sLight m_rotatingColor[4];
  sLight* m_framedTextures = nullptr;
  GLubyte m_rotatingColorIdx[4] = {0, 1, 3, 2};

  bool m_textureUsed = false;
  bool m_startOK = false;
  double m_lastTime;
};
