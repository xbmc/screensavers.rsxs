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

#include "light.h"

#define WAVESTEPS 40

class CScreensaverSkyRocket;

class ATTR_DLL_LOCAL CShockwave
{
public:
  CShockwave(CScreensaverSkyRocket* base) : m_base(base) { }

  void Init();
  void Draw(float temperature, float texmove);

private:
  float m_shockwavegeom[7][WAVESTEPS+1][3];
  float m_colors[7][4];
  sLight m_wavesteps[(WAVESTEPS+1)*2];
  CScreensaverSkyRocket* m_base;
};
