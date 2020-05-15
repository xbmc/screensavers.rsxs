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
#include <Implicit/impSphere.h>

#include "gizmo.h"

class ATTRIBUTE_HIDDEN Grinder : public Gizmo
{
private:
  int mCount;

public:
  Grinder(CScreensaverMicrocosm* base, int count) : Gizmo(base)
  {
    mCount = count;

    for(int i=0; i<mCount*2; i++){
      impRoundedHexahedron* hexa = new impRoundedHexahedron;
      hexa->setThickness(0.035f);
      mShapes.push_back(hexa);
    }

    impSphere* sphere = new impSphere;
    mShapes.push_back(sphere);
  }

  ~Grinder()
  {
    for(unsigned int i=0; i<mShapes.size(); i++)
      delete mShapes[i];
  }

  void update(float frametime)
  {
    updateConstants(frametime);
    updateMatrix();

    const float offset = 0.38f;
    const float xsize = 0.02f;
    const float ysize = 0.4f / powf(float(mCount), 1.1f);  // choose size so gear teeth mesh together comfortably
    rsMatrix m;
    for(int i=0; i<mCount; i++)
    {
      impRoundedHexahedron* hexa = static_cast<impRoundedHexahedron*>(mShapes[i]);
      const float phase = (RS_PIx2 / float(mCount) * float(i)) + (mCoeff[3] * 6.0f);
      m.identity();
      m[9] = -1.0f * sinf(phase);
      m.translate(offset, 0.0f, 0.0f);
      m.rotate(phase, 0.0f, 0.0f, 1.0f);
      m.postMult(mMatrix);
      hexa->setMatrix(m.get());
      hexa->setSize(xsize, ysize, ysize);
    }
    for(int i=0; i<mCount; i++)
    {
      impRoundedHexahedron* hexa = static_cast<impRoundedHexahedron*>(mShapes[mCount+i]);
      const float phase = (RS_PIx2 / float(mCount) * (float(i) + 0.5f)) + (mCoeff[3] * 6.0f);
      m.identity();
      m[9] = 1.0f * sinf(phase);
      m.translate(offset, 0.0f, 0.0f);
      m.rotate(phase, 0.0f, 0.0f, 1.0f);
      m.rotate(RS_PIo2, 0.0f, 1.0f, 0.0f);
      m.postMult(mMatrix);
      hexa->setMatrix(m.get());
      hexa->setSize(xsize, ysize, ysize);
    }

    const float sphereoffset = 0.35f;
    impSphere* sphere = static_cast<impSphere*>(mShapes[mCount * 2]);
    m.makeTranslate(sphereoffset * sinf(mCoeffPhase[0] * 4.0f), sphereoffset * sinf(mCoeffPhase[1] * 4.0f), sphereoffset * sinf(mCoeffPhase[2] * 4.0f));
    m.postMult(mMatrix);
    sphere->setPosition(&((m.get())[12]));
    sphere->setThickness(0.05f + 0.01f * mCoeff[3]);
  }
};
