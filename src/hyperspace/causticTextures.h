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

#pragma once

#include <kodi/gui/gl/GL.h>
#include <kodi/AddonBase.h>
#include <glm/gtc/type_ptr.hpp>

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

  CScreensaverHyperspace* m_base;
};
