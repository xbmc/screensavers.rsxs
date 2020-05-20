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

#include <kodi/AddonBase.h>
#include <rsMath/rsMath.h>
#include <glm/gtc/type_ptr.hpp>

class CScreensaverMicrocosm;

class ATTRIBUTE_HIDDEN rsCamera
{
public:
  rsMatrix mProjMat;
  rsMatrix mViewMat;
  float mNear, mFar;
  float cullVec[4][3];  // vectors perpendicular to viewing volume planes

  rsCamera(CScreensaverMicrocosm* base);
  ~rsCamera(){}
  void setProjectionMatrix(float fovy, float aspectratio, float nearclip, float farclip);
  void setViewMatrix(float* mat){ mViewMat.set(mat); }
  void setViewMatrix(rsMatrix &mat){ mViewMat = mat; }

  // Test to see if a sphere is partially or wholly contained in view volume
  bool inViewVolume(rsVec pos, float radius);

  // Apply projection and modelview matrices with glPushMatrix()es
  void apply();

  // glPopMatrix()es, to be called sometime after apply()
  void revoke();

private:
  glm::mat4 m_projMatOld;
  glm::mat4 m_modelMatOld;
  CScreensaverMicrocosm* m_base;
};
