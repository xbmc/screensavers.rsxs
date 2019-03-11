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

#include <Implicit/impEllipsoid.h>
#include <Implicit/impTorus.h>

#include "gizmo.h"

class ATTRIBUTE_HIDDEN UFO : public Gizmo
{
public:
  unsigned int numEllipsoids;

  UFO(CScreensaverMicrocosm* base, unsigned int num) : Gizmo(base)
  {
    numEllipsoids = num >= 1 ? num : 1;  // at least 1
    for(unsigned int i=0; i<numEllipsoids; ++i)
    {
      impEllipsoid* e = new impEllipsoid;
      e->setThickness(0.04f);
      mShapes.push_back(e);
    }

    impTorus* torus = new impTorus;
    torus->setRadius(0.43f);
    torus->setThickness(0.04f);
    mShapes.push_back(torus);
  }

  ~UFO()
  {
    for(unsigned int i=0; i<mShapes.size(); ++i)
      delete mShapes[i];
  }

  void update(float frametime)
  {
    updateConstants(frametime);
    updateMatrix();

    rsMatrix m;
    const float offset = 0.35f;
    for(unsigned int i=0; i<numEllipsoids; ++i)
    {
      const float phase = RS_PIx2 / float(numEllipsoids) * float(i);
      float ripple0 = (cosf(mCoeffPhase[6] * 10.0f + phase) + 1.0f) * 0.5f;
      ripple0 *= ripple0;
      ripple0 *= ripple0;
      ripple0 *= ripple0;
      float ripple1 = (cosf(mCoeffPhase[6] * 10.0f + phase + (RS_PI * 0.25f)) + 1.0f) * 0.5f;
      ripple1 *= ripple1;
      ripple1 *= ripple1;
      ripple1 *= ripple1;
      m.makeRotate(mCoeff[8], 1.0f, 0.0f, 0.0f);
      m.rotate(mCoeff[9], 0.0f, 1.0f, 0.0f);
      m.scale(1.0f + ripple1 - (ripple0 * 0.5f), 1.0f + ripple0 - (ripple1 * 0.5f), 1.0f + ripple1 - (ripple0 * 0.5f));
      m.rotate(mCoeff[10], 1.0f, 0.0f, 0.0f);
      m.rotate(mCoeff[11], 0.0f, 0.0f, 1.0f);
      m.translate(offset * (0.75f + 0.25f * cosf(mCoeffPhase[12] * 3.0f)), 0.0f, 0.0f);
      m.rotate(phase, 0.0f, 0.0f, 1.0f);
      m.postMult(mMatrix);
      mShapes[i]->setMatrix(m.get());
    }

    m.makeScale(0.75f + 0.25f * cosf(mCoeffPhase[13] * 3.0f), 1.0f, 1.0f);
    m.scale(1.0f, 0.75f + 0.25f * cosf(mCoeffPhase[14] * 4.0f), 1.0f);
    m.rotate(mCoeff[15] * 3.0f, 0.0f, 0.0f, 1.0f);
    m.rotate(mCoeff[16] * 4.0f, 0.0f, 1.0f, 0.0f);
    m.rotate(mCoeff[17] * 5.0f, 1.0f, 0.0f, 0.0f);
    m.postMult(mMatrix);
    mShapes[numEllipsoids]->setMatrix(m.get());
  }
};
