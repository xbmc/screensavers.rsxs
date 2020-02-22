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
#include <vector>

class CSplinePath;
class CScreensaverHyperspace;

class ATTRIBUTE_HIDDEN CTunnel
{
public:
  CTunnel(CScreensaverHyperspace* base, CSplinePath* sp, int res);
  ~CTunnel();

  void Make(float frameTime);
  void Draw(float lerp);

private:
  CScreensaverHyperspace* m_base;
  CSplinePath* m_path;

  int m_resolution;
  int m_numSections;
  int m_section;
  float**** m_v;  // vertex data
  float**** m_t;  // texture coordinate data
  float**** m_c;  // color data

  float m_radius;
  // offset for tunnel width
  float m_widthOffset;

  // spin texture map around tunnel
  float m_texSpin;

  // offsets for low and high frequency 3D color waves
  float m_huelo, m_huehi;
  float m_satlo, m_sathi;
  float m_lumlo, m_lumhi;
  
  std::vector<sLight> m_lights;
};
