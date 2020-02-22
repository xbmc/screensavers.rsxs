/*
 *  Copyright (C) 2005-2019 Team Kodi
 *  Copyright (C) 1999-2010 Terence M. Welsh
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
 *   http://rss-glx.sourceforge.net/
 * and reworked to GL 4.0.
 */

#include "causticTextures.h"
#include "main.h"
#include "mipmap.h"

#include <math.h>
#include <rsMath/rsMath.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

CCausticTextures::CCausticTextures(CScreensaverHyperspace* base, int keys, int frames, int res, int size, float depth, float wa, float rm)
  : m_base(base)
{
  int i, j, k;
  int xminus, xplus, zminus, zplus;
  int viewport[4];
  unsigned char* bitmap = new unsigned char[size * size * 3];

  // initialize dimensions
  m_numKeys = keys;
  if (m_numKeys < 2)
    m_numKeys = 2;
  m_numFrames = frames;
  if (m_numFrames < m_numKeys * 2)
    m_numFrames = m_numKeys * 2;
  m_geoRes = res;
  if (m_geoRes < 8)
    m_geoRes = 8;
  m_texSize = size;
  if (m_texSize < 8)
    m_texSize = 8;
  m_waveAmp = wa;
  m_refractionMult = rm;

  m_caustictex = new GLuint[m_numFrames];
  glGenTextures(m_numFrames, m_caustictex);

  // allocate memory
  m_x = new float[m_geoRes + 1];
  m_z = new float[m_geoRes + 1];
  m_y = new float**[m_numFrames];
  for (k = 0; k < m_numFrames; k++)
  {
    m_y[k] = new float*[m_geoRes];
    for (i = 0; i < m_geoRes; i++)
      m_y[k][i] = new float[m_geoRes];
  }
  m_xz = new float**[m_geoRes + 1];
  for (i = 0; i <= m_geoRes; i++)
  {
    m_xz[i] = new float*[m_geoRes + 1];
    for (j = 0; j <= m_geoRes; j++)
      m_xz[i][j] = new float[2];
  }
  m_intensity = new float*[m_geoRes + 1];
  for (i = 0; i <= m_geoRes; i++)
    m_intensity[i] = new float[m_geoRes + 1];

  // set x and z geometry positions
  for (i = 0; i <= m_geoRes; i++){
    m_x[i] = float(i) / float(m_geoRes);
    m_z[i] = float(i) / float(m_geoRes);
  }

  // set m_y geometry positions (altitudes)
  // fractal altitudes is sort of ugly, so I don't use it
  //makeFractalAltitudes();
  makeTrigAltitudes();

  // prepare to draw textures
  glGetIntegerv(GL_VIEWPORT, viewport);
  glViewport(0, 0, m_texSize, m_texSize);

  glm::mat4& projMat = m_base->ProjectionMatrix();
  glm::mat4 projMatOld = projMat;
  glm::mat4& modelMat = m_base->ModelMatrix();
  glm::mat4 modelMatOld = modelMat;

  projMat = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, -0.5f, 0.5f);
  modelMat = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0));
  glReadBuffer(GL_BACK);
  //glPixelStorei(GL_UNPACK_ROW_LENGTH, m_texSize);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  m_base->BindTexture(GL_TEXTURE_2D, 0);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);

  // project vertices and create textures
  float recvert = float(m_geoRes) * 0.5f;  // reciprocal of vertical component of light ray
  for (k = 0; k < m_numFrames; k++)
  {
    // compute projected offsets
    // (this uses surface normals, not actual refractions, but it's faster this way)
    for (i = 0; i < m_geoRes; i++)
    {
      for (j = 0; j < m_geoRes; j++)
      {
        makeIndices(i, &xminus, &xplus);
        m_xz[i][j][0] = (m_y[k][xplus][j] - m_y[k][xminus][j]) * recvert * (depth + m_y[k][i][j]);
        makeIndices(j, &zminus, &zplus);
        m_xz[i][j][1] = (m_y[k][i][zplus] - m_y[k][i][zminus]) * recvert * (depth + m_y[k][i][j]);
      }
    }

    // copy offsets to edges of m_xz array
    for (i = 0; i < m_geoRes; i++)
    {
      m_xz[i][m_geoRes][0] = m_xz[i][0][0];
      m_xz[i][m_geoRes][1] = m_xz[i][0][1];
    }
    for (j = 0; j <= m_geoRes; j++)
    {
      m_xz[m_geoRes][j][0] = m_xz[0][j][0];
      m_xz[m_geoRes][j][1] = m_xz[0][j][1];
    }

    // compute light intensities
    float space = 1.0f / float(m_geoRes);
    for (i = 0; i < m_geoRes; i++)
    {
      for (j = 0; j < m_geoRes; j++)
      {
        makeIndices(i, &xminus, &xplus);
        makeIndices(j, &zminus, &zplus);
        // this assumes nominal light m_intensity is 0.25
        m_intensity[i][j] = (1.0f / (float(m_geoRes) * float(m_geoRes)))
          / ((myFabs(m_xz[xplus][j][0] - m_xz[i][j][0] + space)
          + myFabs(m_xz[i][j][0] - m_xz[xminus][j][0] + space))
          * (myFabs(m_xz[i][zplus][1] - m_xz[i][j][1] + space)
          + myFabs(m_xz[i][j][1] - m_xz[i][zminus][1] + space)))
          - 0.125f;
        if (m_intensity[i][j] > 1.0f)
          m_intensity[i][j] = 1.0f;
      }
    }

    // copy intensities to edges of m_intensity array
    for (i = 0; i < m_geoRes; i++)
      m_intensity[i][m_geoRes] = m_intensity[i][0];
    for (j = 0; j <= m_geoRes; j++)
      m_intensity[m_geoRes][j] = m_intensity[0][j];

    // draw texture
    glClear(GL_COLOR_BUFFER_BIT);
    // draw most of texture
    draw(0, m_geoRes, 0, m_geoRes);
    // draw edges of texture that wrap around from opposite sides
    int numRows = m_geoRes / 10;
    glm::mat4 modelMatCallBase = modelMat;

    modelMat = glm::translate(modelMat, glm::vec3(-1.0f, 0.0f, 0.0f));
    draw(m_geoRes - numRows, m_geoRes, 0, m_geoRes);
    modelMat = modelMatCallBase;

    modelMat = glm::translate(modelMat, glm::vec3(1.0f, 0.0f, 0.0f));
    draw(0, numRows, 0, m_geoRes);
    modelMat = modelMatCallBase;

    modelMat = glm::translate(modelMat, glm::vec3(0.0f, 0.0f, -1.0f));
    draw(0, m_geoRes, m_geoRes - numRows, m_geoRes);
    modelMat = modelMatCallBase;

    modelMat = glm::translate(modelMat, glm::vec3(0.0f, 0.0f, 1.0f));
    draw(0, m_geoRes, 0, numRows);
    modelMat = modelMatCallBase;

    // draw corners too
    modelMat = glm::translate(modelMat, glm::vec3(-1.0f, 0.0f, -1.0f));
    draw(m_geoRes - numRows, m_geoRes, m_geoRes - numRows, m_geoRes);
    modelMat = modelMatCallBase;

    modelMat = glm::translate(modelMat, glm::vec3(1.0f, 0.0f, -1.0f));
    draw(0, numRows, m_geoRes - numRows, m_geoRes);
    modelMat = modelMatCallBase;

    modelMat = glm::translate(modelMat, glm::vec3(-1.0f, 0.0f, 1.0f));
    draw(m_geoRes - numRows, m_geoRes, 0, numRows);
    modelMat = modelMatCallBase;

    modelMat = glm::translate(modelMat, glm::vec3(1.0f, 0.0f, 1.0f));
    draw(0, numRows, 0, numRows);
    modelMat = modelMatCallBase;

    // read back texture
    glReadPixels(0, 0, m_texSize, m_texSize, GL_RGB, GL_UNSIGNED_BYTE, bitmap);

    // create texture object
    glBindTexture(GL_TEXTURE_2D, m_caustictex[k]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    Build2DMipmaps(GL_TEXTURE_2D, GL_RGB, m_texSize, m_texSize, GL_RGB, GL_UNSIGNED_BYTE, bitmap);
  }

  // restore matrix stack
  modelMat = modelMatOld;
  projMat = projMatOld;

  glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

  for (k = 0; k < m_numFrames; k++)
  {
    for (i = 0; i < m_geoRes; i++)
      delete[] m_y[k][i];
    delete[] m_y[k];
  }
  for (i = 0; i <= m_geoRes; i++)
  {
    for (j = 0; j <= m_geoRes; j++)
      delete[] m_xz[i][j];
    delete[] m_xz[i];
  }
  for (i = 0; i <= m_geoRes; i++)
    delete[] m_intensity[i];

  delete[] m_x;
  delete[] m_y;
  delete[] m_z;
  delete[] m_xz;
  delete[] m_intensity;
  delete[] bitmap;
}

