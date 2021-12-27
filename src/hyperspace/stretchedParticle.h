/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
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

#include <kodi/AddonBase.h>
#include <kodi/gui/gl/GL.h>
#include <glm/gtc/type_ptr.hpp>

class CScreensaverHyperspace;

class ATTR_DLL_LOCAL CStretchedParticle
{
public:
  CStretchedParticle(CScreensaverHyperspace* base);
  ~CStretchedParticle() = default;

  void SetPosition(float x, float y, float z);
  void SetColor(float r, float g, float b, bool withColorFull);

  //void update();
  void Draw(const glm::vec3& eyepoint);

  ATTR_FORCEINLINE void SetRadius(float radius) { m_radius = radius; }
  ATTR_FORCEINLINE void SetFov(float fov) { m_fov = fov; }

  ATTR_FORCEINLINE float* Position() { return glm::value_ptr(m_pos); }
  ATTR_FORCEINLINE float* LastPosition() { return glm::value_ptr(m_lastPos); }

private:
  CScreensaverHyperspace* m_base;
  glm::vec3 m_pos;  // current position
  glm::vec3 m_lastPos;  // position at previous frame
  glm::vec3 m_drawPos;  // median position, where star is actually drawn
  glm::vec2 m_screenPos;  // point where pos maps to screen
  glm::vec2 m_lastScreenPos;  // point where lastPos maps to screen

  float m_fov;

  float m_radius;
  glm::vec3 m_color;
};
