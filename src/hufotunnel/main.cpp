/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2002 Jeremie Allard (Hufo / N.A.A.)
 *  Ported to Kodi by Alwin Esch <alwinus@kodi.tv>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

/*
 * Code is based on:
 *   http://rss-glx.sourceforge.net/
 * and reworked to GL 4.0.
 */

#include "main.h"
#include "marblemap.h"
#include "swirlmap.h"

#include <chrono>
#include <kodi/gui/General.h>
#include <kodi/gui/gl/Texture.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <bzlib.h>

#define LOAD_TEXTURE(dest, src, compressedSize, size) dest = (unsigned char *)malloc(size); BZ2_bzBuffToBuffDecompress((char *)dest, &size, (char *)src, compressedSize, 0, 0);
#define FREE_TEXTURE(tex) free(tex);

namespace {
struct sHufoTunnelSettings
{
  void Load()
  {
    kodi::CheckSettingBoolean("general.autoselection", dAutoSelection);
    if (dAutoSelection)
    {
      dTexture = rsRandi(2) + 1;
      dCoarse = 1 << (3 - rsRandi(4));

      if (dCoarse == 8)
        dCoarse = 0;

      dSinHole = rsRandi(2);
      dWireframe = (rsRandi(10) == 0);
    }
    else
    {
      dTexture = kodi::GetSettingInt("general.type");
      dWireframe = kodi::GetSettingBoolean("general.wireframe");
      dSinHole = kodi::GetSettingBoolean("general.sinusoide");
      dCoarse = 1 << (3 - kodi::GetSettingInt("general.coarseness"));
      if (dCoarse == 8)
        dCoarse = 0;
    }
  }

  bool dAutoSelection = true;
  int dTexture = 2;
  int dCoarse = 0;
  bool dWireframe = false;
  bool dSinHole = false;
} gSettings;
}

/*
Optimization of the tunnel display:
the tunnel is made up of circles.
we can stop the display of the tunnel when the rest hides the entire screen
if you keep an area of visibility that shows the area still visible on the screen
after the display of a tunnel, it stops when this area is empty.
We therefore need:
- a function that calculates the area of the tunnel hole after displaying a given plane
- a function that calculates the intersection of two zones
For their simplicity we will choose rectangles
*/

/* calculating the projected rectangle
  the next points are on the circle: P=O+M*sin(a)+N*cos(a)
  Px=ox+sa*mx+ca+nx Py=oy+sa*my+ca+ny Pz=oz+sa*mz+ca+nz
  ex=cx+fx*Px/Py
  ey=cy-fy*Pz/Py
  we are looking for ex and ey terminals
  ex=exmin or exmax <=> dex/da=0
  ey=eymin or eymax <=> dey/da=0
  dex/da=fx*(Py * dPx/da - Px * dPy/da) / Py² = 0
  <=> Py * dPx/da - Px * dPy/da = 0
  (camx-sanx)(oy+samy+cany)-(ox+samx+canx)(camy-sany)=0
   camxoy+casamxmy+ca²mxny-sanxoy-sa²nxmy-casanxny
  -camyox-casamymx-ca²mynx+sanyox+sa²nymx+casanxny   =0
  ca(mxoy-myox)+sa(nyox-nxoy)+(ca²+sa²)(mxny-mynx)   =0
  ca(mxoy-myox)+sa(nyox-nxoy)=mynx-mxny

  (oymx-oxmy)ca + (-oynx+oxny)sa + (mxny-mynx)ca² + (mxny-mynx)sa² =0
  (oymx-oxmy)ca + (-oynx+oxny)sa = mynx-mxny et ca²+sa²=1
  U*ca+V*sa=W et ca²+sa²=1

  Ux+Vy=W d=1/sqrt(U²+V²),u=U*d,v=V*d,w=W*d => ux+vy=w (u²+v²=1)
  distance between the right and the origin:d=abs(w)
  point at this distance:
  vx-uy=0 && ux+vy=w
  => x=uw
  => y=vw
  si d>1 -> fault
  the solutions are (x,y)+-(1-d)*(-v,u)
  x1=uw-sqrt(1-sqr(d))*v y1=vw+sqrt(1-sqr(d))*u
  x2=uw+sqrt(1-sqr(d))*v y2=vw-sqrt(1-sqr(d))*u
*/

