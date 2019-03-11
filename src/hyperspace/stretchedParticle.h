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

#include <kodi/AddonBase.h>
#include <kodi/gui/gl/GL.h>
#include <glm/gtc/type_ptr.hpp>

class CScreensaverHyperspace;

class ATTRIBUTE_HIDDEN CStretchedParticle
{
public:
  CStretchedParticle(CScreensaverHyperspace* base);
  ~CStretchedParticle() = default;

  void SetPosition(float x, float y, float z);
  void SetColor(float r, float g, float b, bool withColorFull);

  //void update();
  void Draw(const glm::vec3& eyepoint);

  ATTRIBUTE_FORCEINLINE void SetRadius(float radius) { m_radius = radius; }
  ATTRIBUTE_FORCEINLINE void SetFov(float fov) { m_fov = fov; }

  ATTRIBUTE_FORCEINLINE float* Position() { return glm::value_ptr(m_pos); }
  ATTRIBUTE_FORCEINLINE float* LastPosition() { return glm::value_ptr(m_lastPos); }

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
