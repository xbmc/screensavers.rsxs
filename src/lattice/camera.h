/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 1999-2010 Terence M. Welsh
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <math.h>

class CCamera
{
public:
  CCamera(){};
  ~CCamera(){};
  void init(const float* mat, float f);
  bool inViewVolume(float* pos, float radius);

// private:
  float farplane;
  float cullVec[4][3];  // vectors perpendicular to viewing volume planes
};