bool CScreensaverHufoTunnel::Start()
{
  gSettings.Load();

  std::string fraqShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/frag.glsl");
  std::string vertShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
    return false;

  // Reset the matrix to something we know.
  float x = (float)Width() / (float)Height();  // Correct the viewing ratio of the window in the X axis.

  // Window initialization
  glViewport(X(), Y(), Width(), Height());

  if (x > XSTD)
    m_modelProjMat = glm::ortho(-x, x, -1.0f, 1.0f);  // Reset to a 2D screen space.
  else
    m_modelProjMat = glm::ortho(-XSTD, XSTD, -XSTD / x, XSTD / x);  // Reset to a 2D screen space.

  if (!gSettings.dWireframe)
  {
    if (gSettings.dTexture)
    {
      unsigned char *l_tex;
      if (gSettings.dTexture == 1)
      {
        LOAD_TEXTURE (l_tex, swirlmap, swirlmap_compressedsize, swirlmap_size)
        gli::texture Texture(gli::TARGET_2D, gli::FORMAT_RGB8_UNORM_PACK8, gli::texture::extent_type(128, 128, 1), 1, 1, 1);
        std::memcpy(Texture.data(), l_tex, Texture.size());
        m_texture = kodi::gui::gl::Load(Texture);
      }
      else
      {
        LOAD_TEXTURE (l_tex, marblemap, marblemap_compressedsize, marblemap_size)
        gli::texture Texture(gli::TARGET_2D, gli::FORMAT_RGB8_UNORM_PACK8, gli::texture::extent_type(256, 256, 1), 1, 1, 1);
        std::memcpy(Texture.data(), l_tex, Texture.size());
        m_texture = kodi::gui::gl::Load(Texture);
      }
      FREE_TEXTURE (l_tex)
    }
    else
    {
      m_texture = 0;
      glBindTexture(GL_TEXTURE_2D, m_texture);
    }
  }

  glCullFace(GL_FRONT);  // reject fliped faces
  glEnable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);  // no zbuffer

  m_tHole = 0.0;
  m_tVit = 8000.0;
  HoleInit();    // initialise tunnel pos

  glGenBuffers(1, &m_vertexVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);

  m_uniformColorUsed = 0;
  m_lastTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  m_startOK = true;

  return true;
}

void CScreensaverHufoTunnel::Stop()
{
  if (!m_startOK)
    return;

  m_startOK = false;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &m_vertexVBO);
  m_vertexVBO = 0;

  if (m_texture)
  {
    glDeleteTextures(1, &m_texture);
    m_texture = 0;
  }

  glCullFace(GL_BACK);
  glDisable(GL_CULL_FACE);
}

