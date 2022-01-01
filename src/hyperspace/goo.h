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

#include "light.h"

#include <kodi/AddonBase.h>
#include <glm/gtc/type_ptr.hpp>

#include <rsMath/rsMath.h>
#include <Implicit/impCubeVolume.h>
#include <Implicit/impCrawlPoint.h>

class CScreensaverHyperspace;

class ATTR_DLL_LOCAL CGoo
{
public:
  CGoo(CScreensaverHyperspace* base, int res, float rad);
  ~CGoo();

  void update(float x, float z, float heading, float fov);
  static float function(void* thisPtr, float* position);
  void draw(float* goo_rgb);

private:
  int resolution;
  float radius;
  float unitSize;
  float volumeSize;

  static float c[4];  // constants
  float cp[4];  // constants phase
  float cs[4];  // constants speed

  static float camx, camz;
  float centerx, centerz;
  static float shiftx, shiftz;
  int arraySize;
  impCubeVolume* volume;
  impSurface*** surface;
  bool** useSurface;

  // normals of planes for culling impSurfaces
  float clip[3][2];

  CScreensaverHyperspace* m_base;
};
