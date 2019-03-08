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
