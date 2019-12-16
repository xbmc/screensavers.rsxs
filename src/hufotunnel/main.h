/*
 *  Copyright (C) 2005-2019 Team Kodi
 *  Copyright (C) 2002 Jeremie Allard (Hufo / N.A.A.)
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
 *   http://rss-glx.sourceforge.net/
 * and reworked to GL 4.0.
 */

#pragma once

#include <kodi/addon-instance/Screensaver.h>
#include <kodi/gui/gl/GL.h>
#include <kodi/gui/gl/Shader.h>
#include <glm/gtc/type_ptr.hpp>
#include <rsMath/rsMath.h>

#define XSTD (4.0f/3.0f)

#define HVCtrX 0.0f
#define HVCtrY 0.0f
#define HoleFocX 2.0f
#define HoleFocY 2.0f

#define SHFTHTPS 8
#define HOLEGEN 25
#define HoleNbParImg 64
#define HoleNbImg 128
#define HOLEVIT 8

#define TUNNELCOLORRND 0.15f
#define TUNNELCOLORFACT 0.9999f

struct sLight
{
  glm::vec3 vertex;
  glm::vec3 color;
  glm::vec2 coord;
};

class ATTRIBUTE_HIDDEN CScreensaverHufoTunnel
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver,
    public kodi::gui::gl::CShaderProgram
{
public:
  CScreensaverHufoTunnel() = default;

  bool Start() override;
  void Stop() override;
  void Render() override;

  void OnCompiledAndLinked() override;
  bool OnEnabled() override;

private:
  void DrawEntry(int primitive, const sLight* data, unsigned int size);

  float m_tHole;      // tunnel time
  float m_tVit;

  struct THole
  {
    float u, v;       // point position on the plane
    float c1, c2;     // color coefficients
  };
  THole m_hole[HoleNbImg][HoleNbParImg];
  THole m_refHole[HoleNbParImg];

  struct THoleTraj
  {
    rsVec a;          // angles
    rsVec o, m, n;    // vectors based on the plan
    float s;          // circle size
  };
  THoleTraj m_holeTraj[HoleNbImg];

  int m_holeLastP;    // last plan calculated
  int m_holeNbImgA;   // number of plans displayed
  bool m_stopHole = false;

  void HoleInitPlan(int p, int t, float ss = 1.0);
  void HoleInit();

  struct BBox2D
  {
    float u0, v0;
    float u1, v1;
  };

  void CalcBBoxPlan(int p, BBox2D * b);
  void InterLnCircle(double u, double v, double w, float *x1, float *y1, float *x2, float *y2);
  void InterBBox(BBox2D * a, BBox2D const *b);
  bool BBoxEmpty(BBox2D const *b);
  void MkBBoxAll(BBox2D * b);

  BBox2D m_bBPlan[HoleNbImg];

  struct HPT {
    float ex, ey;
    float u, v;
    float c1;
  } m_pt[HoleNbImg][HoleNbParImg + 1];
  float m_ptDist[HoleNbImg];

  void CalcHole(int T);

  GLubyte m_idx[4] = {0, 1, 3, 2};

  int m_uniformColorUsed = 0;
  glm::vec3 m_uniformColor;

  glm::mat4 m_modelProjMat;

  GLint m_modelViewProjectionMatrixLoc = -1;
  GLint m_unformColorLoc = -1;
  GLint m_unformColorUsedLoc = -1;
  GLint m_textureUsedLoc = -1;
  GLint m_positionLoc = -1;
  GLint m_colorLoc = -1;
  GLint m_texCoord0Loc = -1;

  GLuint m_vertexVBO = 0;

  GLuint m_texture = 0;

  bool m_startOK = false;
  double m_lastTime;;
};
