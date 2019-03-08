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

#include <Implicit/impSphere.h>
#include <Implicit/impTorus.h>
#include <Implicit/impCapsule.h>

#include "gizmo.h"

#define CYL_RAD 0.36f

class ATTRIBUTE_HIDDEN Cylinder : public Gizmo
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
