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

#include "main.h"

#include <array>
#include <vector>
#include <chrono>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <rsMath/rsMath.h>
#include "Rgbhsl/Rgbhsl.h"

#define WIDTH 200
#define HIGHT 200

namespace
{
// Parameters edited in the dialog box
struct sCycloneSettings
{
  sCycloneSettings()
  {
    setDefaults();
  }

  void Load()
  {
    setDefaults();

    kodi::addon::CheckSettingInt("general.cyclones", dCyclones);
    kodi::addon::CheckSettingInt("general.particles", dParticles);
    kodi::addon::CheckSettingInt("general.size", dSize);
    kodi::addon::CheckSettingInt("general.complexity", dComplexity);
    kodi::addon::CheckSettingInt("general.speed", dSpeed);
    kodi::addon::CheckSettingBoolean("general.stretch", dStretch);
    kodi::addon::CheckSettingBoolean("general.showcurves", dShowCurves);
  }

  void setDefaults()
  {
    dCyclones = 1;
    dParticles = 400;
    dSize = 7;
    dComplexity = 3;
    dSpeed = 10;
    dStretch = true;
    dShowCurves = false;
  }

  int dCyclones;
  int dParticles;
  int dSize;
  int dComplexity;
  int dSpeed;
  bool dStretch;
  bool dShowCurves;
} gCycloneSettings;

//------------------------------------------------------------------------------

// useful factorial function
int factorial(int x)
{
  int returnval = 1;

  if (x == 0)
    return(1);
  else
  {
    do
    {
      returnval *= x;
      x -= 1;
    }
    while(x!=0);
  }
  return(returnval);
}

//------------------------------------------------------------------------------
}

class CCyclone
{
public:
  std::vector<std::array<float, 3>> m_targetxyz;
  std::vector<std::array<float, 3>> m_xyz;
  std::vector<std::array<float, 3>> m_oldxyz;
  float *m_targetWidth;
  float *m_width;
  float *m_oldWidth;
  float m_targethsl[3];
  float m_hsl[3];
  float m_oldhsl[3];
  float **m_xyzChange;
  float **m_widthChange;
  float m_hslChange[2];

  CCyclone();
  ~CCyclone() = default;
  void Update(CScreensaverCyclone* base);

private:
  std::vector<sLight> m_curves;
};

CCyclone::CCyclone()
{
  int i;

  // Initialize position stuff
  m_curves.resize(std::max(gCycloneSettings.dComplexity + 3, 50));

  m_targetxyz.resize(gCycloneSettings.dComplexity+3);
  m_xyz.resize(gCycloneSettings.dComplexity+3);
  m_oldxyz.resize(gCycloneSettings.dComplexity+3);

  m_xyz[gCycloneSettings.dComplexity+2][0] = rsRandf(float(WIDTH*2)) - float(WIDTH);
  m_xyz[gCycloneSettings.dComplexity+2][1] = float(HIGHT);
  m_xyz[gCycloneSettings.dComplexity+2][2] = rsRandf(float(WIDTH*2)) - float(WIDTH);
  m_xyz[gCycloneSettings.dComplexity+1][0] = m_xyz[gCycloneSettings.dComplexity+2][0];
  m_xyz[gCycloneSettings.dComplexity+1][1] = rsRandf(float(HIGHT / 3)) + float(HIGHT / 4);
  m_xyz[gCycloneSettings.dComplexity+1][2] = m_xyz[gCycloneSettings.dComplexity+2][2];
  for (i = gCycloneSettings.dComplexity; i > 1; i--)
  {
    m_xyz[i][0] = m_xyz[i+1][0] + rsRandf(float(WIDTH)) - float(WIDTH / 2);
    m_xyz[i][1] = rsRandf(float(HIGHT * 2)) - float(HIGHT);
    m_xyz[i][2] = m_xyz[i+1][2] + rsRandf(float(WIDTH)) - float(WIDTH / 2);
  }
  m_xyz[1][0] = m_xyz[2][0] + rsRandf(float(WIDTH / 2)) - float(WIDTH / 4);
  m_xyz[1][1] = -rsRandf(float(HIGHT / 2)) - float(HIGHT / 4);
  m_xyz[1][2] = m_xyz[2][2] + rsRandf(float(WIDTH / 2)) - float(WIDTH / 4);
  m_xyz[0][0] = m_xyz[1][0] + rsRandf(float(WIDTH / 8)) - float(WIDTH / 16);
  m_xyz[0][1] = float(-HIGHT);
  m_xyz[0][2] = m_xyz[1][2] + rsRandf(float(WIDTH / 8)) - float(WIDTH / 16);
  // Initialize width stuff
  m_targetWidth = new float[gCycloneSettings.dComplexity+3];
  m_width = new float[gCycloneSettings.dComplexity+3];
  m_oldWidth = new float[gCycloneSettings.dComplexity+3];
  m_width[gCycloneSettings.dComplexity+2] = rsRandf(175.0f) + 75.0f;
  m_width[gCycloneSettings.dComplexity+1] = rsRandf(60.0f) + 15.0f;
  for (i = gCycloneSettings.dComplexity; i > 1; i--)
    m_width[i] = rsRandf(25.0f) + 15.0f;
  m_width[1] = rsRandf(25.0f) + 5.0f;
  m_width[0] = rsRandf(15.0f) + 5.0f;
  // Initialize transition stuff
  m_xyzChange = new float*[gCycloneSettings.dComplexity + 3];
  m_widthChange = new float*[gCycloneSettings.dComplexity + 3];
  for (i = 0; i < (gCycloneSettings.dComplexity+3); i++)
  {
    m_xyzChange[i] = new float[2];  // 0 = step   1 = total steps
    m_widthChange[i] = new float[2];
    m_xyzChange[i][0] = 0.0f;
    m_xyzChange[i][1] = 0.0f;
    m_widthChange[i][0] = 0.0f;
    m_widthChange[i][1] = 0.0f;
  }
  // Initialize color stuff
  m_hsl[0] = m_oldhsl[0] = rsRandf(1.0f);
  m_hsl[1] = m_oldhsl[1] = rsRandf(1.0f);
  m_hsl[2] = m_oldhsl[2] = 0.0f;  // start out dark
  m_targethsl[0] = rsRandf(1.0f);
  m_targethsl[1] = rsRandf(1.0f);
  m_targethsl[2] = 1.0f;
  m_hslChange[0] = 0.0f;
  m_hslChange[1] = 10.0f;
}