CCausticTextures::~CCausticTextures()
{
  glDeleteTextures(m_numFrames, m_caustictex);
  delete[] m_caustictex;
}

void CCausticTextures::makeFractalAltitudes()
{
  int i, j, k, a;
  float keySeparation = float(m_numFrames) / float(m_numKeys);
  int* keyFrame = new int[m_numKeys+1];

  // generate keyframes
  float phase = 0.0f;
  for (k = 0; k < m_numKeys; k++)
  {
    keyFrame[k] = int(float(k) * keySeparation);
    // set all keyframe altitudes to 0.0
    for (i = 0; i < m_geoRes; i++)
    {
      for (j = 0; j < m_geoRes; j++)
        m_y[keyFrame[k]][i][j] = 0.0f;
    }
    // generate altitudes in first positions
    phase = float(k) * RS_PIx2 / float(m_numKeys);
    m_y[keyFrame[k]][0][0] = m_waveAmp * rsCosf(1.5707f + phase);
    m_y[keyFrame[k]][m_geoRes/2][0] = m_waveAmp * rsCosf(1.5707f + phase);
    m_y[keyFrame[k]][m_geoRes/2][m_geoRes/2] = m_waveAmp * rsCosf(3.1416f + phase);
    m_y[keyFrame[k]][0][m_geoRes/2] = m_waveAmp * rsCosf(4.7124f + phase);
    // recurse to find remaining altitudes
    altitudeSquare(0, m_geoRes/2, 0, m_geoRes/2, m_y[keyFrame[k]]);
    altitudeSquare(m_geoRes/2, m_geoRes, 0, m_geoRes/2, m_y[keyFrame[k]]);
    altitudeSquare(0, m_geoRes/2, m_geoRes/2, m_geoRes, m_y[keyFrame[k]]);
    altitudeSquare(m_geoRes/2, m_geoRes, m_geoRes/2, m_geoRes, m_y[keyFrame[k]]);
  }

  // interpolate to find remaining frames
  int diff, kk;
  int kf0, kf1, kf2, kf3;  // keyframe indices
  float where;
  keyFrame[m_numKeys] = m_numFrames;
  for (k = 0; k < m_numKeys; k++)
  {
    kf1 = keyFrame[k];
    kf2 = keyFrame[k+1];
    diff = kf2 - kf1;
    if (kf2 == m_numFrames)
      kf2 = 0;
    kk = k - 1;
    if (kk < 0)
      kk = m_numKeys - 1;
    kf0 = keyFrame[kk];
    kk = k + 2;
    if (kk >= m_numKeys)
      kk -= m_numKeys;
    kf3 = keyFrame[kk];
    if (diff > 1)
    {
      for (a = kf1+1; a < kf1+diff; a++)
      {
        where = float(a-kf1) / float(diff);
        for (i = 0; i < m_geoRes; i++)
          for (j = 0; j < m_geoRes; j++)
            m_y[a][i][j] = interpolate(m_y[kf0][i][j], m_y[kf1][i][j],
              m_y[kf2][i][j], m_y[kf3][i][j], where);
      }
    }
  }

  delete[] keyFrame;
}

