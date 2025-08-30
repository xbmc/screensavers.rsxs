/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2002-2008 Michael Chapman
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

#include <vector>

#include <rsMath/rsVec.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

class CWisp;

struct sLight
{
  glm::vec3 vertex;
  glm::vec4 color;
  glm::vec2 coord;
};

class ATTR_DLL_LOCAL CScreensaverEuphoria
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver,
    public kodi::gui::gl::CShaderProgram
{
public:
  CScreensaverEuphoria() = default;

  // kodi::addon::CInstanceScreensaver
  bool Start() override;
  void Stop() override;
  void Render() override;

  // kodi::gui::gl::CShaderProgram
  void OnCompiledAndLinked() override;
  bool OnEnabled() override;

  void DrawEntry(int primitive, const sLight* data, unsigned int size);

  inline void BindTexture(int type, int id)
  {
    // Needed to give shader the presence of a texture
    m_textureUsed = id != 0;
    glBindTexture(GL_TEXTURE_2D, id);
  }

private:
  bool m_startOK = false;
  double m_lastTime;
  float m_aspectRatio;
  float m_feedbackIntensity;

  struct
  {
    int x;
    int y;
    int width;
    int height;
  } m_viewport;

  // feedback texture object
  GLuint m_feedbackTex = 0;
  int m_feedbackTexSize;
  unsigned char* m_feedbackTexOld;

  // feedback variables
  float m_fr[4] = {0.0f, 0.0f, 0.0f, 0.0f};
  float m_fv[4];
  float m_f[4];

  // feedback limiters
  float m_lr[3] = {0.0f, 0.0f, 0.0f};
  float m_lv[3];
  float m_l[3];

  bool m_textureUsed = false;
  GLuint m_texture = 0;

  glm::mat4 m_projMat;
  glm::mat4 m_modelMat;

  GLint m_projMatLoc = -1;
  GLint m_modelViewMatLoc = -1;
  GLint m_textureIdLoc = -1;
  GLint m_hVertex = -1;
  GLint m_hCoord = -1;
  GLint m_hColor = -1;

  GLuint m_vao = 0;
  GLuint m_vertexVBO = 0;

  CWisp *m_backwisps;
  CWisp *m_wisps;

  // GL stored variables to pop back during stop
  int m_glUnpackRowLength = 0;
  int m_glReadBuffer = GL_BACK;
};