void CScreensaverHufoTunnel::Render()
{
  if (!m_startOK)
    return;

  /*
   * Following Extra work done here in render to prevent problems with controls
   * from Kodi and during window moving.
   * TODO: Maybe add a separate interface call to inform about?
   */
  //@{
  glCullFace(GL_FRONT);  // reject fliped faces
  glEnable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);  // no zbuffer
  glBindTexture(GL_TEXTURE_2D, m_texture);
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);

  glVertexAttribPointer(m_positionLoc, 3, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, vertex)));
  glEnableVertexAttribArray(m_positionLoc);

  glVertexAttribPointer(m_colorLoc, 3, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, color)));
  glEnableVertexAttribArray(m_colorLoc);

  glVertexAttribPointer(m_texCoord0Loc, 2, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, coord)));
  glEnableVertexAttribArray(m_texCoord0Loc);
  //@}

  double currentTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  float frameTime = static_cast<float>(currentTime - m_lastTime);
  m_lastTime = currentTime;

  m_tHole += frameTime * m_tVit;
  CalcHole ((int)m_tHole);  // animate the tunnel
  if (m_holeNbImgA == 0)
  {
    glClear(GL_COLOR_BUFFER_BIT);
    return;
  }

  // and render it
  glClear(GL_COLOR_BUFFER_BIT);
  int p, i;
  float f1, f2;

  for (p = m_holeNbImgA - 2; p >= 0; --p)
  {
    f1 = std::min (1.0f, 1.0f / (0.1f + m_ptDist[p] * (0.15f)));
    f2 = std::min (1.0f, 1.0f / (0.1f + m_ptDist[p + 1] * (0.15f)));

    int primitive;
    if (gSettings.dWireframe)
      primitive = GL_LINES;
    else
      primitive = GL_TRIANGLE_STRIP;

    sLight entries[(HoleNbParImg+1)*2];
    unsigned int ptr = 0;
    for (i = 0; i <= HoleNbParImg; i += ((gSettings.dCoarse > 0) ? gSettings.dCoarse : 1))
    {
      float f;

      if (gSettings.dCoarse)
      {
        f = f1 * m_pt[p][i].c1;
        entries[ptr].color = glm::vec3(f, f, f);
      }
      else
        entries[ptr].color = glm::vec3(f1, f1, f1);

      entries[ptr  ].coord = glm::vec2(m_pt[p][i].u, m_pt[p][i].v);
      entries[ptr++].vertex = glm::vec3(m_pt[p][i].ex, m_pt[p][i].ey, 0.0f);

      if (gSettings.dCoarse)
      {
        f = f2 * m_pt[p + 1][i].c1;
        entries[ptr].color = glm::vec3(f, f, f);
      }
      else
      {
        entries[ptr].color = glm::vec3(f2, f2, f2);
      }

      entries[ptr].coord = glm::vec2(m_pt[p + 1][i].u, m_pt[p + 1][i].v);
      entries[ptr++].vertex = glm::vec3(m_pt[p + 1][i].ex, m_pt[p + 1][i].ey, 0.0f);
    }

    m_uniformColorUsed = 0;
    DrawEntry(primitive, entries, ptr);

    if (gSettings.dWireframe)
    {
      m_uniformColorUsed = 1;

      if (BBoxEmpty (&m_bBPlan[p]))
        m_uniformColor = glm::vec3(1.0f, 0.0f, 0.0f);
      else
        m_uniformColor = glm::vec3(0.0f, 1.0f, 0.0f);

      sLight line1[5];
      line1[0].vertex = glm::vec3(m_bBPlan[p].u0, m_bBPlan[p].v0, 0.0f);
      line1[1].vertex = glm::vec3(m_bBPlan[p].u1, m_bBPlan[p].v0, 0.0f);
      line1[2].vertex = glm::vec3(m_bBPlan[p].u1, m_bBPlan[p].v1, 0.0f);
      line1[3].vertex = glm::vec3(m_bBPlan[p].u0, m_bBPlan[p].v1, 0.0f);
      line1[4].vertex = glm::vec3(m_bBPlan[p].u0, m_bBPlan[p].v0, 0.0f);
      DrawEntry(GL_LINE_STRIP, line1, 5);

      ptr = 0;
      m_uniformColor = glm::vec3(f1, f1, f1);
      for (i = 0; i <= HoleNbParImg; i += ((gSettings.dCoarse > 0) ? gSettings.dCoarse : 1))
      {
        entries[ptr  ].coord = glm::vec2(m_pt[p][i].u, m_pt[p][i].v);
        entries[ptr  ].color = glm::vec3(f1, f1, f1);
        entries[ptr++].vertex = glm::vec3(m_pt[p][i].ex, m_pt[p][i].ey, 0.0f);
      }

      DrawEntry(GL_LINE_STRIP, entries, ptr);
      m_uniformColorUsed = 0;
    }
  }

  glDisableVertexAttribArray(m_positionLoc);
  glDisableVertexAttribArray(m_colorLoc);
  glDisableVertexAttribArray(m_texCoord0Loc);

  glCullFace(GL_BACK);
  glDisable(GL_CULL_FACE);
}

void CScreensaverHufoTunnel::DrawEntry(int primitive, const sLight* data, unsigned int size)
{
  EnableShader();
  glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*size, data, GL_DYNAMIC_DRAW);
  glDrawArrays(primitive, 0, size);
  DisableShader();
}

