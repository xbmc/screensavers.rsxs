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

class ATTRIBUTE_HIDDEN Kube4 : public Gizmo
{
public:
  Kube4(CScreensaverMicrocosm* base) : Gizmo(base)
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

  ~Kube4()
  {
    for(unsigned int i=0; i<mShapes.size(); i++)
      delete mShapes[i];
  }

  void update(float frametime)
  {
    updateConstants(frametime);
    updateMatrix();

    const float offset = 0.36f;
    const float offset2 = 0.095f;
    float n;
    rsMatrix m;
    impRoundedHexahedron* hexa;
    float shift[12];
    for(int i=0; i<12; i++)
    {
      shift[i] = mCoeff[i] * 6.0f;
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
      const float nn = cosf(mCoeffPhase[6] * 4.0f + float(i) * RS_PIx2 / 6.0f);
      if(nn >= 0.0f)
        n = 1.0f - (1.0f - nn) * (1.0f - nn);
      else
        n = (1.0f + nn) * (1.0f + nn) - 1.0f;
      n *= offset;
      switch(i)
      {
      case 0:
        m.makeRotate(RS_PIo2 * shift[0], 1.0f, 0.0f, 0.0f);
        m.rotate(RS_PIo2 * shift[1], 0.0f, 1.0f, 0.0f);
        m.translate(n, offset2, offset2);
        break;
      case 1:
        m.makeRotate(RS_PIo2 * shift[2], 0.0f, 1.0f, 0.0f);
        m.rotate(RS_PIo2 * shift[3], 0.0f, 0.0f, 1.0f);
        m.translate(n, -offset2, -offset2);
        break;
      case 2:
        m.makeRotate(RS_PIo2 * shift[4], 1.0f, 0.0f, 0.0f);
        m.rotate(RS_PIo2 * shift[5], 0.0f, 0.0f, 1.0f);
        m.translate(offset2, n, offset2);
        break;
      case 3:
        m.makeRotate(RS_PIo2 * shift[6], 1.0f, 0.0f, 0.0f);
        m.rotate(RS_PIo2 * shift[7], 0.0f, 1.0f, 0.0f);
        m.translate(-offset2, n, -offset2);
        break;
      case 4:
        m.makeRotate(RS_PIo2 * shift[8], 0.0f, 1.0f, 0.0f);
        m.rotate(RS_PIo2 * shift[9], 0.0f, 0.0f, 1.0f);
        m.translate(offset2, offset2, n);
        break;
      case 5:
        m.makeRotate(RS_PIo2 * shift[10], 1.0f, 0.0f, 0.0f);
        m.rotate(RS_PIo2 * shift[11], 0.0f, 0.0f, 1.0f);
        m.translate(-offset2, -offset2, n);
        break;
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
