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

#include <Implicit/impSphere.h>
#include <Implicit/impTorus.h>
#include <Implicit/impCapsule.h>

#include "gizmo.h"

#define CYL_RAD 0.36f

class ATTR_DLL_LOCAL Cylinder : public Gizmo
{
public:
  Cylinder(CScreensaverMicrocosm* base) : Gizmo(base)
  {
    for(int i=0; i<3; i++)
    {
      impSphere* sphere = new impSphere;
      sphere->setThickness(0.05f);
      mShapes.push_back(sphere);
    }

    for(int i=0; i<2; i++)
    {
      impTorus* torus = new impTorus;
      torus->setThickness(0.04f);
      torus->setRadius(CYL_RAD);
      mShapes.push_back(torus);
    }

    for(int i=0; i<3; i++)
    {
      impCapsule* cap = new impCapsule;
      cap->setThickness(0.04f);
      cap->setLength(0.29f);
      mShapes.push_back(cap);
    }
  }

  ~Cylinder()
  {
    for(unsigned int i=0; i<mShapes.size(); i++)
      delete mShapes[i];
  }

  void update(float frametime)
  {
    updateConstants(frametime);
    updateMatrix();

    const float offset = 0.36f;
    rsMatrix m;
    for(int i=0; i<3; i++)
    {
      m.makeTranslate(offset * cosf(mCoeffPhase[i*2+1] * 5.0f), offset * cosf(mCoeffPhase[i*2+2] * 5.0f), offset * cosf(mCoeffPhase[i*2+3] * 5.0f));
      m.postMult(mMatrix);
      mShapes[i]->setPosition(&((m.get())[12]));
    }

    impTorus* torus = static_cast<impTorus*>(mShapes[3]);
    m.makeTranslate(0.0f, 0.0f, -0.38f);
    m.postMult(mMatrix);
    torus->setMatrix(m.get());

    torus = static_cast<impTorus*>(mShapes[4]);
    m.makeTranslate(0.0f, 0.0f, 0.38f);
    m.postMult(mMatrix);
    torus->setMatrix(m.get());

    impCapsule* cap = static_cast<impCapsule*>(mShapes[5]);
    m.makeTranslate(CYL_RAD, 0.0f, 0.0f);
    m.rotate(RS_PI + mCoeff[9] * RS_PI, 0.0f, 0.0f, 1.0f);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());

    cap = static_cast<impCapsule*>(mShapes[6]);
    m.makeTranslate(CYL_RAD, 0.0f, 0.0f);
    m.rotate(RS_PIx2 * 0.33333f - mCoeff[9] * RS_PIx2, 0.0f, 0.0f, 1.0f);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());

    cap = static_cast<impCapsule*>(mShapes[7]);
    m.makeTranslate(CYL_RAD, 0.0f, 0.0f);
    m.rotate(RS_PIx2 * -0.33333f + mCoeff[9] * RS_PIx2, 0.0f, 0.0f, 1.0f);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());
  }
};