void CScreensaverHufoTunnel::HoleInitPlan(int p, int t, float ss/* = 1.0f*/)
{
  float c1, c2;
  float s = ss;

  // calc color and position

  for (int i = 0; i < HoleNbParImg; i++)
  {
    if (gSettings.dCoarse)
    {
      c1 = m_refHole[i].c1 + rsRandf (2 * TUNNELCOLORRND) - TUNNELCOLORRND;
      if (c1 < 0)
        c1 = 0;
      else if (c1 > 1)
        c1 = 1;
      c2 = m_refHole[i].c2 + rsRandf (2 * TUNNELCOLORRND) - TUNNELCOLORRND;
      if (c2 < 0)
        c2 = 0;
      else if (c2 > 1)
        c2 = 1;

      m_hole[p][i].c1 = c1;
      m_hole[p][i].c2 = c2;
      s = ss * (2.0f - c1);
    }

    m_hole[p][i].u = m_refHole[i].u * s;
    m_hole[p][i].v = m_refHole[i].v * s;
  }

  if (gSettings.dCoarse)
  {
    // color smoothing
    for (int i = 0; i < HoleNbParImg; i++)
    {
      m_refHole[i].c1 = (m_hole[p][i].c1 + m_hole[p][(i + 1) & (HoleNbParImg - 1)].c1 + m_hole[p][(i - 1) & (HoleNbParImg - 1)].c1) * TUNNELCOLORFACT / 3;
      m_refHole[i].c2 = (m_hole[p][i].c2 + m_hole[p][(i + 1) & (HoleNbParImg - 1)].c2 + m_hole[p][(i - 1) & (HoleNbParImg - 1)].c2) * TUNNELCOLORFACT / 3;
    }
  }

  m_holeTraj[p].s = s * HOLEGEN;

  if (gSettings.dCoarse)
  {
    // calc trajectory (based on c2)

    static float c2a_0 = 0;
    static float c2b_0 = 0;

#define C2_VIT 0.01f

    float c2a = m_refHole[0].c2;
    float c2b = m_refHole[HoleNbParImg / 2].c2;

    if (c2a < c2a_0 - C2_VIT)
      c2a_0 -= C2_VIT;
    else if (c2a > c2a_0 + C2_VIT)
      c2a_0 += C2_VIT;
    else
      c2a_0 = c2a;

    if (c2b < c2b_0 - C2_VIT)
      c2b_0 -= C2_VIT;
    else if (c2b > c2b_0 + C2_VIT)
      c2b_0 += C2_VIT;
    else
      c2b_0 = c2a;

    float az = 2 * c2a_0 - 1.0f;

    az = az * 2;
    float ax = 2 * c2b_0 - 1.0f;

    ax = ax * 2;

    m_holeTraj[p].a[1] = 0;  //(float)(sin(t*glm::pi<float>()/90)*4*HOLEVIT*glm::pi<float>()/1500);
    m_holeTraj[p].a[2] = (az * az * az * 2) * HOLEVIT * glm::pi<float>() / 2500;  //(float)(sin(t*glm::pi<float>()/170)*2*HOLEVIT*glm::pi<float>()/2500);
    m_holeTraj[p].a[0] = (ax * ax * ax * 2) * HOLEVIT * glm::pi<float>() / 2500;  //(float)(sin(t*glm::pi<float>()/170)*2*HOLEVIT*glm::pi<float>()/2500);
  }
  else
  {
    m_holeTraj[p].a[0] = sinf(t * glm::pi<float>() / 40) * HOLEVIT * glm::pi<float>() / 1500;
    m_holeTraj[p].a[1] = sinf(t * glm::pi<float>() / 90) * 4 * HOLEVIT * glm::pi<float>() / 1500;
    m_holeTraj[p].a[2] = sinf(t * glm::pi<float>() / 70) * 2 * HOLEVIT * glm::pi<float>() / 1500;
  }
}

void CScreensaverHufoTunnel::HoleInit()
{
  for (int i = 0; i < HoleNbParImg; i++)
  {
    m_refHole[i].u = sinf(i * (2 * glm::pi<float>() / HoleNbParImg)) * HOLEGEN;
    m_refHole[i].v = cosf(i * (2 * glm::pi<float>() / HoleNbParImg)) * HOLEGEN;
    m_refHole[i].c1 = 0;
    m_refHole[i].c2 = 0;
  }
  m_holeLastP = -1;
}

