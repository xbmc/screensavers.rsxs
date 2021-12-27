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

#include <Implicit/impEllipsoid.h>

#include "gizmo.h"

class ATTR_DLL_LOCAL StringOfEllipsoids : public Gizmo
{
public:
  unsigned int numEllipsoids;
  float mOffset;

  StringOfEllipsoids(CScreensaverMicrocosm* base, unsigned int num, float offset) : Gizmo(base)
  {
    numEllipsoids = num >= 1 ? num : 1;  // at least 1
    for(unsigned int i=0; i<numEllipsoids; ++i)
    {
      impEllipsoid* e = new impEllipsoid;
      e->setThickness(0.04f);
      mShapes.push_back(e);
    }
    mOffset = offset;
  }

  ~StringOfEllipsoids()
  {
    for(unsigned int i=0; i<mShapes.size(); ++i)
      delete mShapes[i];
  }

  void update(float frametime)
  {
    updateConstants(frametime);
    updateMatrix();

    rsMatrix m;
    float n(0.0f);
    for(unsigned int i=0; i<numEllipsoids; ++i)
    {
      const float mult(0.3f);
      m.makeScale(cosf(mCoeffPhase[7]*4.0f+n) * 0.4f + 1.2f, cosf(mCoeffPhase[8]*4.0f+n) * 0.4f + 1.2f, cosf(mCoeffPhase[9]*4.0f+n) * 0.4f + 1.2f);
      m.rotate(cosf(mCoeffPhase[10]*2.0f+n), 1.0f, 0.0f, 0.0f);
      m.rotate(cosf(mCoeffPhase[11]*2.0f+n), 0.0f, 1.0f, 0.0f);
      m.rotate(cosf(mCoeffPhase[12]*2.0f+n), 0.0f, 0.0f, 1.0f);
      m.translate(cosf(mCoeffPhase[13]*3.0f+n*(1.0f+0.5f*mCoeff[14])) * mult, cosf(mCoeffPhase[15]*3.0f+n*(1.0f+0.5f*mCoeff[16])) * mult, cosf(mCoeffPhase[17]*3.0f+n*(1.0f+0.5f*mCoeff[18])) * mult);
      m.postMult(mMatrix);
      mShapes[i]->setMatrix(m.get());
      n += mOffset;
    }
  }
};
