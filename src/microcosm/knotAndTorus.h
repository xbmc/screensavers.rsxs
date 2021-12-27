/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2010 Terence M. Welsh
 *  Ported to Kodi by Alwin Esch <alwinus@kodi.tv>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
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

class ATTR_DLL_LOCAL KnotAndTorus : public Gizmo
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
