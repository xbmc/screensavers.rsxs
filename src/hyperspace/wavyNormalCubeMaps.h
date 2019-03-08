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