void CCyclone::Update(CScreensaverCyclone* base)
{
  int i;
  int temp;
  float between;
  float diff;
  int direction;
  glm::vec3 point;
  float step;
  float blend;

  // update cyclone's path
  temp = gCycloneSettings.dComplexity + 2;
  if (m_xyzChange[temp][0] >= m_xyzChange[temp][1])
  {
    m_oldxyz[temp][0] = m_xyz[temp][0];
    m_oldxyz[temp][1] = m_xyz[temp][1];
    m_oldxyz[temp][2] = m_xyz[temp][2];
    m_targetxyz[temp][0] = rsRandf(float(WIDTH*2)) - float(WIDTH);
    m_targetxyz[temp][1] = float(HIGHT);
    m_targetxyz[temp][2] = rsRandf(float(WIDTH*2)) - float(WIDTH);
    m_xyzChange[temp][0] = 0.0f;
    m_xyzChange[temp][1] = rsRandf(150.0f / float(gCycloneSettings.dSpeed)) + 75.0f / float(gCycloneSettings.dSpeed);
  }
  temp = gCycloneSettings.dComplexity + 1;
  if (m_xyzChange[temp][0] >= m_xyzChange[temp][1])
  {
    m_oldxyz[temp][0] = m_xyz[temp][0];
    m_oldxyz[temp][1] = m_xyz[temp][1];
    m_oldxyz[temp][2] = m_xyz[temp][2];
    m_targetxyz[temp][0] = m_xyz[temp+1][0];
    m_targetxyz[temp][1] = rsRandf(float(HIGHT / 3)) + float(HIGHT / 4);
    m_targetxyz[temp][2] = m_xyz[temp+1][2];
    m_xyzChange[temp][0] = 0.0f;
    m_xyzChange[temp][1] = rsRandf(100.0f / float(gCycloneSettings.dSpeed)) + 75.0f / float(gCycloneSettings.dSpeed);
  }
  for (i = gCycloneSettings.dComplexity; i > 1; i--)
  {
    if (m_xyzChange[i][0] >= m_xyzChange[i][1])
    {
      m_oldxyz[i][0] = m_xyz[i][0];
      m_oldxyz[i][1] = m_xyz[i][1];
      m_oldxyz[i][2] = m_xyz[i][2];
      m_targetxyz[i][0] = m_targetxyz[i+1][0] + (m_targetxyz[i+1][0] - m_targetxyz[i+2][0]) / 2.0f + rsRandf(float(WIDTH / 2)) - float(WIDTH / 4);
      m_targetxyz[i][1] = (m_targetxyz[i+1][1] + m_targetxyz[i-1][1]) / 2.0f + rsRandf(float(HIGHT / 8)) - float(HIGHT / 16);
      m_targetxyz[i][2] = m_targetxyz[i+1][2] + (m_targetxyz[i+1][2] - m_targetxyz[i+2][2]) / 2.0f + rsRandf(float(WIDTH / 2)) - float(WIDTH / 4);
      if (m_targetxyz[i][1] > HIGHT)
        m_targetxyz[i][1] = HIGHT;
      if (m_targetxyz[i][1] < -HIGHT)
        m_targetxyz[i][1] = -HIGHT;
      m_xyzChange[i][0] = 0.0f;
      m_xyzChange[i][1] = rsRandf(75.0f / float(gCycloneSettings.dSpeed)) + 50.0f / float(gCycloneSettings.dSpeed);
    }
  }
  if (m_xyzChange[1][0] >= m_xyzChange[1][1])
  {
    m_oldxyz[1][0] = m_xyz[1][0];
    m_oldxyz[1][1] = m_xyz[1][1];
    m_oldxyz[1][2] = m_xyz[1][2];
    m_targetxyz[1][0] = m_targetxyz[2][0] + rsRandf(float(WIDTH / 2)) - float(WIDTH / 4);
    m_targetxyz[1][1] = -rsRandf(float(HIGHT / 2)) - float(HIGHT / 4);
    m_targetxyz[1][2] = m_targetxyz[2][2] + rsRandf(float(WIDTH / 2)) - float(WIDTH / 4);
    m_xyzChange[1][0] = 0.0f;
    m_xyzChange[1][1] = rsRandf(50.0f / float(gCycloneSettings.dSpeed)) + 30.0f / float(gCycloneSettings.dSpeed);
  }
  if (m_xyzChange[0][0] >= m_xyzChange[0][1])
  {
    m_oldxyz[0][0] = m_xyz[0][0];
    m_oldxyz[0][1] = m_xyz[0][1];
    m_oldxyz[0][2] = m_xyz[0][2];
    m_targetxyz[0][0] = m_xyz[1][0] + rsRandf(float(WIDTH / 8)) - float(WIDTH / 16);
    m_targetxyz[0][1] = float(-HIGHT);
    m_targetxyz[0][2] = m_xyz[1][2] + rsRandf(float(WIDTH / 8)) - float(WIDTH / 16);
    m_xyzChange[0][0] = 0.0f;
    m_xyzChange[0][1] = rsRandf(100.0f / float(gCycloneSettings.dSpeed)) + 75.0f / float(gCycloneSettings.dSpeed);
  }
  for (i = 0; i < (gCycloneSettings.dComplexity+3); i++)
  {
    between = m_xyzChange[i][0] / m_xyzChange[i][1] * (2 * glm::pi<float>());
    between = (1.0f - float(cos(between))) / 2.0f;
    m_xyz[i][0] = ((m_targetxyz[i][0] - m_oldxyz[i][0]) * between) + m_oldxyz[i][0];
    m_xyz[i][1] = ((m_targetxyz[i][1] - m_oldxyz[i][1]) * between) + m_oldxyz[i][1];
    m_xyz[i][2] = ((m_targetxyz[i][2] - m_oldxyz[i][2]) * between) + m_oldxyz[i][2];
    m_xyzChange[i][0] += base->FrameTime();
  }

  // Update cyclone's widths
  temp = gCycloneSettings.dComplexity + 2;
  if (m_widthChange[temp][0] >= m_widthChange[temp][1])
  {
    m_oldWidth[temp] = m_width[temp];
    m_targetWidth[temp] = rsRandf(225.0f) + 75.0f;
    m_widthChange[temp][0] = 0.0f;
    m_widthChange[temp][1] = rsRandf(50.0f / float(gCycloneSettings.dSpeed)) + 50.0f / float(gCycloneSettings.dSpeed);
  }
  temp = gCycloneSettings.dComplexity + 1;
  if (m_widthChange[temp][0] >= m_widthChange[temp][1])
  {
    m_oldWidth[temp] = m_width[temp];
    m_targetWidth[temp] = rsRandf(100.0f) + 15.0f;
    m_widthChange[temp][0] = 0.0f;
    m_widthChange[temp][1] = rsRandf(50.0f / float(gCycloneSettings.dSpeed)) + 50.0f / float(gCycloneSettings.dSpeed);
  }
  for (i = gCycloneSettings.dComplexity; i > 1; i--)
  {
    if (m_widthChange[i][0] >= m_widthChange[i][1])
    {
      m_oldWidth[i] = m_width[i];
      m_targetWidth[i] = rsRandf(50.0f) + 15.0f;
      m_widthChange[i][0] = 0.0f;
      m_widthChange[i][1] = rsRandf(50.0f / float(gCycloneSettings.dSpeed)) + 40.0f / float(gCycloneSettings.dSpeed);
    }
  }
  if (m_widthChange[1][0] >= m_widthChange[1][1])
  {
    m_oldWidth[1] = m_width[1];
    m_targetWidth[1] = rsRandf(40.0f) + 5.0f;
    m_widthChange[1][0] = 0.0f;
    m_widthChange[1][1] = rsRandf(50.0f / float(gCycloneSettings.dSpeed)) + 30.0f / float(gCycloneSettings.dSpeed);
  }
  if (m_widthChange[0][0] >= m_widthChange[0][1])
  {
    m_oldWidth[0] = m_width[0];
    m_targetWidth[0] = rsRandf(30.0f) + 5.0f;
    m_widthChange[0][0] = 0.0f;
    m_widthChange[0][1] = rsRandf(50.0f / float(gCycloneSettings.dSpeed)) + 20.0f / float(gCycloneSettings.dSpeed);
  }
  for (i = 0; i < (gCycloneSettings.dComplexity+3); i++)
  {
    between = m_widthChange[i][0] / m_widthChange[i][1];
    m_width[i] = ((m_targetWidth[i] - m_oldWidth[i]) * between) + m_oldWidth[i];
    m_widthChange[i][0] += base->FrameTime();
  }

  // Update cyclones color
  if (m_hslChange[0] >= m_hslChange[1])
  {
    m_oldhsl[0] = m_hsl[0];
    m_oldhsl[1] = m_hsl[1];
    m_oldhsl[2] = m_hsl[2];
    m_targethsl[0] = rsRandf(1.0f);
    m_targethsl[1] = rsRandf(1.0f);
    m_targethsl[2] = rsRandf(1.0f) + 0.5f;
    if (m_targethsl[2] > 1.0f)
      m_targethsl[2] = 1.0f;
    m_hslChange[0] = 0.0f;
    m_hslChange[1] = rsRandf(30.0f) + 2.0f;
  }
  between = m_hslChange[0] / m_hslChange[1];
  diff = m_targethsl[0] - m_oldhsl[0];
  direction = 0;
  if ((m_targethsl[0] > m_oldhsl[0] && diff > 0.5f) || (m_targethsl[0] < m_oldhsl[0] && diff < -0.5f))
    if (diff > 0.5f)
      direction = 1;
  hslTween(m_oldhsl[0], m_oldhsl[1], m_oldhsl[2],
           m_targethsl[0], m_targethsl[1], m_targethsl[2], between, direction,
           m_hsl[0], m_hsl[1], m_hsl[2]);
  m_hslChange[0] += base->FrameTime();

  if (gCycloneSettings.dShowCurves)
  {
    unsigned int ptr = 0;
    base->m_lightingEnabled = 0;
    for (step=0.0; step<1.0; step+=0.02f)
    {
      point = glm::vec3(0.0f);
      for (i = 0; i < (gCycloneSettings.dComplexity+3); i++)
      {
        blend = base->m_fact[gCycloneSettings.dComplexity+2] / (base->m_fact[i]
              * base->m_fact[gCycloneSettings.dComplexity+2-i]) * powf(step, float(i))
              * powf((1.0f - step), float(gCycloneSettings.dComplexity+2-i));
        point.x += m_xyz[i][0] * blend;
        point.y += m_xyz[i][1] * blend;
        point.z += m_xyz[i][2] * blend;
      }
      m_curves[ptr  ].color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
      m_curves[ptr++].vertex = point;
    }
    base->DrawEntry(GL_LINE_STRIP, m_curves.data(), ptr);
    ptr = 0;

    for (i = 0; i < (gCycloneSettings.dComplexity+3); i++)
    {
      m_curves[ptr  ].color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
      m_curves[ptr++].vertex = glm::vec3(m_xyz[i][0], m_xyz[i][1], m_xyz[i][2]);
    }
    base->DrawEntry(GL_LINE_STRIP, m_curves.data(), ptr);
    base->m_lightingEnabled = 1;
  }
}

