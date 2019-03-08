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
 *
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

#define FLARESIZE 128

class CScreensaverSkyRocket;

enum flareType
{
  FLARE_BASIC_SPHERE = 0,
  FLARE_FLATTENED_SPHERE = 1,
  FLARE_TORUS = 2,
  FLARE_WEIRD = 3
};

struct sFlare
{
  sLight light[4];
  GLuint texture;
  void SetColor(const sColor& color)
  {
    light[0].color = color;
    light[1].color = color;
    light[2].color = color;
    light[3].color = color;
  }
};

class CFlare
{
public:
  CFlare(CScreensaverSkyRocket* base) : m_base(base) { };
  ~CFlare();

  // Generate textures for lens flares
  // then applies textures to geometry in display lists
  void Init();

  // Draw a flare at a specified (x,y) location on the screen
  // Screen corners are at (0,0) and (1,1)
  // alpha = 0.0 for lowest intensity; alpha = 1.0 for highest intensity
  void Flare(float x, float y, float red, float green, float blue, float alpha);
  void Draw(flareType type, const sColor& color);
  inline sFlare* Flares() { return m_flares; }

  struct data
  {
    float x, y, r, g, b, a;
  };

private:
  CScreensaverSkyRocket* m_base;
  sFlare m_flares[4];
  GLuint m_flaretex[4];
  unsigned char m_flare1[FLARESIZE][FLARESIZE][4];
  unsigned char m_flare2[FLARESIZE][FLARESIZE][4];
  unsigned char m_flare3[FLARESIZE][FLARESIZE][4];
  unsigned char m_flare4[FLARESIZE][FLARESIZE][4];
};
