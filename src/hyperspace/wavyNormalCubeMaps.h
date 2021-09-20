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

class ATTRIBUTE_HIDDEN CWavyNormalCubeMaps
{
public:
  CWavyNormalCubeMaps(int frames, int size);
  ~CWavyNormalCubeMaps();

  // Pass a point on a unit sphere and receive the corresponding
  // normal at that point on the sphere
  void WavyFunc(const glm::vec3& point, glm::vec3& normal);

  ATTRIBUTE_FORCEINLINE const GLuint* Texture() const { return m_texture; }

private:
  int m_numFrames;
  int m_texSize;

  GLuint* m_texture;

  // Phase must be 0.0 at frame 0
  // and phase must be 1.0 at frame numFrames
  // so that cubemaps have temporal continuity.
  float m_phase;
};