//------------------------------------------------------------------------------

class CParticle
{
public:
  float m_r, m_g, m_b;
  glm::vec3 m_xyz, m_lastxyz;
  float m_width;
  float m_step;
  float m_spinAngle;
  CCyclone *m_cy;

  CParticle(CCyclone *);
  virtual ~CParticle(){};
  void Init();
  void Update(CScreensaverCyclone* base);
};

CParticle::CParticle(CCyclone *c) : m_cy(c)
{
  Init();
}

void CParticle::Init()
{
  m_width = rsRandf(0.8f) + 0.2f;
  m_step = 0.0f;
  m_spinAngle = rsRandf(360);
  hsl2rgb(m_cy->m_hsl[0], m_cy->m_hsl[1],m_cy->m_hsl[2], m_r, m_g, m_b);
}

void CParticle::Update(CScreensaverCyclone* base)
{
  int i;
  float scale, temp;
  float newStep;
  float newSpinAngle;
  float cyWidth;
  float between;
  glm::vec3 dir;

  float tiltAngle;
  float blend;

  m_lastxyz = m_xyz;
  if (m_step > 1.0f)
    Init();
  m_xyz = glm::vec3(0.0f);
  for (i = 0; i < (gCycloneSettings.dComplexity+3); i++)
  {
    blend = base->m_fact[gCycloneSettings.dComplexity+2] / (base->m_fact[i]
      * base->m_fact[gCycloneSettings.dComplexity+2-i]) * powf(m_step, float(i))
      * powf((1.0f - m_step), float(gCycloneSettings.dComplexity+2-i));
    m_xyz.x += m_cy->m_xyz[i][0] * blend;
    m_xyz.y += m_cy->m_xyz[i][1] * blend;
    m_xyz.z += m_cy->m_xyz[i][2] * blend;
  }
  dir = glm::vec3(0.0f);
  for (i = 0; i < (gCycloneSettings.dComplexity+3); i++)
  {
    blend = base->m_fact[gCycloneSettings.dComplexity+2] / (base->m_fact[i]
          * base->m_fact[gCycloneSettings.dComplexity+2-i]) * powf(m_step - 0.01f, float(i))
          * powf((1.0f - (m_step - 0.01f)), float(gCycloneSettings.dComplexity+2-i));
    dir.x += m_cy->m_xyz[i][0] * blend;
    dir.y += m_cy->m_xyz[i][1] * blend;
    dir.z += m_cy->m_xyz[i][2] * blend;
  }
  dir = m_xyz - dir;

  glm::vec3 up = {0.0f, 1.0f, 0.0f};
  glm::vec3 ret = glm::normalize(glm::vec3(dir[0], dir[1], dir[2]));
  glm::vec3 crossVec = glm::cross(ret, up);
  tiltAngle = -acosf(glm::dot(ret, up)) * 180.0f / glm::pi<float>();
  i = int(m_step * (float(gCycloneSettings.dComplexity) + 2.0f));
  if (i >= (gCycloneSettings.dComplexity + 2))
    i = gCycloneSettings.dComplexity + 1;
  between = (m_step - (float(i) / float(gCycloneSettings.dComplexity + 2))) * float(gCycloneSettings.dComplexity + 2);
  cyWidth = m_cy->m_width[i] * (1.0f - between) + m_cy->m_width[i+1] * (between);
  newStep = (0.2f * base->FrameTime() * float(gCycloneSettings.dSpeed)) / (m_width * m_width * cyWidth);
  m_step += newStep;
  newSpinAngle = (1500.0f * base->FrameTime() * float(gCycloneSettings.dSpeed)) / (m_width * cyWidth);
  m_spinAngle += newSpinAngle;
  if (gCycloneSettings.dStretch)
  {
    scale = m_width * cyWidth * newSpinAngle * 0.02f;
    temp = cyWidth * 2.0f / float(gCycloneSettings.dSize);
    if (scale > temp)
      scale = temp;
    if (scale < 3.0f)
      scale = 3.0f;
  }

  glm::mat4 modelMat = base->m_modelMat;
  base->m_modelMat = glm::translate(glm::mat4(1.0f), m_xyz);
  base->m_modelMat = glm::rotate(base->m_modelMat, glm::radians(tiltAngle), crossVec);
  base->m_modelMat = glm::rotate(base->m_modelMat, glm::radians(m_spinAngle), glm::vec3(0.0f, 1.0f, 0.0f));
  base->m_modelMat = glm::translate(base->m_modelMat, glm::vec3(m_width * cyWidth, 0.0f, 0.0f));
  if (gCycloneSettings.dStretch)
    base->m_modelMat = glm::scale(base->m_modelMat, glm::vec3(1.0f, 1.0f, scale));

  base->DrawSphere(glm::vec4(m_r, m_g, m_b, 1.0f));

  base->m_modelMat = modelMat;
}

