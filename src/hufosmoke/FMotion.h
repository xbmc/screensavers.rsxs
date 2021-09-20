/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2000 Jeremie Allard (Hufo / N.A.A.)
 *  Ported to Kodi by Alwin Esch <alwinus@kodi.tv>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

/*
 * Code is based on:
 *   http://rss-glx.sourceforge.net/
 * and reworked to GL 4.0.
 */

#pragma once

#include <rsMath/rsVec.h>

struct PARTICLE
{
  rsVec p, v;
};

class CScreensaverHufoSmoke;

bool FMotionInit ();
void FMotionQuit ();
void FMotionAnimate (const float &dt);
void FMotionWarp (rsVec &p, const float &dt);
void AffFMotion(CScreensaverHufoSmoke* base, const rsVec& fireSrc, const rsMatrix& fireM, const rsVec& fireO);
