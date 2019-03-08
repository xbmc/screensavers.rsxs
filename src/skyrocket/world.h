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

#define STARMESH 12
#define STARTEXSIZE 1024
#define MOONGLOWTEXSIZE 128
#define CLOUDMESH 70

#undef ATTRIBUTE_FORCEINLINE
#define ATTRIBUTE_FORCEINLINE

class CScreensaverSkyRocket;

class ATTRIBUTE_HIDDEN CWorld
{
public:
  CWorld(CScreensaverSkyRocket* base) : m_base(base) { }
  ~CWorld();

  void Init();

  // For building mountain sillohettes in sunset
  void makeHeights(int first, int last, int *h);
  void update(float frameTime);
  void draw();

  ATTRIBUTE_FORCEINLINE unsigned int CloudTex() { return m_cloudtex; }
  float m_clouds[CLOUDMESH+1][CLOUDMESH+1][9];  // 9 = x,y,z,u,v,std bright,r,g,b

private:
  int m_doSunset;
  float m_moonRotation, m_moonHeight;
  float m_cloudShift;
  float m_stars[STARMESH+1][STARMESH/2][6];  // 6 = x,y,z,u,v,bright

  sLight m_starlistStrip[STARMESH/2-1][(STARMESH+1)*2];
  unsigned int m_starlistStripEntries = 0;
  unsigned int m_starlistStripSize = 0;
  sLight m_starlistFan[(STARMESH+2)*2];
  unsigned int m_starlistFanSize = 0;
  unsigned int m_startex;

  sLight m_moonlist[4];
  unsigned int m_moontex;

  sLight m_moonglowlist[4];
  unsigned int m_moonglowtex;

  unsigned int m_cloudtex;

  unsigned int m_sunsettex;
  sLight m_sunsetlist[18];

  unsigned int m_earthneartex;
  unsigned int m_earthfartex;
  unsigned int m_earthlighttex;
  sLight m_earthlist[5][4];
  sLight m_earthnearlist[4];
  sLight m_earthfarlist[4][4];

  CScreensaverSkyRocket* m_base;
};
