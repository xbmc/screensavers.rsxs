/*
 *  Copyright (C) 2005-2019 Team Kodi
 *  Copyright (C) 2010 Terence M. Welsh
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
 * and reworked to GL 4.0.
 */

#pragma once

#include <Implicit/impTorus.h>

#include "gizmo.h"

class ATTRIBUTE_HIDDEN RingOfTori : public Gizmo
{
public:
  unsigned int numTori;
  impTorus** tori;

  RingOfTori(CScreensaverMicrocosm* base, unsigned int num) : Gizmo(base)
  {
    mMaxDisplacement = 0.5f;

    numTori = num >= 2 ? num : 2;  // at least 2
    tori = new impTorus*[numTori];
    for(unsigned int i=0; i<numTori; ++i)
    {
      tori[i] = new impTorus;
      tori[i]->setRadius(0.18f);
      tori[i]->setThickness(0.04f);
      mShapes.push_back(tori[i]);
    }
  }

  void update(float frametime)
  {
    updateConstants(frametime);
    updateMatrix();

    rsMatrix m;
    float x(0.0f);
    const float dx(RS_PIx2 / float(numTori));
    for(unsigned int i=0; i<numTori; ++i)
    {
      m.makeScale(0.75f + 0.25f * cosf(mCoeffPhase[6] * 3.0f), 1.0f, 0.85f);
      m.rotate(mCoeffPhase[7] * 3.0f, 0.0f, 1.0f, 0.0f);
      m.translate(0.26f * cosf(mCoeffPhase[8] * 3.0f), 0.0f, 0.0f);
      m.rotate(mCoeffPhase[9] * 3.0f/* + x*/, 1.0f, 0.0f, 0.0f);
      m.rotate(x, 0.0f, 0.0f, 1.0f);
      m.postMult(mMatrix);
      tori[i]->setMatrix(m.get());
      x += dx;
    }
  }
};
