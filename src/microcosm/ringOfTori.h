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

class ATTR_DLL_LOCAL RingOfTori : public Gizmo
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
