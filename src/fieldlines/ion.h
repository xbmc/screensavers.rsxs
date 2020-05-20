/*
 *  Copyright (C) 2005-2019 Team Kodi
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

class CScreensaverFieldLines;

class ATTRIBUTE_HIDDEN CIon
{
public:
  float charge;
  float xyz[3];
  float vel[3];
  float angle;
  float anglevel;

  CIon(CScreensaverFieldLines* base);
  ~CIon(){};
  void update(float frameTime);

private:
  CScreensaverFieldLines* m_base;
};