void CCausticTextures::altitudeSquare(int left, int right, int bottom, int top, float** alt)
{
  // find wrapped indices
  int rr = right;
  if (rr == m_geoRes)
    rr = 0;
  int tt = top;
  if (tt == m_geoRes)
    tt = 0;

  // for determining if there is a gap to be filled
  int hor = right - left;
  int vert = top - bottom;

  int centerHor = (left + right) / 2;
  int centerVert = (bottom + top) / 2;
  float offset;
  if (hor > 1)  // find bottom and top altitudes
  {
    offset = myFabs(m_waveAmp * float(m_x[right] - m_x[left]));
    if (alt[centerHor][bottom] == 0.0f)
      alt[centerHor][bottom] = (alt[left][bottom] + alt[rr][bottom]) * 0.5f
        + rsRandf(offset+offset) - offset;
    if (alt[centerHor][tt] == 0.0f)
      alt[centerHor][tt] = (alt[left][tt] + alt[rr][tt]) * 0.5f
        + rsRandf(offset+offset) - offset;
  }
  if (vert > 1)  // find left and right altitudes
  {
    offset = myFabs(m_waveAmp * float(m_z[top] - m_z[bottom]));
    if (alt[left][centerVert] == 0.0f)
      alt[left][centerVert] = (alt[left][bottom] + alt[left][tt]) * 0.5f
        + rsRandf(offset+offset) - offset;
    if (alt[rr][centerVert] == 0.0f)
      alt[rr][centerVert] = (alt[rr][bottom] + alt[rr][tt]) * 0.5f
        + rsRandf(offset+offset) - offset;
  }
  if (hor > 1 && vert > 1)  // find center altitude
  {
    offset = m_waveAmp * 0.5f *
      (myFabs(float(m_x[right] - m_x[left]))
      + myFabs(float(m_z[top] - m_z[bottom])));
    alt[centerHor][centerVert] = (alt[left][bottom] + alt[rr][bottom] + alt[left][tt]
      + alt[rr][tt]) * 0.25f + rsRandf(offset+offset) - offset;
  }

  // keep recursing if necessary
  int quadrant[4] = {0, 0, 0, 0};
  if (centerHor - left > 1)
  {
    quadrant[0] ++;
    quadrant[2] ++;
  }
  if (right - centerHor > 1)
  {
    quadrant[1] ++;
    quadrant[2] ++;
  }
  if (centerVert - bottom > 1)
  {
    quadrant[0] ++;
    quadrant[1] ++;
  }
  if (top - centerVert > 1)
  {
    quadrant[2] ++;
    quadrant[3] ++;
  }
  if (quadrant[0])
    altitudeSquare(left, centerHor, bottom, centerVert, alt);
  if (quadrant[1])
    altitudeSquare(centerHor, right, bottom, centerVert, alt);
  if (quadrant[2])
    altitudeSquare(left, centerHor, centerVert, top, alt);
  if (quadrant[3])
    altitudeSquare(centerHor, right, centerVert, top, alt);

  return;
}

