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

#include <kodi/AddonBase.h>

#include "light.h"

#define SMOKETIMES 8
#define WHICHSMOKES 100

class CScreensaverSkyRocket;

#undef ATTRIBUTE_FORCEINLINE
#define ATTRIBUTE_FORCEINLINE

class ATTRIBUTE_HIDDEN CSmoke
{
public:
  CSmoke(CScreensaverSkyRocket* base) : m_base(base) { }
  ~CSmoke();

  void Init();
  void Draw(unsigned int entry, const sColor& color);

  ATTRIBUTE_FORCEINLINE int WhichSmoke(unsigned int which) const { return m_whichSmoke[which]; }
  ATTRIBUTE_FORCEINLINE int SmokeTime(unsigned int which) const { return m_smokeTime[which]; }

private:
  // lifespans for smoke particles
  float m_smokeTime[SMOKETIMES];  // lifespans of consecutive smoke particles
  int m_whichSmoke[WHICHSMOKES];  // table to indicate which particles produce smoke
  sLight m_smokelist[5][4]; // smoke display lists

  unsigned int m_smoketex[5];
  CScreensaverSkyRocket* m_base;
};
