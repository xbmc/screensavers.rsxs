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

#include <Implicit/impCapsule.h>
#include <Implicit/impSphere.h>

#include "gizmo.h"

class ATTR_DLL_LOCAL SpheresAndCapsules : public Gizmo
{
private:
  int mCount;

public:
  SpheresAndCapsules(CScreensaverMicrocosm* base, int num) : Gizmo(base)
  {
    mCount = num;

    for(int i=0; i<mCount; i++)
    {
      impSphere* sphere = new impSphere;
      sphere->setThickness(0.05f);
      mShapes.push_back(sphere);
    }

    for(int i=0; i<mCount; i++)
    {
      impCapsule* cap = new impCapsule;
      cap->setThickness(0.03f);
      mShapes.push_back(cap);
    }
  };

  ~SpheresAndCapsules()
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

    rsMatrix m;
    for(unsigned int i=0; i< (unsigned int)mCount; i++)
    {
      m.makeTranslate(0.4f * cosf(mCoeffPhase[0] * 5.0f), 0.0f, 0.0f);
      m.rotate(RS_PIx2 / float(mCount) * float(i), 0.0f, 0.0f, 1.0f);
      m.rotate(cosf(mCoeffPhase[1] * 4.0f), 1.0f, 0.0f, 0.0f);
      m.rotate(cosf(mCoeffPhase[2] * 4.0f), 0.0f, 1.0f, 0.0f);
      m.postMult(mMatrix);
      mShapes[i]->setPosition(&((m.get())[12]));
    }

    for(unsigned int i=0; i< (unsigned int)mCount; i++)
    {
      impCapsule* cap = static_cast<impCapsule*>(mShapes[i+mCount]);
      m.makeRotate(cosf(mCoeffPhase[3] * 4.0f), 1.0f, 0.0f, 0.0f);
      m.rotate(cosf(mCoeffPhase[4] * 4.0f), 0.0f, 1.0f, 0.0f);
      m.translate(0.35f * cosf(mCoeffPhase[5] * 5.0f), 0.0f, 0.0f);
      m.rotate(RS_PIx2 / float(mCount) * float(i), 0.0f, 0.0f, 1.0f);
      m.rotate(cosf(mCoeffPhase[6] * 4.0f), 1.0f, 0.0f, 0.0f);
      m.rotate(cosf(mCoeffPhase[7] * 4.0f), 0.0f, 1.0f, 0.0f);
      m.postMult(mMatrix);
      cap->setMatrix(m.get());
      cap->setLength(0.07f + 0.07f * cosf(mCoeffPhase[6] * 5.0f));
    }
  }
};
