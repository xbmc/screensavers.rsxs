/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 1999-2010 Terence M. Welsh
 *  Ported to Kodi by Alwin Esch <alwinus@kodi.tv>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

/*
 * Code is based on:
 *   https://github.com/reallyslickscreensavers/reallyslickscreensavers
 *   http://rss-glx.sourceforge.net/
 * and reworked to GL 4.0.
 */

#include "splinePath.h"
#include <rsMath/rsMath.h>
#include <math.h>

CSplinePath::CSplinePath(int length)
{
  m_step = 0.0f;

  m_numPoints = length;
  // 6 is the minimum number of points necessary for a tunnel to have one segment
  if (m_numPoints < 6)
    m_numPoints = 6;

  m_phase = new float[m_numPoints];
  m_rate = new float[m_numPoints];
  m_movevec = new float*[m_numPoints];
  m_basexyz = new float*[m_numPoints];
  m_xyz = new float*[m_numPoints];
  m_basedir = new float*[m_numPoints];
  m_dir = new float*[m_numPoints];
  for (int i = 0; i < m_numPoints; i++)
  {
    m_movevec[i] = new float[3];
    m_basexyz[i] = new float[3];
    m_xyz[i] = new float[3];
    m_basedir[i] = new float[3];
    m_dir[i] = new float[3];
  }

  m_basexyz[m_numPoints-1][0] = 0.0f;
  m_basexyz[m_numPoints-1][1] = 0.0f;
  m_basexyz[m_numPoints-1][2] = 0.0f;
  m_basexyz[m_numPoints-2][0] = 0.0f;
  m_basexyz[m_numPoints-2][1] = 0.0f;
  m_basexyz[m_numPoints-2][2] = 4.0f;

  for (int i = 0; i < m_numPoints; i++)
    MakeNewPoint();
}

CSplinePath::~CSplinePath()
{
  for (int i = 0; i < m_numPoints; i++)
  {
    delete[] m_movevec[i];
    delete[] m_basexyz[i];
    delete[] m_xyz[i];
    delete[] m_basedir[i];
    delete[] m_dir[i];
  }

  delete[] m_phase;
  delete[] m_rate;
  delete[] m_movevec;
  delete[] m_basexyz;
  delete[] m_xyz;
  delete[] m_basedir;
  delete[] m_dir;
}

void CSplinePath::MoveAlongPath(float increment)
{
  m_step += increment;
  while(m_step >= 1.0f)
  {
    m_step -= 1.0f;
    MakeNewPoint();
  }
}

// "section" indicates the pair of points you're between (from 1 to numPoints-1)
// "where" indicates where you are between that pair of points (0.0 - 1.0)
// "position" receives the information about the point you want
void CSplinePath::GetPoint(int section, float where, glm::vec3& position)
{
  if (section < 1)
    section = 1;
  if (section > m_numPoints - 3)
    section = m_numPoints - 3;

  position.x = Interpolate(m_xyz[section-1][0], m_xyz[section][0],
    m_xyz[section+1][0], m_xyz[section+2][0], where);
  position.y = Interpolate(m_xyz[section-1][1], m_xyz[section][1],
    m_xyz[section+1][1], m_xyz[section+2][1], where);
  position.z = Interpolate(m_xyz[section-1][2], m_xyz[section][2],
    m_xyz[section+1][2], m_xyz[section+2][2], where);
}

// "section" indicates the pair of points you're between (from 1 to numPoints-1)
// "where" indicates where you are between that pair of points (0.0 - 1.0)
// "direction" receives the information about the direction you want
void CSplinePath::GetDirection(int section, float where, float* direction)
{
  if (section < 1)
    section = 1;
  if (section > m_numPoints - 3)
    section = m_numPoints - 3;

  direction[0] = Interpolate(m_dir[section-1][0], m_dir[section][0],
    m_dir[section+1][0], m_dir[section+2][0], where);
  direction[1] = Interpolate(m_dir[section-1][1], m_dir[section][1],
    m_dir[section+1][1], m_dir[section+2][1], where);
  direction[2] = Interpolate(m_dir[section-1][2], m_dir[section][2],
    m_dir[section+1][2], m_dir[section+2][2], where);

  const float normalizer(1.0f / sqrtf(direction[0] * direction[0]
    + direction[1] * direction[1] + direction[2] * direction[2]));
  direction[0] *= normalizer;
  direction[1] *= normalizer;
  direction[2] *= normalizer;
}

// "section" indicates the pair of points you're between (from 1 to numPoints-1)
// "where" indicates where you are between that pair of points (0.0 - 1.0)
// "direction" receives the information about the direction you want
void CSplinePath::GetBaseDirection(int section, float where, float* direction)
{
  if (section < 1)
    section = 1;
  if (section > m_numPoints - 3)
    section = m_numPoints - 3;

  direction[0] = Interpolate(m_basedir[section-1][0], m_basedir[section][0],
    m_basedir[section+1][0], m_basedir[section+2][0], where);
  direction[1] = Interpolate(m_basedir[section-1][1], m_basedir[section][1],
    m_basedir[section+1][1], m_basedir[section+2][1], where);
  direction[2] = Interpolate(m_basedir[section-1][2], m_basedir[section][2],
    m_basedir[section+1][2], m_basedir[section+2][2], where);

  const float normalizer(1.0f / sqrtf(direction[0] * direction[0]
    + direction[1] * direction[1] + direction[2] * direction[2]));
  direction[0] *= normalizer;
  direction[1] *= normalizer;
  direction[2] *= normalizer;
}