void CCausticTextures::makeTrigAltitudes()
{
  int i, j, k;
  float xx, zz, offset;

  for (k = 0; k < m_numFrames; k++)
  {
    offset = RS_PIx2 * float(k) / float(m_numFrames);
    for (i = 0; i < m_geoRes; i++)
    {
      xx = RS_PIx2 * float(i) / float(m_geoRes);
      for (j = 0; j < m_geoRes; j++)
      {
        zz = RS_PIx2 * float(j) / float(m_geoRes);
        /*m_y[k][i][j] = m_waveAmp
          * (0.12f * rsCosf(xx + 2.0f * offset)
          + 0.08f * rsCosf(-1.0f * xx + 2.0f * zz + offset)
          + 0.04f * rsCosf(-2.0f * xx - 4.0f * zz + offset)
          + 0.014f * rsCosf(xx - 7.0f * zz - 2.0f * offset)
          + 0.014f * rsCosf(3.0f * xx + 5.0f * zz + offset)
          + 0.014f * rsCosf(9.0f * xx + zz - offset)
          + 0.007f * rsCosf(11.0f * xx + 7.0f * zz - offset)
          + 0.007f * rsCosf(4.0f * xx - 13.0f * zz + offset)
          + 0.007f * rsCosf(19.0f * xx - 9.0f * zz - offset));*/
        m_y[k][i][j] = m_waveAmp
          * (0.08f * rsCosf(xx * 2.0f + offset)
          + 0.06f * rsCosf(-1.0f * xx + 2.0f * zz + offset)
          + 0.04f * rsCosf(-2.0f * xx - 3.0f * zz + offset)
          + 0.01f * rsCosf(xx - 7.0f * zz - 2.0f * offset)
          + 0.01f * rsCosf(3.0f * xx + 5.0f * zz + offset)
          + 0.01f * rsCosf(9.0f * xx + zz - offset)
          + 0.005f * rsCosf(11.0f * xx + 7.0f * zz - offset)
          + 0.005f * rsCosf(4.0f * xx - 13.0f * zz + offset)
          + 0.003f * rsCosf(19.0f * xx - 9.0f * zz - offset));
      }
    }
  }
}