//------------------------------------------------------------------------------

bool CScreensaverCyclone::Start()
{
  int i, j;

  std::string fraqShader = kodi::addon::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/frag.glsl");
  std::string vertShader = kodi::addon::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
    return false;

  gCycloneSettings.Load();

  srand((unsigned)time(nullptr));

  // Window initialization
  glViewport(X(), Y(), Width(), Height());

  glEnable(GL_DEPTH_TEST);
  glFrontFace(GL_CCW);
  glEnable(GL_CULL_FACE);
  glClearColor(0.0, 0.0, 0.0, 1.0);

  m_modelMat = glm::mat4(1.0f);
  m_projMat = glm::perspective(glm::radians(80.0f), (GLfloat)Height() / (GLfloat)Width(), 50.0f, 3000.0f);
  if (!rsRandi(500))  // Easter egg view
  {
    m_projMat = glm::rotate(m_projMat, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m_projMat = glm::translate(m_projMat, glm::vec3(0.0f, -(WIDTH * 2), 0.0f));
  }
  else  // Normal view
    m_projMat = glm::translate(m_projMat, glm::vec3(0.0f, 0.0f, -(WIDTH * 2)));

  Sphere(float(gCycloneSettings.dSize) / 4.0f, 3, 2);
  m_lightingEnabled = 1;

  // Initialize cyclones and their particles
  for (i = 0; i < 13; i++)
    m_fact[i] = float(factorial(i));
  m_cyclones = new CCyclone*[gCycloneSettings.dCyclones];
  m_particles = new CParticle*[gCycloneSettings.dParticles * gCycloneSettings.dCyclones];
  for (i = 0; i < gCycloneSettings.dCyclones; i++)
  {
    m_cyclones[i] = new CCyclone;
    for (j=i*gCycloneSettings.dParticles; j<((i+1)*gCycloneSettings.dParticles); j++)
      m_particles[j] = new CParticle(m_cyclones[i]);
  }

  glGenBuffers(1, &m_vertexVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);

  m_lastTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  m_startOK = true;
  return true;
}

void CScreensaverCyclone::Stop()
{
  if (!m_startOK)
    return;

  m_startOK = false;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &m_vertexVBO);
  m_vertexVBO = 0;

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  // Free memory
  delete[] m_particles;
  delete[] m_cyclones;
}