void CSplinePath::Update(float multiplier)
{
  int i;

  // calculate xyz positions
  for (i = 0; i < m_numPoints; i++)
  {
    m_phase[i] += m_rate[i] * multiplier;
    m_xyz[i][0] = m_basexyz[i][0] + m_movevec[i][0] * cosf(m_phase[i]);
    m_xyz[i][1] = m_basexyz[i][1] + m_movevec[i][1] * cosf(m_phase[i]);
    m_xyz[i][2] = m_basexyz[i][2] + m_movevec[i][2] * cosf(m_phase[i]);
  }

  // calculate direction vectors
  for (i = 1; i < m_numPoints-1; i++)
  {
    m_dir[i][0] = m_xyz[i+1][0] - m_xyz[i-1][0];
    m_dir[i][1] = m_xyz[i+1][1] - m_xyz[i-1][1];
    m_dir[i][2] = m_xyz[i+1][2] - m_xyz[i-1][2];
  }
}

void CSplinePath::MakeNewPoint()
{
  int i;

  // shift points to rear of path
  for (i = 0; i < m_numPoints-1; i++)
  {
    m_basexyz[i][0] = m_basexyz[i+1][0];
    m_basexyz[i][1] = m_basexyz[i+1][1];
    m_basexyz[i][2] = m_basexyz[i+1][2];
    m_movevec[i][0] = m_movevec[i+1][0];
    m_movevec[i][1] = m_movevec[i+1][1];
    m_movevec[i][2] = m_movevec[i+1][2];
    m_xyz[i][0] = m_xyz[i+1][0];
    m_xyz[i][1] = m_xyz[i+1][1];
    m_xyz[i][2] = m_xyz[i+1][2];
    m_phase[i] = m_phase[i+1];
    m_rate[i] = m_rate[i+1];
  }

  // make vector to new point
  int lastPoint = m_numPoints - 1;
  float tempx = m_basexyz[lastPoint-1][0] - m_basexyz[lastPoint-2][0];
  float tempz = m_basexyz[lastPoint-1][2] - m_basexyz[lastPoint-2][2];

  // find good angle
  float turnAngle;
  const float pathAngle = atan2f(tempx, tempz);
  const float dist_from_center = sqrtf(m_basexyz[lastPoint][0] * m_basexyz[lastPoint][0] + m_basexyz[lastPoint][2] * m_basexyz[lastPoint][2]);
  if (dist_from_center > 100.0f)
  {
    const float angleToCenter = atan2f(-m_basexyz[lastPoint][0], -m_basexyz[lastPoint][2]);
    turnAngle = angleToCenter - pathAngle;
    if (turnAngle > M_PI)
      turnAngle -= RS_PIx2;
    if (turnAngle < -M_PI)
      turnAngle += RS_PIx2;
    if (turnAngle > 0.7f)
      turnAngle = 0.7f;
    if (turnAngle < -0.7f)
      turnAngle = -0.7f;
  }
  else
    turnAngle = rsRandf(1.4f) - 0.7f;

  // rotate new point to some new position
  float ca = cosf(turnAngle);
  float sa = sinf(turnAngle);
  m_basexyz[lastPoint][0] = tempx * ca + tempz * sa;
  m_basexyz[lastPoint][1] = 0.0f;
  m_basexyz[lastPoint][2] = tempx * -sa + tempz * ca;

  // normalize and set length of vector
  // make it at least length 2, which is the grid size of the goo
  float lengthener = (rsRandf(6.0f) + 2.0f) / sqrtf(m_basexyz[lastPoint][0] * m_basexyz[lastPoint][0]
    + m_basexyz[lastPoint][2] * m_basexyz[lastPoint][2]);
  m_basexyz[lastPoint][0] *= lengthener;
  m_basexyz[lastPoint][2] *= lengthener;

  // make new movement vector proportional to base vector
  m_movevec[lastPoint][0] = rsRandf(0.25f) * -m_basexyz[lastPoint][2];
  m_movevec[lastPoint][1] = 0.3f;
  m_movevec[lastPoint][2] = rsRandf(0.25f) * m_basexyz[lastPoint][0];

  // add vector to previous point to get new point
  m_basexyz[lastPoint][0] += m_basexyz[lastPoint-1][0];
  m_basexyz[lastPoint][2] += m_basexyz[lastPoint-1][2];

  // make new phase and movement rate
  m_phase[lastPoint] = rsRandf(RS_PIx2);
  m_rate[lastPoint] = rsRandf(1.0f);

  // reset base direction vectors
  for (i = 1; i < m_numPoints-2; i++)
  {
    m_basedir[i][0] = m_basexyz[i+1][0] - m_basexyz[i-1][0];
    m_basedir[i][1] = m_basexyz[i+1][1] - m_basexyz[i-1][1];
    m_basedir[i][2] = m_basexyz[i+1][2] - m_basexyz[i-1][2];
  }
}

// Here's a little calculus that takes 4 points along a single
// dimension and interpolates smoothly between the second and third
// depending on the value of where which can be 0.0 to 1.0.
// The slope at b is estimated using a and c.  The slope at c
// is estimated using b and d.
float CSplinePath::Interpolate(float a, float b, float c, float d, float where)
{
  float q, r, s, t;

  q = (((3.0f * b) + d - a - (3.0f * c)) * (where * where * where)) * 0.5f;
  r = (((2.0f * a) - (5.0f * b) + (4.0f * c) - d) * (where * where)) * 0.5f;
  s = ((c - a) * where) * 0.5f;
  t = b;
  return(q + r + s + t);
}