/*
 see calculations at the end
*/
void CScreensaverHufoTunnel::CalcBBoxPlan(int p, BBox2D * b)
{
  rsVec o, m, n;

  o = m_holeTraj[p].o;
  m = m_holeTraj[p].m * m_holeTraj[p].s;
  n = m_holeTraj[p].n * m_holeTraj[p].s;
  float ca1, ca2, sa1, sa2;
  float f1, f2;

  //ca(mxoy-myox)+sa(nyox-nxoy)=mynx-mxny
  InterLnCircle (o[1] * m[0] - o[0] * m[1], o[0] * n[1] - o[1] * n[0], m[1] * n[0] - m[0] * n[1], &ca1, &sa1, &ca2, &sa2);
  f1 = HVCtrX + HoleFocX * (o[0] + m[0] * sa1 + n[0] * ca1) / (o[1] + m[1] * sa1 + n[1] * ca1);
  f2 = HVCtrX + HoleFocX * (o[0] + m[0] * sa2 + n[0] * ca2) / (o[1] + m[1] * sa2 + n[1] * ca2);
  if (f1 < f2)
  {
    b->u0 = f1;
    b->u1 = f2;
  }
  else
  {
    b->u0 = f2;
    b->u1 = f1;
  }
  //ca(mzoy-myoz)+sa(nyoz-nzoy)=mynz-mzny
  InterLnCircle (o[1] * m[2] - o[2] * m[1], o[2] * n[1] - o[1] * n[2], m[1] * n[2] - m[2] * n[1], &ca1, &sa1, &ca2, &sa2);
  f1 = HVCtrY - HoleFocY * (o[2] + m[2] * sa1 + n[2] * ca1) / (o[1] + m[1] * sa1 + n[1] * ca1);
  f2 = HVCtrY - HoleFocY * (o[2] + m[2] * sa2 + n[2] * ca2) / (o[1] + m[1] * sa2 + n[1] * ca2);
  if (f1 < f2)
  {
    b->v0 = f1;
    b->v1 = f2;
  }
  else
  {
    b->v0 = f2;
    b->v1 = f1;
  }
}

void CScreensaverHufoTunnel::InterLnCircle(double u, double v, double w, float *x1, float *y1, float *x2, float *y2)
{
  double d = 1.0f / sqrt(u * u + v * v);

  u *= d;
  v *= d;
  w *= d;
#ifdef _DEBUG
  assert (abs (w) < 1.0f);
#endif
  d = sqrt (1.0f - w * w);
  *x1 = w * u - d * v;
  *y1 = w * v + d * u;
  *x2 = w * u + d * v;
  *y2 = w * v - d * u;
#if defined(_DEBUG) && !defined(_WIN32)
  assert (abs ((*x1) * u + (*y1) * v - w) < 1e-6);
  assert (abs ((*x2) * u + (*y2) * v - w) < 1e-6);
  assert (abs (sqr (*x1) + sqr (*y1) - 1.0f) < 1e-6);
  assert (abs (sqr (*x2) + sqr (*y2) - 1.0f) < 1e-6);
#endif
}

void CScreensaverHufoTunnel::InterBBox(BBox2D * a, BBox2D const *b)
{
#define max(a, b) ((a > b) ? a : b);
#define min(a, b) ((a > b) ? b : a);

  a->u0 = max (a->u0, b->u0);
  a->u1 = min (a->u1, b->u1);
  a->v0 = max (a->v0, b->v0);
  a->v1 = min (a->v1, b->v1);
}

bool CScreensaverHufoTunnel::BBoxEmpty(BBox2D const *b)
{
  return (b->u0 > b->u1 || b->v0 > b->v1);
}

void CScreensaverHufoTunnel::MkBBoxAll(BBox2D * b)
{
  b->u0 = -1e10;
  b->u1 = 1e10;
  b->v0 = -1e10;
  b->v1 = 1e10;
}