void CCausticTextures::draw(int xlo, int xhi, int zlo, int zhi)
{
  int i, j;
  float mult;
  unsigned int ptr = 0;
  m_lights.resize((xhi+1)*2);

  for (j = zlo; j < zhi; j++)
  {
    // red
    mult = 1.0f - m_refractionMult / float(m_geoRes);
    for (i = xlo; i <= xhi; i++)
    {
      m_lights[ptr  ].color = sColor(m_intensity[i][j+1], 0.0f, 0.0f);
      m_lights[ptr++].vertex = sPosition(m_x[i] + m_xz[i][j+1][0] * mult, 0.0f, m_z[j+1] + m_xz[i][j+1][1] * mult);
      m_lights[ptr  ].color = sColor(m_intensity[i][j], 0.0f, 0.0f);
      m_lights[ptr++].vertex = sPosition(m_x[i] + m_xz[i][j][0] * mult, 0.0f, m_z[j] + m_xz[i][j][1] * mult);
    }
    m_base->Draw(GL_TRIANGLE_STRIP, m_lights.data(), ptr);
    ptr = 0;

    // green
    for (i = xlo; i <= xhi; i++)
    {
      m_lights[ptr  ].color = sColor(0.0f, m_intensity[i][j+1], 0.0f);
      m_lights[ptr++].vertex = sPosition(m_x[i] + m_xz[i][j+1][0], 0.0f, m_z[j+1] + m_xz[i][j+1][1]);
      m_lights[ptr  ].color = sColor(0.0f, m_intensity[i][j], 0.0f);
      m_lights[ptr++].vertex = sPosition(m_x[i] + m_xz[i][j][0], 0.0f, m_z[j] + m_xz[i][j][1]);
    }
    m_base->Draw(GL_TRIANGLE_STRIP, m_lights.data(), ptr);
    ptr = 0;

    // blue
    mult = 1.0f + m_refractionMult / float(m_geoRes);
    for (i = xlo; i <= xhi; i++)
    {
      m_lights[ptr  ].color = sColor(0.0f, 0.0f, m_intensity[i][j+1]);
      m_lights[ptr++].vertex = sPosition(m_x[i] + m_xz[i][j+1][0] * mult, 0.0f, m_z[j+1] + m_xz[i][j+1][1] * mult);
      m_lights[ptr  ].color = sColor(0.0f, 0.0f, m_intensity[i][j]);
      m_lights[ptr++].vertex = sPosition(m_x[i] + m_xz[i][j][0] * mult, 0.0f, m_z[j] + m_xz[i][j][1] * mult);
    }
    m_base->Draw(GL_TRIANGLE_STRIP, m_lights.data(), ptr);
    ptr = 0;
  }
}

void CCausticTextures::makeIndices(int index, int* minus, int* plus)
{
  *minus = index - 1;
  if (*minus < 0)
    *minus = m_geoRes - 1;
  *plus = index + 1;
  if (*plus >= m_geoRes)
    *plus = 0;
}

// Here's a little calculus that takes 4 points along a single
// dimension and interpolates smoothly between the second and third
// depending on the value of where which can be 0.0 to 1.0.
// The slope at b is estimated using a and c.  The slope at c
// is estimated using b and d.
float CCausticTextures::interpolate(float a, float b, float c, float d, float where)
{
  float q, r, s, t;

  q = (((3.0f * b) + d - a - (3.0f * c)) * (where * where * where)) * 0.5f;
  r = (((2.0f * a) - (5.0f * b) + (4.0f * c) - d) * (where * where)) * 0.5f;
  s = ((c - a) * where) * 0.5f;
  t = b;
  return(q + r + s + t);
}