void CScreensaverCyclone::Render()
{
  int i, j;

  if (!m_startOK)
    return;

  /*
   * Following Extra work done here in render to prevent problems with controls
   * from Kodi and during window moving.
   * TODO: Maybe add a separate interface call to inform about?
   */
  //@{
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
  glVertexAttribPointer(m_hVertex, 3, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, vertex)));
  glEnableVertexAttribArray(m_hVertex);

  glVertexAttribPointer(m_hNormal, 3, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, normal)));
  glEnableVertexAttribArray(m_hNormal);

  glVertexAttribPointer(m_hColor, 4, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, color)));
  glEnableVertexAttribArray(m_hColor);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  //@}

  double currentTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  m_frameTime = static_cast<float>(currentTime - m_lastTime);
  m_lastTime = currentTime;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  for (i = 0; i < gCycloneSettings.dCyclones; i++)
  {
    m_cyclones[i]->Update(this);
    for (j=(i * gCycloneSettings.dParticles); j<((i+1) * gCycloneSettings.dParticles); j++)
      m_particles[j]->Update(this);
  }

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  glDisableVertexAttribArray(m_hVertex);
  glDisableVertexAttribArray(m_hNormal);
  glDisableVertexAttribArray(m_hColor);
}

void CScreensaverCyclone::DrawEntry(int primitive, const sLight* data, unsigned int size)
{
  m_modelProjMat = m_projMat * m_modelMat;
  EnableShader();
  glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*size, data, GL_DYNAMIC_DRAW);
  glDrawArrays(primitive, 0, size);
  DisableShader();
}

