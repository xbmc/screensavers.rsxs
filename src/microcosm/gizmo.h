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

// A Gizmo is a collection of impShapes that interact with one another

#include <Implicit/impShape.h>
#include <rsMath/rsMath.h>
#include <kodi/AddonBase.h>
#include <vector>

#define GIZMO_NUM_COEFFS 25

typedef std::vector <impShape*> ShapeVector;

class CScreensaverMicrocosm;

class ATTR_DLL_LOCAL Gizmo
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
