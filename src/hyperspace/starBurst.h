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

#include "stretchedParticle.h"
#include "light.h"

#include <kodi/AddonBase.h>

#define SB_NUM_STARS 200
#define SB_LIGHT_SIZE (32+1)*2

class CScreensaverHyperspace;

class ATTR_DLL_LOCAL CStarBurst
{
public:
  CStarBurst(CScreensaverHyperspace* base);
  ~CStarBurst();

  void Restart(float* position);
  void DrawStars();
  void Draw(float lerp);  // draw with shaders

  ATTR_FORCEINLINE CStretchedParticle** Stars() { return m_stars; }

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
