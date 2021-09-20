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

#pragma once

#include "light.h"

#include <kodi/gui/gl/GL.h>
#include <kodi/AddonBase.h>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

class CScreensaverHyperspace;

class ATTRIBUTE_HIDDEN CCausticTextures
{
public:
  // constructor takes the following parameters:
  // num keyframes (only for fractal heightfield generation),
  // number of frames, geometry resolution, texture resolution, water depth, wave amplitude,
  // refraction multiplier (sort of like index of refraction)
  CCausticTextures(CScreensaverHyperspace* base, int keys, int frames, int res, int size, float depth, float wa, float rm);
  ~CCausticTextures();

  ATTRIBUTE_FORCEINLINE GLuint* CausticTex() { return m_caustictex; }

private:
  void makeFractalAltitudes();
  void makeTrigAltitudes();
  void altitudeSquare(int left, int right, int bottom, int top, float** alt);
  void draw(int xlo, int xhi, int zlo, int zhi);
  void makeIndices(int index, int* minus, int* plus);
  float myFabs(float x){if(x<0) return -x; return x;};
  float interpolate(float a, float b, float c, float d, float where);

  int m_numKeys;
  int m_numFrames;
  int m_geoRes;
  int m_texSize;
  float m_waveAmp;
  float m_refractionMult;

  // textures indices for OpenGL texture objects
  GLuint* m_caustictex;

  // space for storing geometry of water surface
  float* m_x;  // x and z are the same for each frame
  float* m_z;
  float*** m_y;  // y (altitude) is different

  float*** m_xz;  // projected vertex positions
  float** m_intensity;  // projected light intensity

  GLubyte** m_bitmap;

  std::vector<sLight> m_lights;
  
  CScreensaverHyperspace* m_base;
};
