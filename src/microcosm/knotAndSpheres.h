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
#include <Implicit/impSphere.h>

#include "gizmo.h"

class ATTRIBUTE_HIDDEN KnotAndSpheres : public Gizmo
{
public:
  KnotAndSpheres(CScreensaverMicrocosm* base, int coils, int twists, int spheres) : Gizmo(base)
  {
    impKnot* knot = new impKnot;
    knot->setRadius1(0.28f);
    knot->setRadius2(0.14f);
    knot->setNumCoils(coils);
    knot->setNumTwists(twists);
    knot->setThickness(0.04f);
    mShapes.push_back(knot);
    for(int i=0; i<spheres; i++)
    {
      impSphere* sphere = new impSphere;
      sphere->setThickness(0.06f);
      mShapes.push_back(sphere);
    }

    setScale(1.0f);
  };

  ~KnotAndSpheres()
  {
    for(unsigned int i=0; i<mShapes.size(); i++)
      delete mShapes[i];
  }

  void setScale(float s)
  {
    mScale = s;

    // Must set thickness because impSphere ignores the scale portion of its matrix for efficiency.
    for(unsigned int i=1; i<mShapes.size(); ++i)
      mShapes[i]->setThickness(0.06f * mScale);
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
    const float mult = 0.35f;
    for(unsigned int i=1; i<mShapes.size(); i++)
    {
      m.makeTranslate(mult * cosf(mCoeffPhase[7+i] * 5.0f), mult * cosf(mCoeffPhase[8+i] * 5.0f), mult * cosf(mCoeffPhase[9+i] * 5.0f));
      m.postMult(mMatrix);
      mShapes[i]->setPosition(&((m.get())[12]));
    }
  }
};
