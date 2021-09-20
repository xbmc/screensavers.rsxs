/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 1999-2010 Terence M. Welsh
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "camera.h"

void CCamera::init(const float* mat, float f)
{
  float temp;

  // far clipping plane
  farplane = f;

  // bottom and top planes' vectors
  temp = atanf(1.0f / mat[5]);
  cullVec[0][0] = 0.0f;
  cullVec[0][1] = cosf(temp);
  cullVec[0][2] = -sinf(temp);
  cullVec[1][0] = 0.0f;
  cullVec[1][1] = -cullVec[0][1];
  cullVec[1][2] = cullVec[0][2];

  // left and right planes' vectors
  temp = atanf(1.0f / mat[0]);
  cullVec[2][0] = cosf(temp);
  cullVec[2][1] = 0.0f;
  cullVec[2][2] = -sinf(temp);
  cullVec[3][0] = -cullVec[2][0];
  cullVec[3][1] = 0.0f;
  cullVec[3][2] = cullVec[2][2];
}

bool CCamera::inViewVolume(float* pos, float radius)
{
  // check back plane
  if (pos[2] < -(farplane + radius))
    return false;
  // check bottom plane
  else if (pos[0]*cullVec[0][0] + pos[1]*cullVec[0][1] + pos[2]*cullVec[0][2] < -radius)
    return false;
  // check top plane
  else if (pos[0]*cullVec[1][0] + pos[1]*cullVec[1][1] + pos[2]*cullVec[1][2] < -radius)
    return false;
  // check left plane
  else if (pos[0]*cullVec[2][0] + pos[1]*cullVec[2][1] + pos[2]*cullVec[2][2] < -radius)
    return false;
  // check right plane
  else if (pos[0]*cullVec[3][0] + pos[1]*cullVec[3][1] + pos[2]*cullVec[3][2] < -radius)
    return false;
  return true;
}
