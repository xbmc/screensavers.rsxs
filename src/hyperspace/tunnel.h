/*
 *  Copyright (C) 2005-2020 Team Kodi (https://kodi.tv)
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
