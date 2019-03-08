/*
 *  Copyright (C) 2005-2019 Team Kodi
 *  Copyright (C) 2010 Terence M. Welsh
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
 * and reworked to GL 4.0.
 */

#pragma once

#include <kodi/AddonBase.h>
#include <rsMath/rsMath.h>

#define TEX_SIZE 128
#define NUM_COLORS 4
#define NUM_TEX_COEFF (6 * NUM_COLORS)

class CScreensaverMicrocosm;

class ATTRIBUTE_HIDDEN Texture1D
{
private:
  unsigned int mTexId;
  unsigned char mData[TEX_SIZE * 4];

  rsVec4 mColor[NUM_COLORS];

  // A rate of change for each coefficient
  float mCoeffRate[NUM_TEX_COEFF];
  // A phase for each coefficient
  float mCoeffPhase[NUM_TEX_COEFF];
  // a coefficient for each color's r, g, b, a, and position
  float mCoeff[NUM_TEX_COEFF];

public:
  Texture1D(CScreensaverMicrocosm* base) : m_base(base) { init(); }
  ~Texture1D();

  void init();
  void update();
  void bind();

private:
  CScreensaverMicrocosm* m_base;

  const float& minimum(const float& a, const float& b){ return (a<b) ? a : b; }
};
