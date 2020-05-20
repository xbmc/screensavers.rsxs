/*
 *  Copyright (C) 2005-2020 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2010 Terence M. Welsh
 *  Ported to Kodi by Alwin Esch <alwinus@kodi.tv>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
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
