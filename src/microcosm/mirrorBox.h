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

#define USE_CLIP_PLANES 0

class CScreensaverMicrocosm;

// MirrorBox renders the same unit cube 8 times, reflecting it about the x, y, and z axes.
// So the whole MirrorBox has extents {(-1,-1,-1), (1,1,1)}.

class ATTRIBUTE_HIDDEN MirrorBox
{
private:
  double plane0[4];
  double plane1[4];
  double plane2[4];
  double plane3[4];
  double plane4[4];
  double plane5[4];

  // Coefficients for rotating and translating contents of MirrorBox
  float mCoeffRate[6];  // coefficients' rates
  float mCoeffPhase[6];  // coefficients' phases
  float mCoeff[6];  // coefficients that oscillate within {-1.0, 1.0}

  rsMatrix mMatrix;
  CScreensaverMicrocosm* m_base;

public:
  MirrorBox(CScreensaverMicrocosm* base);
  ~MirrorBox(){}
  void update(float frametime);
  // Pass center of MirrorBox and vector from eye to center.
  void draw(const float& x, const float& y, const float& z, const float& eyex, const float& eyey, const float& eyez);

private:
  // Pass vector from eye to center of this sub-box.  There are 8 sub-boxes.
  void drawSubBox(const float& eyex, const float& eyey, const float& eyez);
  inline void setClipPlanes();
};
