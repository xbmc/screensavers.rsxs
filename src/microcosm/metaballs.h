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

#include "gizmo.h"

using namespace std;

class ATTR_DLL_LOCAL Metaballs : public Gizmo
{
public:
  Metaballs(CScreensaverMicrocosm* base, unsigned int num) : Gizmo(base)
  {
    mMaxDisplacement = 0.4f;

    unsigned int n = num >= 1 ? num : 1;  // at least 1
    for(unsigned int i=0; i<n; ++i)
    {
      impSphere* sphere = new impSphere;
      sphere->setThickness(0.06f);
      mShapes.push_back(sphere);
    }
  }

  ~Metaballs(){}

  void setScale(float s)
  {
    mScale = s;

    // Must set thickness because impSphere ignores the scale portion of
    // its matrix for efficiency.
    for(unsigned int i=0; i<mShapes.size(); ++i)
      mShapes[i]->setThickness(0.2f * mScale);
  }

  void update(float frametime)
  {
    updateConstants(frametime);
    updateMatrix();

    rsMatrix m;
    float x(0.0f);
    const float dx(RS_PIx2 / float(mShapes.size()));
    const float offset(0.3f);
    for(unsigned int i=0; i<mShapes.size(); ++i)
    {
      m.makeTranslate(0.0f, 0.27f * mCoeff[7], 0.27f * mCoeff[8]);
      m.rotate((mCoeffPhase[9] * 2.0f + x), 1.0f, 0.0f, 0.0f);
      m.rotate((mCoeffPhase[10] * 3.0f + x), 0.0f, 1.0f, 1.0f);
      m.postMult(mMatrix);
      //mShapes[i]->setMatrix(m.get());  // Why doesn't this work?
      mShapes[i]->setPosition(&((m.get())[12]));
      x += dx;
    }
  }
};
