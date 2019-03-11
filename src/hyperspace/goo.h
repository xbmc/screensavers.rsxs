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

#include "light.h"

#include <kodi/AddonBase.h>
#include <glm/gtc/type_ptr.hpp>

#include <rsMath/rsMath.h>
#include <Implicit/impCubeVolume.h>
#include <Implicit/impCrawlPoint.h>

class CScreensaverHyperspace;

class ATTRIBUTE_HIDDEN CGoo
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
