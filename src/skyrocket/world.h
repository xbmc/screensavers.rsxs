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

#define STARMESH 12
#define STARTEXSIZE 1024
#define MOONGLOWTEXSIZE 128
#define CLOUDMESH 70

#undef ATTR_FORCEINLINE
#define ATTR_FORCEINLINE

class CScreensaverSkyRocket;

class ATTR_DLL_LOCAL CWorld
{
public:
  CWorld(CScreensaverSkyRocket* base) : m_base(base) { }
  ~CWorld();

  void Init();

  // For building mountain sillohettes in sunset
  void makeHeights(int first, int last, int *h);
  void update(float frameTime);
  void draw();

  ATTR_FORCEINLINE unsigned int CloudTex() { return m_cloudtex; }
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
