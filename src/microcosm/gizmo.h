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

// A Gizmo is a collection of impShapes that interact with one another

#include <Implicit/impShape.h>
#include <rsMath/rsMath.h>
#include <kodi/AddonBase.h>
#include <vector>

#define GIZMO_NUM_COEFFS 25

typedef std::vector <impShape*> ShapeVector;

class CScreensaverMicrocosm;

class ATTRIBUTE_HIDDEN Gizmo
{
protected:
  CScreensaverMicrocosm* m_base;

  ShapeVector mShapes;
  // to be set to compute time relative to other Gizmos
  float mComputeComplexity;

  // Amount to displace Gizmo when scale = 1.0
  float mMaxDisplacement;

  // scale factor
  float mScale;

  // Stuff to make Gizmo wander around
  float mCoeffRate[GIZMO_NUM_COEFFS];  // coefficients' rates
  float mCoeffPhase[GIZMO_NUM_COEFFS];  // coefficients' phases
  float mCoeff[GIZMO_NUM_COEFFS];  // coefficients that oscillate within {-1.0, 1.0}
  rsMatrix mMatrix;

  void updateConstants(float frametime);
  void updateMatrix();

public:
  Gizmo(CScreensaverMicrocosm* base);
  ~Gizmo();

  virtual void setScale(float s);

  // returns the value of this group of shapes at a given position
  // "position" is an array of 3 floats
  float value(float* position);

  // Add Gizmo's shapes to global vector of shapes
  // so that all shapes can be cycled through quickly.
  void getShapes(ShapeVector &sv);

  // Adds surface crawler start position(s) to given crawlPointVector
  void addCrawlPoints(impCrawlPointVector &cpv);
  virtual void update(float frametime);
};
