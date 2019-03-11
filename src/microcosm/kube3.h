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
#include <Implicit/impSphere.h>

#include "gizmo.h"

class ATTRIBUTE_HIDDEN Kube3 : public Gizmo
{
public:
  Kube3(CScreensaverMicrocosm* base) : Gizmo(base)
  {
    const float size = 0.06f;
    for(int i=0; i<6; i++)
    {
      impRoundedHexahedron* hexa = new impRoundedHexahedron;
      hexa->setThickness(0.03f);
      hexa->setSize(size, size, size);
      mShapes.push_back(hexa);
    }

    impSphere* sphere = new impSphere;
    mShapes.push_back(sphere);
  }

  ~Kube3()
  {
    for(unsigned int i=0; i<mShapes.size(); i++)
      delete mShapes[i];
  }

  void update(float frametime)
  {
    updateConstants(frametime);
    updateMatrix();

    const float offset = 0.23f;
    rsMatrix m;
    impRoundedHexahedron* hexa;
    float shift[21];
    for(int i=0; i<21; i++)
    {
      shift[i] = mCoeff[i] * 5.0f;
      if(shift[i] < -1.0f)
        shift[i] = -1.0f;
      if(shift[i] > 1.0f)
        shift[i] = 1.0f;
      // smooth the motion
      if(shift[i] < 0.0f)
      {
        const float x = shift[i] + 1.0f;
        shift[i] = (x * x) - 1.0f;
      }
      else
      {
        const float x = 1.0f - shift[i];
        shift[i] = 1.0f - (x * x);
      }
    }
    for(int i=0; i<6; i++)
    {
      hexa = static_cast<impRoundedHexahedron*>(mShapes[i]);
      if(i < 2)
      {
        m.makeRotate(shift[0] * RS_PIo2, 1.0f, 0.0f, 0.0f);
        m.rotate(shift[1] * RS_PIo2, 0.0f, 1.0f, 0.0f);
        m.rotate(shift[2] * RS_PIo2, 0.0f, 0.0f, 1.0f);
        const float x = offset * shift[3];
        const float y = offset * shift[4];
        const float z = offset * shift[5];
        if(i % 2)
          m.translate(x, 0.0f, z);
        else
          m.translate(0.0f, y, z);
        m.rotate(shift[6] * RS_PIo2 + RS_PIo2, 1.0f, 0.0f, 0.0f);
        m.rotate(shift[7] * RS_PIo2 + RS_PIo2, 0.0f, 1.0f, 0.0f);
        m.rotate(shift[8] * RS_PIo2, 0.0f, 0.0f, 1.0f);
      }
      else if(i < 4)
      {
        m.makeRotate(shift[9] * RS_PIo2, 1.0f, 0.0f, 0.0f);
        m.rotate(shift[10] * RS_PIo2, 0.0f, 1.0f, 0.0f);
        m.rotate(shift[11] * RS_PIo2, 0.0f, 0.0f, 1.0f);
        const float x = (i % 2) ? offset * shift[12] : -offset * shift[12];
        //const float y = ((i / 2) % 2) ? offset * shift[13] : -offset * shift[13];
        m.translate(x, x, 0.0f);
        m.rotate(shift[14] * RS_PIo2 + RS_PIo2, 1.0f, 0.0f, 0.0f);
        m.rotate(shift[15] * RS_PIo2 + RS_PIo2, 0.0f, 1.0f, 0.0f);
        m.rotate(shift[16] * RS_PIo2, 0.0f, 0.0f, 1.0f);
      }
      else
      {
        m.makeRotate(shift[17] * RS_PIo2, 1.0f, 0.0f, 0.0f);
        m.rotate(shift[18] * RS_PIo2, 0.0f, 1.0f, 0.0f);
        m.rotate(shift[19] * RS_PIo2, 0.0f, 0.0f, 1.0f);
        const float x = offset * shift[3];
        const float y = offset * shift[4];
        const float z = offset * shift[5];
        if(i % 2)
          m.translate(-x, 0.0f, -z);
        else
          m.translate(0.0f, -y, -z);
        m.rotate(shift[6] * RS_PIo2 + RS_PIo2, 1.0f, 0.0f, 0.0f);
        m.rotate(shift[7] * RS_PIo2 + RS_PIo2, 0.0f, 1.0f, 0.0f);
        m.rotate(shift[20] * RS_PIo2, 0.0f, 0.0f, 1.0f);
      }
      m.postMult(mMatrix);
      hexa->setMatrix(m.get());
    }

    const float sphereoffset = 0.35f;
    impSphere* sphere = static_cast<impSphere*>(mShapes[6]);
    m.makeTranslate(sphereoffset * sinf(mCoeffPhase[10] * 4.0f), sphereoffset * sinf(mCoeffPhase[11] * 4.0f), sphereoffset * sinf(mCoeffPhase[12] * 4.0f));
    m.postMult(mMatrix);
    sphere->setPosition(&((m.get())[12]));
    sphere->setThickness(0.05f + 0.01f * mCoeff[3]);
  }
};
