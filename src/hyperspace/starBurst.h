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

#include "stretchedParticle.h"
#include "light.h"

#include <kodi/AddonBase.h>

#define SB_NUM_STARS 200
#define SB_LIGHT_SIZE (32+1)*2

class CScreensaverHyperspace;

class ATTRIBUTE_HIDDEN CStarBurst
{
public:
  CStarBurst(CScreensaverHyperspace* base);
  ~CStarBurst();

  void Restart(float* position);
  void DrawStars();
  void Draw(float lerp);  // draw with shaders

  ATTRIBUTE_FORCEINLINE CStretchedParticle** Stars() { return m_stars; }

private:
  // stars only stay active while they're within viewing range
  bool* m_stars_active;
  float** m_stars_velocity;
  float m_size;
  float m_pos[3];

  sLight m_light[32][SB_LIGHT_SIZE];
  CStretchedParticle** m_stars;
  CScreensaverHyperspace* m_base;
};
