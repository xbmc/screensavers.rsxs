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

#include <glm/gtc/type_ptr.hpp>
#include <kodi/AddonBase.h>
#include <kodi/gui/gl/GL.h>

#define FLARESIZE 64

class CScreensaverHyperspace;

class ATTRIBUTE_HIDDEN CFlare
{
public:
  CFlare() = default;
  ~CFlare();

  /*
   * Generate textures for lens flares
   * then applies textures to geometry in display lists
   */
  void Init(CScreensaverHyperspace* base);

  /*
   * Draw a flare at a specified (x,y) location on the screen
   * Screen corners are at (0,0) and (1,1)
   * alpha = 0.0 for lowest intensity; alpha = 1.0 for highest intensity
   */
  void Draw(glm::vec3 const& pos, float red, float green, float blue, float alpha);

  /*
   * Externed so flares can be drawn elsewhere
   */
  unsigned int* FlareTex() { return m_flaretex; }

private:
  float m_flicker = 0.95f;

  unsigned char m_flare1[FLARESIZE][FLARESIZE][4];
  unsigned char m_flare2[FLARESIZE][FLARESIZE][4];
  unsigned char m_flare3[FLARESIZE][FLARESIZE][4];
  unsigned char m_flare4[FLARESIZE][FLARESIZE][4];
  unsigned int m_flaretex[4];

  sLight m_light[4];
  CScreensaverHyperspace* m_base;
};
