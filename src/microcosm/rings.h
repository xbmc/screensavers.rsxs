/*
 *  Copyright (C) 2005-2020 Team Kodi (https://kodi.tv)
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

#include <Implicit/impRoundedHexahedron.h>
#include <Implicit/impTorus.h>

#include "gizmo.h"

class ATTRIBUTE_HIDDEN Rings : public Gizmo
{
private:
  int mCount;

public:
  Rings(CScreensaverMicrocosm* base, int count) : Gizmo(base)
  {
    mCount = count;

    for(int i=0; i<mCount; i++)
    {
      impRoundedHexahedron* hexa = new impRoundedHexahedron;
      hexa->setThickness(0.035f);
      mShapes.push_back(hexa);
    }

    impTorus* torus = new impTorus;
    torus->setRadius(0.4f);
    torus->setThickness(0.04f);
    mShapes.push_back(torus);
  }

  ~Rings()
  {
    for(unsigned int i=0; i<mShapes.size(); i++)
      delete mShapes[i];
  }

  void update(float frametime)
  {
    updateConstants(frametime);
    updateMatrix();

    const float size = 0.06f;
    rsMatrix m;
    for(int i=0; i<mCount; i++)
    {
      impRoundedHexahedron* hexa = static_cast<impRoundedHexahedron*>(mShapes[i]);
      m.identity();
      m[1] = 0.5f * cosf(mCoeffPhase[0] * 5.0f);
      m[2] = 0.5f * cosf(mCoeffPhase[1] * 5.0f);
      m[4] = 0.5f * cosf(mCoeffPhase[2] * 5.0f);
      m.translate(0.32f, 0.0f, 0.0f);
      m.rotate(RS_PIx2 / float(mCount) * float(i), 0.0f, 0.0f, 1.0f);
      m.rotate(mCoeff[3] * 4.0f, 0.0f, 0.0f, 1.0f);
      m.postMult(mMatrix);
      hexa->setMatrix(m.get());
      hexa->setSize(size, size, size);
    }

    m.makeRotate(mCoeff[10] * 5.0f, 1.0f, 0.0f, 0.0f);
    m.rotate(mCoeff[11] * 5.0f, 0.0f, 1.0f, 0.0f);
    m.postMult(mMatrix);
    impTorus* torus = static_cast<impTorus*>(mShapes[mCount]);
    torus->setMatrix(m.get());
    torus->setRadius(0.3f + 0.1f * mCoeff[12]);
  }
};
