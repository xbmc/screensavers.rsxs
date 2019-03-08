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

#include "gizmo.h"

class ATTRIBUTE_HIDDEN Brain : public Gizmo
{
public:
  unsigned int numEllipsoids;
  impEllipsoid** xEllipsoids;
  impEllipsoid** yEllipsoids;
  impEllipsoid** zEllipsoids;

  Brain(CScreensaverMicrocosm* base, unsigned int num) : Gizmo(base)
  {
    numEllipsoids = num >= 1 ? num : 1;  // at least 1
    xEllipsoids = new impEllipsoid*[numEllipsoids];
    yEllipsoids = new impEllipsoid*[numEllipsoids];
    zEllipsoids = new impEllipsoid*[numEllipsoids];
    for (unsigned int i=0; i<numEllipsoids; ++i)
    {
      xEllipsoids[i] = new impEllipsoid;
      yEllipsoids[i] = new impEllipsoid;
      zEllipsoids[i] = new impEllipsoid;
      xEllipsoids[i]->setThickness(0.05f);
      yEllipsoids[i]->setThickness(0.05f);
      zEllipsoids[i]->setThickness(0.05f);
      mShapes.push_back(xEllipsoids[i]);
      mShapes.push_back(yEllipsoids[i]);
      mShapes.push_back(zEllipsoids[i]);
    }
  }

  ~Brain(){
    delete[] xEllipsoids;
    delete[] yEllipsoids;
    delete[] zEllipsoids;
  }

  void update(float frametime)
  {
    updateConstants(frametime);
    updateMatrix();

    rsMatrix m;
    float x(0.0f);
    const float dx(RS_PIx2 / float(numEllipsoids));
    for(unsigned int i=0; i<numEllipsoids; ++i)
    {
      const float offset1(x + mCoeffPhase[9] * 4.0f);
      const float offset2(x + mCoeffPhase[9]);
      const float mult(0.38f);
      m.makeScale((1.0f - fabs(cosf(offset1))) * 2.0f + 1.0f, 1.0f, 1.0f);
      m.translate(cosf(offset1) * mult, cosf(offset2) * mult, sinf(offset2) * mult);
      m.postMult(mMatrix);
      xEllipsoids[i]->setMatrix(m.get());
      m.makeScale(1.0f, (1.0f - fabs(cosf(offset1))) * 2.0f + 1.0f, 1.0f);
      m.translate(cosf(offset2) * mult, cosf(offset1) * mult, sinf(offset2) * mult);
      m.postMult(mMatrix);
      yEllipsoids[i]->setMatrix(m.get());
      m.makeScale(1.0f, 1.0f, (1.0f - fabs(cosf(offset1))) * 2.0f + 1.0f);
      m.translate(cosf(offset2) * mult, sinf(offset2) * mult, cosf(offset1) * mult);
      m.postMult(mMatrix);
      zEllipsoids[i]->setMatrix(m.get());
      x += dx;
    }
  }
};
