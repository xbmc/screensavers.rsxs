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

class ATTRIBUTE_HIDDEN Kube2 : public Gizmo
{
public:
  Kube2(CScreensaverMicrocosm* base) : Gizmo(base)
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

  ~Kube2()
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
    float shift[15];
    for(int i=0; i<15; i++)
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
        const float x = (i % 2) ? offset * shift[6] : -offset * shift[6];
        m.translate(x, 0.0f, 0.0f);
        m.rotate(shift[9] * RS_PIo2, 0.0f, 1.0f, 0.0f);
        m.rotate(shift[10] * RS_PIo2, 0.0f, 0.0f, 1.0f);
      }
      else if(i < 4)
      {
        m.makeRotate(shift[2] * RS_PIo2, 1.0f, 0.0f, 0.0f);
        m.rotate(shift[3] * RS_PIo2, 0.0f, 1.0f, 0.0f);
        m.rotate(shift[4] * RS_PIo2, 0.0f, 0.0f, 1.0f);
        const float y = (i % 2) ? offset * shift[7] : -offset * shift[7];
        m.translate(0.0f, y, 0.0f);
        m.rotate(shift[11] * RS_PIo2, 0.0f, 1.0f, 0.0f);
        m.rotate(shift[12] * RS_PIo2, 0.0f, 0.0f, 1.0f);
      }
      else
      {
        m.makeRotate(shift[4] * RS_PIo2, 1.0f, 0.0f, 0.0f);
        m.rotate(shift[5] * RS_PIo2, 0.0f, 1.0f, 0.0f);
        m.rotate(shift[0] * RS_PIo2, 0.0f, 0.0f, 1.0f);
        const float z = (i % 2) ? offset * shift[8] : -offset * shift[8];
        m.translate(0.0f, 0.0f, z);
        m.rotate(shift[13] * RS_PIo2, 0.0f, 1.0f, 0.0f);
        m.rotate(shift[14] * RS_PIo2, 0.0f, 0.0f, 1.0f);
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
