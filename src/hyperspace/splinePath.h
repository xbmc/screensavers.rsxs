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
#include <glm/gtc/type_ptr.hpp>

class ATTRIBUTE_HIDDEN CSplinePath
{
public:
  CSplinePath(int length);
  ~CSplinePath();

  void MoveAlongPath(float increment);
  void GetPoint(int section, float where, glm::vec3& position);
  void GetDirection(int section, float where, float* direction);
  void GetBaseDirection(int section, float where, float* direction);
  void Update(float multiplier);

  ATTRIBUTE_FORCEINLINE float Step() { return m_step; }
  ATTRIBUTE_FORCEINLINE int NumPoints() { return m_numPoints; }

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
