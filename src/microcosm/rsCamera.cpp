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

#include <math.h>
#include <glm/glm.hpp>

#include "rsCamera.h"
#include "main.h"

rsCamera::rsCamera(CScreensaverMicrocosm* base) : m_base(base)
{
  setProjectionMatrix(RS_PIo2, 1.0f, 1.0f, 100.0f);
  mViewMat.identity();
}

void rsCamera::setProjectionMatrix(float fovy, float aspectratio, float nearclip, float farclip)
{
  mNear = nearclip;
  mFar = farclip;

  const float A(tanf(fovy*0.5f));
  float projmat[] = {1.0f / (aspectratio * A), 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f / A, 0.0f, 0.0f,
    0.0f, 0.0f, -(mNear + mFar) / (mFar - mNear), -1.0f,
    0.0f, 0.0f, (-2.0f * mNear * mFar) / (mFar - mNear), 0.0f};
  mProjMat.set(projmat);

  // left and right planes' vectors
  {
    const float temp(atanf(aspectratio * A));
    const float cos_temp(cosf(temp));
    const float sin_temp(sinf(temp));
    cullVec[0][0] = cos_temp;
    cullVec[0][1] = 0.0f;
    cullVec[0][2] = -sin_temp;
    cullVec[1][0] = -cos_temp;
    cullVec[1][1] = 0.0f;
    cullVec[1][2] = -sin_temp;
  }

  // bottom and top planes' vectors
  {
    const float temp(fovy * 0.5f);
    const float cos_temp(cosf(temp));
    const float sin_temp(sinf(temp));
    cullVec[2][0] = 0.0f;
    cullVec[2][1] = cos_temp;
    cullVec[2][2] = -sin_temp;
    cullVec[3][0] = 0.0f;
    cullVec[3][1] = -cos_temp;
    cullVec[3][2] = -sin_temp;
  }
}

void rsCamera::apply()
{
  glm::mat4& projMat = m_base->ProjMatrix();
  glm::mat4& modelMat = m_base->ModelMatrix();
  m_projMatOld = projMat;
  m_modelMatOld = modelMat;
  projMat = glm::mat4(mProjMat[0], mProjMat[1], mProjMat[2], mProjMat[3],
                      mProjMat[4], mProjMat[5], mProjMat[6], mProjMat[7],
                      mProjMat[8], mProjMat[9], mProjMat[10], mProjMat[11],
                      mProjMat[12], mProjMat[13], mProjMat[14], mProjMat[15]);
  modelMat = glm::mat4(mViewMat[0], mViewMat[1], mViewMat[2], mViewMat[3],
                       mViewMat[4], mViewMat[5], mViewMat[6], mViewMat[7],
                       mViewMat[8], mViewMat[9], mViewMat[10], mViewMat[11],
                       mViewMat[12], mViewMat[13], mViewMat[14], mViewMat[15]);
}

void rsCamera::revoke()
{
  glm::mat4& projMat = m_base->ProjMatrix();
  glm::mat4& modelMat = m_base->ModelMatrix();
  projMat = m_projMatOld;
  modelMat = m_modelMatOld;
}

bool rsCamera::inViewVolume(rsVec pos, float radius)
{
  pos.transPoint(mViewMat);

  // check far plane
  if(pos[2] < -(mFar + radius))
    return 0;
  // check left plane
  if(pos[0]*cullVec[0][0] + pos[1]*cullVec[0][1] + pos[2]*cullVec[0][2] < -radius)
    return 0;
  // check right plane
  if(pos[0]*cullVec[1][0] + pos[1]*cullVec[1][1] + pos[2]*cullVec[1][2] < -radius)
    return 0;
  // check bottom plane
  if(pos[0]*cullVec[2][0] + pos[1]*cullVec[2][1] + pos[2]*cullVec[2][2] < -radius)
    return 0;
  // check top plane
  if(pos[0]*cullVec[3][0] + pos[1]*cullVec[3][1] + pos[2]*cullVec[3][2] < -radius)
    return 0;

  return true;
}
