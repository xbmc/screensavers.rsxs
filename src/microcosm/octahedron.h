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
#include <Implicit/impCapsule.h>

#include "gizmo.h"

class ATTRIBUTE_HIDDEN Octahedron : public Gizmo
{
public:
  Octahedron(CScreensaverMicrocosm* base) : Gizmo(base)
  {
    for(int i=0; i<3; i++)
    {
      impSphere* sphere = new impSphere;
      sphere->setThickness(0.05f);
      mShapes.push_back(sphere);
    }

    for(int i=0; i<12; i++)
    {
      impCapsule* cap = new impCapsule;
      cap->setThickness(0.03f);
      cap->setLength(0.25f);
      mShapes.push_back(cap);
    }
  }

  ~Octahedron()
  {
    for(unsigned int i=0; i<mShapes.size(); i++)
      delete mShapes[i];
  }

  void update(float frametime)
  {
    updateConstants(frametime);
    updateMatrix();

    const float offset = 0.3f;
    rsMatrix m;
    for(int i=0; i<3; i++)
    {
      m.makeTranslate(offset * cosf(mCoeffPhase[i*2+1] * 5.0f), offset * cosf(mCoeffPhase[i*2+2] * 5.0f), offset * cosf(mCoeffPhase[i*2+3] * 5.0f));
      m.postMult(mMatrix);
      mShapes[i]->setPosition(&((m.get())[12]));
    }

    const float osize = 0.23f;
    impCapsule* cap = static_cast<impCapsule*>(mShapes[3]);
    m.makeRotate(RS_PI * -0.25f, 0.0f, 1.0f, 0.0f);
    m.translate(osize, 0.0f, osize);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());

    cap = static_cast<impCapsule*>(mShapes[4]);
    m.makeRotate(RS_PI * -0.25f, 0.0f, 1.0f, 0.0f);
    m.translate(-osize, 0.0f, -osize);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());

    cap = static_cast<impCapsule*>(mShapes[5]);
    m.makeRotate(RS_PI * 0.25f, 0.0f, 1.0f, 0.0f);
    m.translate(-osize, 0.0f, osize);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());

    cap = static_cast<impCapsule*>(mShapes[6]);
    m.makeRotate(RS_PI * 0.25f, 0.0f, 1.0f, 0.0f);
    m.translate(osize, 0.0f, -osize);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());

    cap = static_cast<impCapsule*>(mShapes[7]);
    m.makeRotate(RS_PI * 0.25f, 1.0f, 0.0f, 0.0f);
    m.translate(0.0f, osize, osize);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());

    cap = static_cast<impCapsule*>(mShapes[8]);
    m.makeRotate(RS_PI * 0.25f, 1.0f, 0.0f, 0.0f);
    m.translate(0.0f, -osize, -osize);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());

    cap = static_cast<impCapsule*>(mShapes[9]);
    m.makeRotate(RS_PI * -0.25f, 1.0f, 0.0f, 0.0f);
    m.translate(0.0f, osize, -osize);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());

    cap = static_cast<impCapsule*>(mShapes[10]);
    m.makeRotate(RS_PI * -0.25f, 1.0f, 0.0f, 0.0f);
    m.translate(0.0f, -osize, osize);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());

    cap = static_cast<impCapsule*>(mShapes[11]);
    m.makeRotate(RS_PI * 0.25f, 1.0f, 0.0f, 0.0f);
    m.rotate(RS_PIo2, 0.0f, 1.0f, 0.0f);
    m.translate(osize, osize, 0.0f);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());

    cap = static_cast<impCapsule*>(mShapes[12]);
    m.makeRotate(RS_PI * 0.25f, 1.0f, 0.0f, 0.0f);
    m.rotate(RS_PIo2, 0.0f, 1.0f, 0.0f);
    m.translate(-osize, -osize, 0.0f);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());

    cap = static_cast<impCapsule*>(mShapes[13]);
    m.makeRotate(RS_PI * -0.25f, 1.0f, 0.0f, 0.0f);
    m.rotate(RS_PIo2, 0.0f, 1.0f, 0.0f);
    m.translate(osize, -osize, 0.0f);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());

    cap = static_cast<impCapsule*>(mShapes[14]);
    m.makeRotate(RS_PI * -0.25f, 1.0f, 0.0f, 0.0f);
    m.rotate(RS_PIo2, 0.0f, 1.0f, 0.0f);
    m.translate(-osize, osize, 0.0f);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());
  }
};
