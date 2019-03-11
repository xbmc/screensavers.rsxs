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
 *
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

class ATTRIBUTE_HIDDEN CShockwave
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
