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

#include <Implicit/impSphere.h>
#include <Implicit/impCapsule.h>

#include "gizmo.h"

class ATTRIBUTE_HIDDEN Tetrahedron : public Gizmo
{
public:
  Tetrahedron(CScreensaverMicrocosm* base) : Gizmo(base)
  {
    for(int i=0; i<3; i++)
    {
      impSphere* sphere = new impSphere;
      sphere->setThickness(0.05f);
      mShapes.push_back(sphere);
    }

    for(int i=0; i<6; i++)
    {
      impCapsule* cap = new impCapsule;
      cap->setThickness(0.03f);
      cap->setLength(0.32f);
      mShapes.push_back(cap);

      rsMatrix m;
      m.makeScale(0.01f, 0.01f, 0.01f);
      cap->setMatrix(m.get());
    }
  }

  ~Tetrahedron()
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

    const float edge_face_angle = -0.61548f;
    const float radius_edge = 0.27f;
    impCapsule* cap = static_cast<impCapsule*>(mShapes[3]);
    m.makeTranslate(radius_edge, 0.0f, 0.0f);
    m.rotate(edge_face_angle, 0.0f, 1.0f, 0.0f);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());

    cap = static_cast<impCapsule*>(mShapes[4]);
    m.makeTranslate(radius_edge, 0.0f, 0.0f);
    m.rotate(edge_face_angle, 0.0f, 1.0f, 0.0f);
    m.rotate(RS_PI * 0.66667f, 0.0f, 0.0f, 1.0f);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());

    cap = static_cast<impCapsule*>(mShapes[5]);
    m.makeTranslate(radius_edge, 0.0f, 0.0f);
    m.rotate(edge_face_angle, 0.0f, 1.0f, 0.0f);
    m.rotate(RS_PI * -0.66667f, 0.0f, 0.0f, 1.0f);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());

    cap = static_cast<impCapsule*>(mShapes[6]);
    m.makeRotate(RS_PI * 0.5f, 1.0f, 0.0f, 0.0f);
    m.translate(radius_edge, 0.0f, 0.0f);
    m.rotate(RS_PI * 0.16667f, 0.0f, 1.0f, 0.0f);
    m.rotate(RS_PI, 0.0f, 0.0f, 1.0f);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());

    cap = static_cast<impCapsule*>(mShapes[7]);
    m.makeRotate(RS_PI * 0.5f, 1.0f, 0.0f, 0.0f);
    m.translate(radius_edge, 0.0f, 0.0f);
    m.rotate(RS_PI * 0.16667f, 0.0f, 1.0f, 0.0f);
    m.rotate(RS_PI * 0.33333f, 0.0f, 0.0f, 1.0f);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());

    cap = static_cast<impCapsule*>(mShapes[8]);
    m.makeRotate(RS_PI * 0.5f, 1.0f, 0.0f, 0.0f);
    m.translate(radius_edge, 0.0f, 0.0f);
    m.rotate(RS_PI * 0.16667f, 0.0f, 1.0f, 0.0f);
    m.rotate(RS_PI * -0.33333f, 0.0f, 0.0f, 1.0f);
    m.postMult(mMatrix);
    cap->setMatrix(m.get());
  }
};
