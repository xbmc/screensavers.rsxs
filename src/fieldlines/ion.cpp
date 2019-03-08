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

#include "ion.h"
#include "main.h"

#include <math.h>
#include <rsMath/rsMath.h>

#define PIx2 6.28318530718f

CIon::CIon(CScreensaverFieldLines* base) : m_base(base)
{
  float speed = m_base->Speed();
  if (rsRandi(2))
    charge = -1.0f;
  else
    charge = 1.0f;
  xyz[0] = rsRandf(2.0f * m_base->RenderWidth()) - m_base->RenderWidth();
  xyz[1] = rsRandf(2.0f * m_base->RenderHeight()) - m_base->RenderHeight();
  xyz[2] = rsRandf(2.0f * m_base->RenderDeep()) - m_base->RenderDeep();
  vel[0] = rsRandf(speed * 4.0f) - (speed * 2.0f);
  vel[1] = rsRandf(speed * 4.0f) - (speed * 2.0f);
  vel[2] = rsRandf(speed * 4.0f) - (speed * 2.0f);
  angle = 0.0f;
  anglevel = 0.0005f * speed + 0.0005f * rsRandf(speed);
}

void CIon::update(float frameTime)
{
  float speed = m_base->Speed();

  xyz[0] += vel[0] * frameTime;
  xyz[1] += vel[1] * frameTime;
  xyz[2] += vel[2] * frameTime;
  if(xyz[0] > m_base->RenderWidth())
    vel[0] -= 0.1f * speed;
  if(xyz[0] < -m_base->RenderWidth())
    vel[0] += 0.1f * speed;
  if(xyz[1] > m_base->RenderHeight())
    vel[1] -= 0.1f * speed;
  if(xyz[1] < -m_base->RenderHeight())
    vel[1] += 0.1f * speed;
  if(xyz[2] > m_base->RenderDeep())
    vel[2] -= 0.1f * speed;
  if(xyz[2] < -m_base->RenderDeep())
    vel[2] += 0.1f * speed;

  angle += anglevel;
  if (angle > PIx2)
    angle -= PIx2;
}
