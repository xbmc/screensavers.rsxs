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

#include <Implicit/impTorus.h>

#include "gizmo.h"

class ATTRIBUTE_HIDDEN TorusBox : public Gizmo
{
public:
  impTorus* torus1;
  impTorus* torus2;
  impTorus* torus3;

  TorusBox(CScreensaverMicrocosm* base) : Gizmo(base)
  {
    torus1 = new impTorus;
    torus1->setRadius(0.4f);
    torus1->setThickness(0.05f);
    mShapes.push_back(torus1);
    torus2 = new impTorus;
    torus2->setRadius(0.4f);
    torus2->setThickness(0.05f);
    mShapes.push_back(torus2);
    torus3 = new impTorus;
    torus3->setRadius(0.4f);
    torus3->setThickness(0.05f);
    mShapes.push_back(torus3);
  }

  void update(float frametime)
  {
    updateConstants(frametime);
    updateMatrix();

    rsMatrix m;
    m.makeTranslate(0.0f, 0.0f, 0.41f * cosf(mCoeffPhase[1] * 5.0f));
    m.postMult(mMatrix);
    torus1->setMatrix(m.get());

    m.makeTranslate(0.0f, 0.0f, 0.41f * cosf((mCoeffPhase[1] + 1.0472f) * 5.0f));
    m.rotate(RS_PIo2, 1.0f, 0.0f, 0.0f);
    m.postMult(mMatrix);
    torus2->setMatrix(m.get());

    m.makeTranslate(0.0f, 0.0f, 0.41f * cosf((mCoeffPhase[1] + 2.0944f) * 5.0f));
    m.rotate(RS_PIo2, 0.0f, 1.0f, 0.0f);
    m.postMult(mMatrix);
    torus3->setMatrix(m.get());
  }
};

