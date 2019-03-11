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

#include <Implicit/impRoundedHexahedron.h>
#include <Implicit/impCapsule.h>

#include "gizmo.h"

class ATTRIBUTE_HIDDEN CubesAndCapsules : public Gizmo
{
private:
  int mCount;
  float mDx;

public:
  CubesAndCapsules(CScreensaverMicrocosm* base, int num, float dx) : Gizmo(base)
  {
    mCount = num;
    mDx = dx;

    for(int i=0; i<mCount; i++){
      impRoundedHexahedron* hexa = new impRoundedHexahedron;
      hexa->setThickness(0.03f);
      mShapes.push_back(hexa);
    }

    for(int i=0; i<mCount; i++){
      impCapsule* cap = new impCapsule;
      cap->setThickness(0.03f);
      mShapes.push_back(cap);
    }
  };

  ~CubesAndCapsules()
  {
    for(unsigned int i=0; i<mShapes.size(); i++)
      delete mShapes[i];
  }

  void update(float frametime)
  {
    updateConstants(frametime);
    updateMatrix();

    rsMatrix m;

    const float hoffset = 0.35f;
    const float size = 0.03f;
    float x = 0.0f;
    for(int i=0; i<mCount; i++)
    {
      impRoundedHexahedron* hexa = static_cast<impRoundedHexahedron*>(mShapes[i]);
      m.makeRotate(cosf(mCoeffPhase[6] * 4.0f), 1.0f, 0.0f, 0.0f);
      m.rotate(cosf(mCoeffPhase[7] * 4.0f), 0.0f, 1.0f, 0.0f);
      m.translate(hoffset * cosf(mCoeffPhase[8] * 2.0f + x), hoffset * cosf(mCoeffPhase[9] * 3.0f + x), hoffset * cosf(mCoeffPhase[10] * 3.0f + x));
      m.postMult(mMatrix);
      hexa->setMatrix(m.get());
      hexa->setSize(size + size * cosf(mCoeffPhase[11] * 5.0f), size + size * cosf(mCoeffPhase[12] * 5.0f), size + size * cosf(mCoeffPhase[13] * 5.0f));
      x += mDx;
    }

    const float coffset = 0.32f;
    x = 0.0f;
    for(int i=0; i<mCount; i++)
    {
      impCapsule* cap = static_cast<impCapsule*>(mShapes[i+mCount]);
      m.makeRotate(cosf(mCoeffPhase[14] * 4.0f), 1.0f, 0.0f, 0.0f);
      m.rotate(cosf(mCoeffPhase[15] * 4.0f), 0.0f, 1.0f, 0.0f);
      m.translate(coffset * cosf(mCoeffPhase[16] * 3.0f + x), coffset * cosf(mCoeffPhase[17] * 2.0f + x), coffset * cosf(mCoeffPhase[18] * 2.0f + x));
      m.postMult(mMatrix);
      cap->setMatrix(m.get());
      cap->setLength(0.07f + 0.07f * cosf(mCoeffPhase[19] * 5.0f));
      x += mDx;
    }
  }
};