void CScreensaverCyclone::DrawSphere(const glm::vec4& color)
{
  m_uniformColor = color;
  m_normalMat = glm::transpose(glm::inverse(glm::mat3(m_modelMat)));
  m_modelProjMat = m_projMat * m_modelMat;
  EnableShader();
  glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizei>(sizeof(sLight)*m_sphereTriangleFan1.size()),
               &m_sphereTriangleFan1[0], GL_DYNAMIC_DRAW);
  glDrawArrays(GL_TRIANGLE_FAN, 0, static_cast<GLsizei>(m_sphereTriangleFan1.size()));
  glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizei>(sizeof(sLight)*m_sphereTriangleFan2.size()),
               &m_sphereTriangleFan2[0], GL_DYNAMIC_DRAW);
  glDrawArrays(GL_TRIANGLE_FAN, 0, static_cast<GLsizei>(m_sphereTriangleFan2.size()));
  DisableShader();
}

void CScreensaverCyclone::Sphere(GLfloat radius, GLint slices, GLint stacks)
{
/* Make it not a power of two to avoid cache thrashing on the chip */
#define CACHE_SIZE 240

  sLight light;
  GLint i,j;
  GLfloat sinCache1a[CACHE_SIZE];
  GLfloat cosCache1a[CACHE_SIZE];
  GLfloat sinCache2a[CACHE_SIZE];
  GLfloat cosCache2a[CACHE_SIZE];
  GLfloat sinCache1b[CACHE_SIZE];
  GLfloat cosCache1b[CACHE_SIZE];
  GLfloat sinCache2b[CACHE_SIZE];
  GLfloat cosCache2b[CACHE_SIZE];
  GLfloat angle;
  GLfloat zHigh;
  GLfloat sintemp1 = 0.0, sintemp2 = 0.0;
  GLfloat costemp3 = 0.0;

  for (i = 0; i < slices; i++)
  {
    angle = 2 * glm::pi<float>() * i / slices;
    sinCache1a[i] = sinf(angle);
    cosCache1a[i] = cosf(angle);
    sinCache2a[i] = sinCache1a[i];
    cosCache2a[i] = cosCache1a[i];
  }

  for (j = 0; j <= stacks; j++)
  {
    angle = glm::pi<float>() * j / stacks;
    sinCache2b[j] = sinf(angle);
    cosCache2b[j] = cosf(angle);
    sinCache1b[j] = radius * sinf(angle);
    cosCache1b[j] = radius * cosf(angle);
  }
  /* Make sure it comes to a point */
  sinCache1b[0] = 0;
  sinCache1b[stacks] = 0;

  sinCache1a[slices] = sinCache1a[0];
  cosCache1a[slices] = cosCache1a[0];
  sinCache2a[slices] = sinCache2a[0];
  cosCache2a[slices] = cosCache2a[0];

  /* Low end first (j == 0 iteration) */
  sintemp1 = sinCache1b[1];
  zHigh = cosCache1b[1];
  sintemp2 = sinCache2b[1];
  costemp3 = cosCache2b[1];

  light.normal = glm::vec3(sinCache2a[0] * sinCache2b[0], cosCache2a[0] * sinCache2b[0], cosCache2b[0]);
  light.vertex = glm::vec3(0.0f, 0.0f, radius);
  m_sphereTriangleFan1.push_back(light);
  for (i = slices; i >= 0; i--)
  {
    light.normal = glm::vec3(sinCache2a[i] * sintemp2, cosCache2a[i] * sintemp2, costemp3);
    light.vertex = glm::vec3(sintemp1 * sinCache1a[i], sintemp1 * cosCache1a[i], zHigh);
    m_sphereTriangleFan1.push_back(light);
  }

  /* High end next (j == stacks-1 iteration) */
  sintemp1 = sinCache1b[stacks-1];
  zHigh = cosCache1b[stacks-1];
  sintemp2 = sinCache2b[stacks-1];
  costemp3 = cosCache2b[stacks-1];

  light.normal = glm::vec3(sinCache2a[stacks] * sinCache2b[stacks], cosCache2a[stacks] * sinCache2b[stacks], cosCache2b[stacks]);
  light.vertex = glm::vec3(0.0f, 0.0f, -radius);
  m_sphereTriangleFan2.push_back(light);
  for (i = 0; i <= slices; i++)
  {
    light.normal = glm::vec3(sinCache2a[i] * sintemp2, cosCache2a[i] * sintemp2, costemp3);
    light.vertex = glm::vec3(sintemp1 * sinCache1a[i], sintemp1 * cosCache1a[i], zHigh);
    m_sphereTriangleFan2.push_back(light);
  }
}