void CScreensaverHufoTunnel::CalcHole(int T)
{
  float ft = (T & ((1 << SHFTHTPS) - 1)) * (1.0f / (1 << SHFTHTPS));
  int it = T >> SHFTHTPS;
  int i, p, s;

  // Premiere etape : calcul de la position des plans
  rsVec o(0, 0, 0);
  rsVec m(1, 0, 0);
  rsVec v(0, HOLEVIT, 0);
  rsVec n(0, 0, 1);
  rsQuat mm;
  BBox2D bplan, bhole;

  MkBBoxAll(&bhole);
  for (i = 0; i < HoleNbImg && !BBoxEmpty (&bhole); i++)
  {
    p = (i + it) & (HoleNbImg - 1);
    if (i + it > m_holeLastP)  // le plan n'a pas été calculé
    {
      if (m_stopHole)
        break;  // on ne calcule plus de plan
      HoleInitPlan(p, i + it, (gSettings.dSinHole ? (sinf((i + it) * glm::pi<float>() / 10) + 4.0f) / 4 : 1.0f));
      m_holeLastP = i + it;
    }
    if (!i)
      o += v * (1 - ft);
    else
      o += v;
    m_holeTraj[p].o = o;
    m_holeTraj[p].m = m;
    m_holeTraj[p].n = n;
    if (!i)
      mm.fromEuler(m_holeTraj[p].a[0] * (1 - ft), m_holeTraj[p].a[1] * (1 - ft), m_holeTraj[p].a[2] * (1 - ft));
    else
      mm.fromEuler(m_holeTraj[p].a[0], m_holeTraj[p].a[1], m_holeTraj[p].a[2]);
    m = mm.apply(m);
    v = mm.apply(v);
    n = mm.apply(n);
    CalcBBoxPlan (p, &bplan);
    m_bBPlan[i] = bhole;
    InterBBox (&bhole, &bplan);
  }
  m_holeNbImgA = i;
  if (!m_holeNbImgA)
    return;
  // Deuxieme etape : position des points
  // Equations:
  // un point de coordonnees (u,v) du plan (O,m,n)
  // on a : s>=o>+u*m>+v*n>
  // px=ox+u*mx+v*nx
  // py=oy+u*my+v*ny
  // pz=oz+u*mz+v*nz
  rsVec pp;
  float txtv;
  float pu, pv;

//      for (i=HoleNbImgA-1;i>=0;i--)
  for (i = 0; i < m_holeNbImgA; ++i)
  {
    m_ptDist[i] = i + 1.0f - ft;
    p = (i + it) & (HoleNbImg - 1);
    o = m_holeTraj[p].o;
    m = m_holeTraj[p].m;
    n = m_holeTraj[p].n;
    txtv = (i + it) * (2.0 / HoleNbImg);
    for (s = 0; s <= HoleNbParImg; s += ((gSettings.dCoarse > 0) ? gSettings.dCoarse : 1))
    {
      pu = m_hole[p][s & (HoleNbParImg - 1)].u;
      pv = m_hole[p][s & (HoleNbParImg - 1)].v;
      //pp=o+m*pu+n*pv;
      pp[0] = o[0] + pu * m[0] + pv * n[0];
      pp[1] = o[1] + pu * m[1] + pv * n[1];
      pp[2] = o[2] + pu * m[2] + pv * n[2];
      if (pp[1] <= 0)
        pp[1] = 0.1f;  // en cas de probleme
      m_pt[i][s].ex = HVCtrX + HoleFocX * pp[0] / pp[1];
      m_pt[i][s].ey = HVCtrY - HoleFocY * pp[2] / pp[1];
      if (gSettings.dCoarse)
        m_pt[i][s].c1 = 0.25f + 0.75f * m_hole[p][s & (HoleNbParImg - 1)].c1;
      m_pt[i][s].u = s * (1.0f / HoleNbParImg);
      m_pt[i][s].v = txtv;
    }
  }
}

void CScreensaverHufoTunnel::OnCompiledAndLinked()
{
  // Variables passed directly to the Vertex shader
  m_modelViewProjectionMatrixLoc = glGetUniformLocation(ProgramHandle(), "u_modelViewProjectionMatrix");
  m_unformColorLoc = glGetUniformLocation(ProgramHandle(), "u_color");
  m_unformColorUsedLoc = glGetUniformLocation(ProgramHandle(), "u_colorUsed");
  m_textureUsedLoc = glGetUniformLocation(ProgramHandle(), "u_textureUsed");

  m_positionLoc = glGetAttribLocation(ProgramHandle(), "a_position");
  m_colorLoc = glGetAttribLocation(ProgramHandle(), "a_color");
  m_texCoord0Loc = glGetAttribLocation(ProgramHandle(), "a_texCoord0");
}

bool CScreensaverHufoTunnel::OnEnabled()
{
  // This is called after glUseProgram()
  glUniformMatrix4fv(m_modelViewProjectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(m_modelProjMat));
  glUniform1i(m_unformColorUsedLoc, m_uniformColorUsed);
  glUniform1i(m_textureUsedLoc, gSettings.dTexture && !gSettings.dWireframe);
  glUniform4f(m_unformColorLoc, m_uniformColor.r, m_uniformColor.g, m_uniformColor.b, 1.0f);

  return true;
}

ADDONCREATOR(CScreensaverHufoTunnel);
