/*
 *  Copyright (C) 2005-2019 Team Kodi
 *  Copyright (C) 2000 Jeremie Allard (Hufo / N.A.A.)
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
