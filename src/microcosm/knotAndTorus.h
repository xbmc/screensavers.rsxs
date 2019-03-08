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

#include <Implicit/impKnot.h>
#include <Implicit/impTorus.h>

#include "gizmo.h"

class ATTRIBUTE_HIDDEN KnotAndTorus : public Gizmo
{
public:
  KnotAndTorus(CScreensaverMicrocosm* base, int coils, int twists) : Gizmo(base)
  {
    impKnot* knot = new impKnot;
    knot->setRadius1(0.28f);
    knot->setRadius2(0.14f);
    knot->setNumCoils(coils);
    knot->setNumTwists(twists);
    knot->setThickness(0.04f);
    mShapes.push_back(knot);

    impTorus* torus = new impTorus;
    torus->setRadius(0.42f);
    torus->setThickness(0.04f);
    mShapes.push_back(torus);

    setScale(1.0f);
  };

  ~KnotAndTorus()
  {
    for(unsigned int i=0; i<mShapes.size(); i++)
      delete mShapes[i];
  }

  void update(float frametime)
  {
    updateConstants(frametime);
    updateMatrix();

    impKnot* knot = static_cast<impKnot*>(mShapes[0]);
    knot->setMatrix(mMatrix.get());
    const float fade0 = mCoeff[6] * 0.5f + 0.5f;
    const float fade1 = fade0 * (cosf(mCoeffPhase[7] * 5.0f) * 0.5f + 0.5f);
    knot->setRadius1(0.28f + 0.1f * fade1);
    knot->setRadius2(0.14f * (1.0f - fade1));

    rsMatrix m;
    m.makeScale(0.75f + 0.25f * cosf(mCoeffPhase[13] * 3.0f), 1.0f, 1.0f);
    m.scale(1.0f, 0.75f + 0.25f * cosf(mCoeffPhase[14] * 4.0f), 1.0f);
    m.rotate(mCoeff[15] * 3.0f, 0.0f, 0.0f, 1.0f);
    m.rotate(mCoeff[16] * 4.0f, 0.0f, 1.0f, 0.0f);
    m.rotate(mCoeff[17] * 5.0f, 1.0f, 0.0f, 0.0f);
    m.postMult(mMatrix);
    mShapes[1]->setMatrix(m.get());
  }
};
