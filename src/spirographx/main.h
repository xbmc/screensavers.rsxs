/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2003 Holmes Futrell <holmes@neatosoftware.com>
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

#define MAXSUBLOOPS 5
#define MAXDETAIL 100

struct renderContent
{
  glm::vec3 points[360 * MAXSUBLOOPS * MAXDETAIL];
  glm::vec4 colorsBlur[360 * MAXSUBLOOPS * MAXDETAIL];
  glm::vec4 colorsLine[360 * MAXSUBLOOPS * MAXDETAIL];
  int numberOfPoints;
  int subLoops;
  int blurWidth;
  float equationBase;
  float speed;
  int graphTo;
  float lineColor[4];
  float blurColor[4];
  float fade = 1.0f;
};

class ATTR_DLL_LOCAL CScreensaverSpiroGraphX
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver,
    public kodi::gui::gl::CShaderProgram
{
public:
  CScreensaverSpiroGraphX() = default;
  ~CScreensaverSpiroGraphX() override = default;

  bool Start() override;
  void Stop() override;
  void Render() override;

  void OnCompiledAndLinked() override;
  bool OnEnabled() override;

private:
  const float m_blurAlpha = 0.2f;
  const float m_lineAlpha = 0.4f;

  void DrawAll(renderContent& content, glm::vec4* colors);
  void GetAll(renderContent& content);
  void ChangeSettings();

  double m_lastTime;
  bool m_startOK = false;
  unsigned int m_vertexVBO[2] = {0};

  int m_detail;
  float m_timeInterval;
  renderContent m_content;
  renderContent m_contentOld;
  bool m_contentOldActive = false;
  double m_lastSettingsChange = -1;

  glm::mat4 m_projMat;
  glm::mat4 m_modelMat;

  GLint m_hProj = -1;
  GLint m_hModel = -1;
  GLint m_hPos = -1;
  GLint m_hCol = -1;
};