void CScreensaverCyclone::OnCompiledAndLinked()
{
  // Variables passed directly to the Vertex shader
  m_projMatLoc = glGetUniformLocation(ProgramHandle(), "u_projectionMatrix");
  m_modelViewMatLoc = glGetUniformLocation(ProgramHandle(), "u_modelViewMatrix");
  m_modelViewProjectionMatrixLoc = glGetUniformLocation(ProgramHandle(), "u_modelViewProjectionMatrix");
  m_transposeAdjointModelViewMatrixLoc = glGetUniformLocation(ProgramHandle(), "u_transposeAdjointModelViewMatrix");
  m_lightingLoc = glGetUniformLocation(ProgramHandle(), "u_lighting");
  m_uniformColorLoc = glGetUniformLocation(ProgramHandle(), "u_uniformColor");
  m_light0_ambientLoc = glGetUniformLocation(ProgramHandle(), "u_light0.ambient");
  m_light0_diffuseLoc = glGetUniformLocation(ProgramHandle(), "u_light0.diffuse");
  m_light0_specularLoc = glGetUniformLocation(ProgramHandle(), "u_light0.specular");
  m_light0_positionLoc = glGetUniformLocation(ProgramHandle(), "u_light0.position");
  m_light0_constantAttenuationLoc = glGetUniformLocation(ProgramHandle(), "u_light0.constantAttenuation");
  m_light0_linearAttenuationLoc = glGetUniformLocation(ProgramHandle(), "u_light0.linearAttenuation");
  m_light0_quadraticAttenuationLoc = glGetUniformLocation(ProgramHandle(), "u_light0.quadraticAttenuation");
  m_light0_spotDirectionLoc = glGetUniformLocation(ProgramHandle(), "u_light0.spotDirection");
  m_light0_spotExponentLoc = glGetUniformLocation(ProgramHandle(), "u_light0.spotExponent");
  m_light0_spotCutoffAngleCosLoc = glGetUniformLocation(ProgramHandle(), "u_light0.spotCutoffAngleCos");
  m_material_ambientLoc = glGetUniformLocation(ProgramHandle(), "u_material.ambient");
  m_material_diffuseLoc = glGetUniformLocation(ProgramHandle(), "u_material.diffuse");
  m_material_specularLoc = glGetUniformLocation(ProgramHandle(), "u_material.specular");
  m_material_emissionLoc = glGetUniformLocation(ProgramHandle(), "u_material.emission");
  m_material_shininessLoc = glGetUniformLocation(ProgramHandle(), "u_material.shininess");

  m_hVertex = glGetAttribLocation(ProgramHandle(), "a_position");
  m_hNormal = glGetAttribLocation(ProgramHandle(), "a_normal");
  m_hColor = glGetAttribLocation(ProgramHandle(), "a_color");
}

