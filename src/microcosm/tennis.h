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

#include <rsMath/rsMath.h>
#include <Implicit/impRoundedHexahedron.h>
#include <Implicit/impSphere.h>

#include "gizmo.h"
#include "main.h"

class ATTRIBUTE_HIDDEN Tennis : public Gizmo
{
private:
  rsVec pos;
  rsVec dir;

public:
  Tennis(CScreensaverMicrocosm* base) : Gizmo(base)
  {
    mMaxDisplacement = 0.4f;

    pos.set(0.0f, 0.0f, 0.0f);
    dir.set(1.0f + rsRandf(1.0f), rsRandf(1.0f), rsRandf(1.0f));
    dir.normalize();

    for(int i=0; i<2; i++)
    {
      impRoundedHexahedron* hexa = new impRoundedHexahedron;
      hexa->setThickness(0.03f);
      hexa->setSize(0.0f, 0.1f, 0.1f);
      mShapes.push_back(hexa);
    }

    impSphere* sphere = new impSphere;
    sphere->setThickness(0.05f);
    mShapes.push_back(sphere);
  }

  ~Tennis()
  {
    for(unsigned int i=0; i<mShapes.size(); i++)
      delete mShapes[i];
  }

  void update(float frametime)
  {
    updateConstants(frametime);
    updateMatrix();

    rsMatrix m;
    const float soffset = 0.35f;
    pos += dir * (frametime * 0.05f * float(m_base->Settings().dGizmoSpeed));
    bool newdirection = false;
    if(pos[0] < -soffset)
    {
      pos[0] += -soffset - pos[0];
      dir[0] *= -1.0f;
      newdirection = true;
    }
    else if(pos[0] > soffset)
    {
      pos[0] += soffset - pos[0];
      dir[0] *= -1.0f;
      newdirection = true;
    }
    if(pos[1] < -soffset)
    {
      pos[1] += -soffset - pos[1];
      dir[1] *= -1.0f;
    }
    else if(pos[1] > soffset)
    {
      pos[1] += soffset - pos[1];
      dir[1] *= -1.0f;
    }
    if(pos[2] < -soffset)
    {
      pos[2] += -soffset - pos[2];
      dir[2] *= -1.0f;
    }
    else if(pos[2] > soffset)
    {
      pos[2] += soffset - pos[2];
      dir[2] *= -1.0f;
    }
    if(newdirection)
    {
      dir[1] += rsRandf(0.5f) - 0.25f;
      dir[2] += rsRandf(0.5f) - 0.25f;
      dir.normalize();
      if(dir[0] > -0.3f && dir[0] < 0.3f){
        dir[0] *= 2.0f;
        dir.normalize();
      }
    }
    impSphere* sphere = static_cast<impSphere*>(mShapes[2]);
    m.makeTranslate(pos);
    m.postMult(mMatrix);
    sphere->setPosition(&((m.get())[12]));

    const float hoffset = 0.3f;
    impRoundedHexahedron* hexa = static_cast<impRoundedHexahedron*>(mShapes[0]);
    rsVec driftpos(-0.43f, hoffset * cosf(mCoeffPhase[0] * 6.0f), hoffset * cosf(mCoeffPhase[1] * 6.0f));
    rsVec hitpos(-0.43f, pos[1], pos[2]);
    float t = -pos[0] / soffset;
    if(t < 0.0f)
      t = 0.0f;
    if(t > 1.0f)
      t = 1.0f;
    m.makeTranslate(driftpos * (1.0f - t) + hitpos * t);
    m.postMult(mMatrix);
    hexa->setMatrix(m.get());

    hexa = static_cast<impRoundedHexahedron*>(mShapes[1]);
    driftpos.set(0.43f, hoffset * cosf(mCoeffPhase[0] * 6.0f), hoffset * cosf(mCoeffPhase[1] * 6.0f));
    hitpos.set(0.43f, pos[1], pos[2]);
    t = pos[0] / soffset;
    if(t < 0.0f)
      t = 0.0f;
    if(t > 1.0f)
      t = 1.0f;
    m.makeTranslate(driftpos * (1.0f - t) + hitpos * t);
    m.postMult(mMatrix);
    hexa->setMatrix(m.get());
  }
};
