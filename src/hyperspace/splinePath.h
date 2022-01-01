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
#include <glm/gtc/type_ptr.hpp>

class ATTR_DLL_LOCAL CSplinePath
{
public:
  CSplinePath(int length);
  ~CSplinePath();

  void MoveAlongPath(float increment);
  void GetPoint(int section, float where, glm::vec3& position);
  void GetDirection(int section, float where, float* direction);
  void GetBaseDirection(int section, float where, float* direction);
  void Update(float multiplier);

  ATTR_FORCEINLINE float Step() { return m_step; }
  ATTR_FORCEINLINE int NumPoints() { return m_numPoints; }

private:
  void MakeNewPoint();
  float Interpolate(float a, float b, float c, float d, float where);

  float* m_phase;
  float* m_rate;
  float** m_movevec;
  float** m_basexyz;
  float** m_xyz;
  float** m_basedir;
  float** m_dir;
  int m_numPoints;
  float m_step;
};
