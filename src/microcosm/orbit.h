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

#include <Implicit/impTorus.h>

#include "gizmo.h"

class ATTRIBUTE_HIDDEN Orbit : public Gizmo
{
public:
  impTorus* torus1;
  impTorus* torus2;
  impTorus* torus3;

  Orbit(CScreensaverMicrocosm* base) : Gizmo(base)
  {
    torus1 = new impTorus;
    torus1->setRadius(0.21f);
    torus1->setThickness(0.04f);
    mShapes.push_back(torus1);
    torus2 = new impTorus;
    torus2->setRadius(0.32f);
    torus2->setThickness(0.04f);
    mShapes.push_back(torus2);
    torus3 = new impTorus;
    torus3->setRadius(0.43f);
    torus3->setThickness(0.04f);
    mShapes.push_back(torus3);
  }

  ~Orbit()
  {
    delete torus1;
    delete torus2;
    delete torus3;
  }

  void update(float frametime)
  {
    updateConstants(frametime);
    updateMatrix();

    rsMatrix m;
    m.makeRotate(mCoeff[9] * 8.0f, 1.0f, 0.0f, 0.0f);
    m.rotate(mCoeff[8] * 10.0f, 0.0f, 1.0f, 0.0f);
    m.postMult(mMatrix);
    torus1->setMatrix(m.get());

    m.makeRotate(mCoeff[7] * 6.0f, 0.0f, 1.0f, 0.0f);
    m.rotate(mCoeff[6] * 7.5f, 0.0f, 0.0f, 1.0f);
    m.postMult(mMatrix);
    torus2->setMatrix(m.get());

    m.makeRotate(mCoeff[5] * 4.0f, 0.0f, 0.0f, 1.0f);
    m.rotate(mCoeff[4] * 5.0f, 1.0f, 0.0f, 0.0f);
    m.postMult(mMatrix);
    torus3->setMatrix(m.get());
  }
};