bool CScreensaverCyclone::OnEnabled()
{
  // This is called after glUseProgram()

  float ambient[4] = {0.25f, 0.25f, 0.25f, 0.0f};
  float diffuse[4] = {1.0f, 1.0f, 1.0f, 0.0f};
  float specular[4] = {1.0f, 1.0f, 1.0f, 0.0f};
  float position[4] = {float(WIDTH * 2), -float(HIGHT), float(WIDTH * 2), 0.0f};

  glUniformMatrix4fv(m_projMatLoc, 1, GL_FALSE, glm::value_ptr(m_projMat));
  glUniformMatrix4fv(m_modelViewMatLoc, 1, GL_FALSE, glm::value_ptr(m_modelMat));
  glUniformMatrix4fv(m_modelViewProjectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(m_modelProjMat));
  glUniformMatrix3fv(m_transposeAdjointModelViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(m_normalMat));
  glUniform1i(m_lightingLoc, m_lightingEnabled);
  glUniform4f(m_uniformColorLoc, m_uniformColor.r, m_uniformColor.g, m_uniformColor.b, m_uniformColor.a);

  glUniform4f(m_light0_ambientLoc, ambient[0], ambient[1], ambient[2], ambient[3]);
  glUniform4f(m_light0_diffuseLoc, diffuse[0], diffuse[1], diffuse[2], diffuse[3]);
  glUniform4f(m_light0_specularLoc, specular[0], specular[1], specular[2], specular[3]);
  glUniform4f(m_light0_positionLoc, position[0], position[1], position[2], position[3]);
  glUniform1f(m_light0_constantAttenuationLoc, 1.0f);
  glUniform1f(m_light0_linearAttenuationLoc, 0.0f);
  glUniform1f(m_light0_quadraticAttenuationLoc, 0.0f);
  glUniform3f(m_light0_spotDirectionLoc, 0.0f, 0.0f, -1.0f);
  glUniform1f(m_light0_spotExponentLoc, 0.0f);
  glUniform1f(m_light0_spotCutoffAngleCosLoc, -1.0f);

  glUniform4f(m_material_ambientLoc, 0.2f, 0.2f, 0.2f, 1.0f);
  glUniform4f(m_material_diffuseLoc, 0.8f, 0.8f, 0.8f, 1.0f);
  glUniform4f(m_material_specularLoc, 0.0f, 0.0f, 0.0f, 1.0f);
  glUniform4f(m_material_emissionLoc, 0.0f, 0.0f, 0.0f, 1.0f);
  glUniform1f(m_material_shininessLoc, 20.0f);

  return true;
}

ADDONCREATOR(CScreensaverCyclone);
