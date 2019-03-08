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

#include "gizmo.h"

class ATTRIBUTE_HIDDEN TriangleOfSpheres : public Gizmo
{
public:
  TriangleOfSpheres(CScreensaverMicrocosm* base, unsigned int num) : Gizmo(base)
  {
    mMaxDisplacement = 0.5f;

    unsigned int n = num >= 1 ? num : 1;  // at least 1
    for(unsigned int i=0; i<n; ++i){
      impSphere* sphere = new impSphere;
      sphere->setThickness(0.06f);
      mShapes.push_back(sphere);
    }
  }

  ~TriangleOfSpheres() = default;

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
    for(unsigned int i=0; i<mShapes.size(); ++i)
    {
      m.makeTranslate(0.28f * cosf(mCoeffPhase[8]), 0.0f, 0.0f);
      m.rotate((mCoeffPhase[9] * 4.0f + x) * -3.0f, 0.0f, 0.0f, 1.0f);
      m.translate(0.28f * cosf(mCoeffPhase[8] + RS_PIo2), 0.0f, 0.0f);
      m.rotate(mCoeffPhase[9] * 4.0f + x, 0.0f, 0.0f, 1.0f);
      m.postMult(mMatrix);
      //mShapes[i]->setMatrix(m.get());  // Why doesn't this work?
      mShapes[i]->setPosition(&((m.get())[12]));
      x += dx;
    }
  }
};

