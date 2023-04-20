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

#include <chrono>
#include <kodi/gui/General.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <Rgbhsl/Rgbhsl.h>
#include <rsMath/rsMath.h>

// Parameters edited in the dialog box

#define PRESET_AUTO_SELECTION 0
#define PRESET_NORMAL 1
#define PRESET_TRAILS 2
#define PRESET_BLURRED_TRAILS 3
#define PRESET_BLURRED_CONNECTIONS 4
#define PRESET_CIRCLES 5
#define PRESET_CIRCLES_RANDOM 6
#define PRESET_ADVANCED_SETTINGS -1

namespace {
struct sFlocksSettings
{
  sFlocksSettings()
  {
    setDefaults(PRESET_NORMAL);
  }

  void Load()
  {
    int type = PRESET_NORMAL;
    kodi::addon::CheckSettingInt("general.type", type);
    if (type == PRESET_AUTO_SELECTION)
      setDefaults(rsRandi(6) + 1);
    else
      setDefaults(type);

    if (type != PRESET_ADVANCED_SETTINGS &&
        type != kodi::addon::GetSettingInt("general.lastType"))
    {
      kodi::addon::SetSettingInt("general.lastType", type);

      kodi::addon::SetSettingInt("advanced.leaders", dLeaders);
      kodi::addon::SetSettingInt("advanced.followers", dFollowers);
      if (dGeometry && !dCircles)
        kodi::addon::SetSettingInt("advanced.geometry", 0);
      else if (!dGeometry && dCircles)
        kodi::addon::SetSettingInt("advanced.geometry", 1);
      else
        kodi::addon::SetSettingInt("advanced.geometry", 2);
      kodi::addon::SetSettingInt("advanced.size", dSize);
      kodi::addon::SetSettingInt("advanced.complexity", dComplexity);
      kodi::addon::SetSettingInt("advanced.speed", dSpeed);
      kodi::addon::SetSettingInt("advanced.stretch", dStretch);
      kodi::addon::SetSettingInt("advanced.colorfadespeed", dColorfadespeed);
      kodi::addon::SetSettingBoolean("advanced.chromatek", dChromatek);
      kodi::addon::SetSettingBoolean("advanced.connections", dConnections);
      kodi::addon::SetSettingInt("advanced.traillength", dTrail);
      kodi::addon::SetSettingInt("advanced.blur", dBlur);
      kodi::addon::SetSettingBoolean("advanced.randomcolors", dRandomColors);
      kodi::addon::SetSettingBoolean("advanced.clearcompletely", dClear);
    }
  }

  void setDefaults(int preset)
  {
    dClear = true;
    dCircles = false;
    dRandomColors = false;

    switch (preset)
    {
      case PRESET_NORMAL:
        dLeaders = 4;
        dFollowers = 400;
        dGeometry = true;
        dSize = 10;
        dComplexity = 1;
        dSpeed = 15;
        dStretch = 20;
        dColorfadespeed = 15;
        dChromatek = false;
        dConnections = false;
        dTrail = 0;
        dBlur = 0;
        break;
      case PRESET_TRAILS:
        dLeaders = 4;
        dFollowers = 400;
        dGeometry = true;
        dSize = 10;
        dComplexity = 1;
        dSpeed = 15;
        dStretch = 1;
        dColorfadespeed = 15;
        dChromatek = false;
        dConnections = false;
        dTrail = 100;
        dBlur = 0;
        break;
      case PRESET_BLURRED_TRAILS:
        dLeaders = 4;
        dFollowers = 200;
        dGeometry = false;
        dSize = 1;
        dComplexity = 1;
        dSpeed = 15;
        dStretch = 0;
        dColorfadespeed = 15;
        dChromatek = false;
        dConnections = false;
        dTrail = 100;
        dBlur = 100;
        break;
      case PRESET_BLURRED_CONNECTIONS:
        dLeaders = 8;
        dFollowers = 400;
        dGeometry = true;
        dSize = 10;
        dComplexity = 1;
        dSpeed = 15;
        dStretch = 20;
        dColorfadespeed = 15;
        dChromatek = false;
        dConnections = true;
        dTrail = 0;
        dBlur = 100;
        break;
      case PRESET_CIRCLES:
        dLeaders = 8;
        dFollowers = 400;
        dGeometry = false;
        dSize = 10;
        dComplexity = 1;
        dSpeed = 15;
        dStretch = 20;
        dColorfadespeed = 15;
        dChromatek = false;
        dConnections = false;
        dTrail = 0;
        dBlur = 0;
        dClear = false;
        dCircles = true;
        dRandomColors = false;
        break;
      case PRESET_CIRCLES_RANDOM:
        dLeaders = 8;
        dFollowers = 400;
        dGeometry = false;
        dSize = 10;
        dComplexity = 1;
        dSpeed = 15;
        dStretch = 20;
        dColorfadespeed = 15;
        dChromatek = false;
        dConnections = false;
        dTrail = 0;
        dBlur = 0;
        dClear = false;
        dCircles = true;
        dRandomColors = true;
        break;
      case PRESET_ADVANCED_SETTINGS:
      {
        dLeaders = kodi::addon::GetSettingInt("advanced.leaders");
        dFollowers = kodi::addon::GetSettingInt("advanced.followers");

        switch (kodi::addon::GetSettingInt("advanced.geometry"))
        {
          case 0:
            dGeometry = true;
            dCircles = false;
            break;
          case 1:
            dGeometry = false;
            dCircles = true;
            break;
          case 2:
            dGeometry = false;
            dCircles = false;
            break;
        }

        dSize = kodi::addon::GetSettingInt("advanced.size");
        dComplexity = kodi::addon::GetSettingInt("advanced.complexity");
        dSpeed = kodi::addon::GetSettingInt("advanced.speed");
        dStretch = kodi::addon::GetSettingInt("advanced.stretch");
        dColorfadespeed = kodi::addon::GetSettingInt("advanced.colorfadespeed");
        dChromatek = kodi::addon::GetSettingBoolean("advanced.chromatek");
        dConnections = kodi::addon::GetSettingBoolean("advanced.connections");
        dTrail = kodi::addon::GetSettingInt("advanced.traillength");
        dBlur = kodi::addon::GetSettingInt("advanced.blur");
        dRandomColors = kodi::addon::GetSettingBoolean("advanced.randomcolors");
        dClear = kodi::addon::GetSettingBoolean("advanced.clearcompletely");
      }
    }
  }

  int dLeaders;
  int dFollowers;
  bool dGeometry;
  int dSize;
  int dComplexity;
  int dSpeed;
  int dStretch;
  int dColorfadespeed;
  bool dChromatek;
  bool dConnections;
  int dBlur;
  int dTrail;
  int dClear;
  bool dCircles;
  int dRandomColors;
} gSettings;
}

//------------------------------------------------------------------------------

class CBug
{
public:
  CBug();
  ~CBug();

  void initTrail();
  void initLeader(int width, int height, int depth);
  void initFollower(int width, int height, int depth);
  void update(CBug* bugs, float colorFade, float elapsedTime);
  void render(CBug* bugs, CScreensaverFlocks* base);

private:
  int m_width;
  int m_height;
  int m_depth;
  int type;    // 0 = leader 1 = follower
  float h, s, l;
  float r, g, b;
  float halfr, halfg, halfb;
  float x, y, z;
  float xSpeed, ySpeed, zSpeed, maxSpeed;
  float accel;
  int right, up, forward;
  int leader;
  float craziness;  // How prone to switching direction is this leader
  float nextChange;  // Time until this leader's next direction change
  int hcount;

  int skipTrail;
  int trailEndPtr;

  std::vector<float> xtrail;
  std::vector<float> ytrail;
  std::vector<float> ztrail;

  std::vector<float> rtrail;
  std::vector<float> gtrail;
  std::vector<float> btrail;

  float xdrift;
  float ydrift;
  float zdrift;

  sLight* m_trailLight = nullptr;
};

CBug::CBug()
{
  hcount = rand();
}

CBug::~CBug()
{
  delete[] m_trailLight;

}

void CBug::initTrail()
{
  trailEndPtr = 0;
  skipTrail = 0;

  xtrail.resize(gSettings.dTrail);
  ytrail.resize(gSettings.dTrail);
  ztrail.resize(gSettings.dTrail);
  rtrail.resize(gSettings.dTrail);
  gtrail.resize(gSettings.dTrail);
  btrail.resize(gSettings.dTrail);
  m_trailLight = new sLight[gSettings.dTrail];

  for (int i = 0; i < gSettings.dTrail; i++)
  {
    xtrail[i] = x;
    ytrail[i] = y;
    ztrail[i] = z;

    rtrail[i] = 0;
    gtrail[i] = 0;
    btrail[i] = 0;
  }
}

void CBug::initLeader(int width, int height, int depth)
{
  m_width = width;
  m_height = height;;
  m_depth = depth;;

  type = 0;
  h = rsRandf(1.0f);
  s = 1.0f;
  l = 1.0f;
  x = rsRandf(float (m_width * 2)) - float (m_width);
  y = rsRandf(float (m_height * 2)) - float (m_height);
  z = rsRandf(float (m_width * 2)) + float (m_width * 2);

  if (gSettings.dTrail)
  {
    CBug::initTrail();

    xdrift = (rsRandf (2) - 1);
    ydrift = (rsRandf (2) - 1);
    zdrift = (rsRandf (2) - 1);
  }

  right = up = forward = 1;
  xSpeed = ySpeed = zSpeed = 0.0f;
  maxSpeed = 8.0f * float(gSettings.dSpeed);
  accel = 13.0f * float(gSettings.dSpeed);

  craziness = rsRandf(4.0f) + 0.05f;
  nextChange = 1.0f;

  leader = -1;
}

void CBug::initFollower(int width, int height, int depth)
{
  m_width = width;
  m_height = height;;
  m_depth = depth;;

  type = 1;
  h = rsRandf (1.0f);
  s = 1.0f;
  l = 1.0f;
  x = rsRandf (float (width * 2)) - float (width);
  y = rsRandf (float (height * 2)) - float (height);
  z = rsRandf (float (width * 5)) + float (width * 2);

  if (gSettings.dTrail)
  {
    CBug::initTrail();
  }

  right = up = forward = 0;
  xSpeed = ySpeed = zSpeed = 0.0f;
  maxSpeed = (rsRandf (6.0f) + 4.0f) * float (gSettings.dSpeed);
  accel = (rsRandf (4.0f) + 9.0f) * float (gSettings.dSpeed);

  leader = 0;
}

void CBug::update(CBug* bugs, float colorFade, float elapsedTime)
{
  int i;

  if (!type)    // leader
  {
    nextChange -= elapsedTime;

    if (nextChange <= 0.0f)
    {
      if (rsRandi(2))
        right++;
      if (rsRandi(2))
        up++;
      if (rsRandi(2))
        forward++;

      if (right >= 2)
        right = 0;
      if (up >= 2)
        up = 0;
      if (forward >= 2)
        forward = 0;

      nextChange = rsRandf (craziness);
    }

    if (right)
      xSpeed += accel * elapsedTime;
    else
      xSpeed -= accel * elapsedTime;
    if (up)
      ySpeed += accel * elapsedTime;
    else
      ySpeed -= accel * elapsedTime;
    if (forward)
      zSpeed -= accel * elapsedTime;
    else
      zSpeed += accel * elapsedTime;

    if (x < float (-m_width))
      right = 1;
    else if (x > float (m_width))
      right = 0;
    if (y < float (-m_height))
      up = 1;
    else if (y > float (m_height))
      up = 0;
    if (z < float (-m_depth))
      forward = 0;
    else if (z > float (m_depth))
      forward = 1;

    // Even leaders change color from Chromatek 3D
    if (gSettings.dChromatek)
    {
      h = 0.666667f * ((float (m_width) - z)/float (m_width + m_width));
      if (h > 0.666667f)
        h = 0.666667f;
      if (h < 0.0f)
        h = 0.0f;
    }
  }
  else     // follower
  {
    if (!rsRandi(10))
    {
      float oldDistance = 10000000.0f, newDistance;

      for (i = 0; i < gSettings.dLeaders; i++)
      {
        newDistance = ((bugs[i].x - x) * (bugs[i].x - x)
                 + (bugs[i].y - y) * (bugs[i].y - y)
                 + (bugs[i].z - z) * (bugs[i].z - z));
        if (newDistance < oldDistance)
        {
          oldDistance = newDistance;
          leader = i;
        }
      }
    }

    if ((bugs[leader].x - x) > 0.0f)
      xSpeed += accel * elapsedTime;
    else
      xSpeed -= accel * elapsedTime;
    if ((bugs[leader].y - y) > 0.0f)
      ySpeed += accel * elapsedTime;
    else
      ySpeed -= accel * elapsedTime;
    if ((bugs[leader].z - z) > 0.0f)
      zSpeed += accel * elapsedTime;
    else
      zSpeed -= accel * elapsedTime;

    if (gSettings.dChromatek)
    {
      h = 0.666667f * ((float (m_width) - z)/float (m_width + m_width));
      if (h > 0.666667f)
        h = 0.666667f;
      if (h < 0.0f)
        h = 0.0f;
    }
    else
    {
      if (fabs (h - bugs[leader].h) < (colorFade * elapsedTime))
        h = bugs[leader].h;
      else
      {
        if (fabs (h - bugs[leader].h) < 0.5f) {
          if (h > bugs[leader].h)
            h -= colorFade * elapsedTime;
          else
            h += colorFade * elapsedTime;
        }
        else
        {
          if (h > bugs[leader].h)
            h += colorFade * elapsedTime;
          else
            h -= colorFade * elapsedTime;
          if (h > 1.0f)
            h -= 1.0f;
          if (h < 0.0f)
            h += 1.0f;
        }
      }
    }
  }

  if (xSpeed > maxSpeed)
    xSpeed = maxSpeed;
  else if (xSpeed < -maxSpeed)
    xSpeed = -maxSpeed;
  if (ySpeed > maxSpeed)
    ySpeed = maxSpeed;
  else if (ySpeed < -maxSpeed)
    ySpeed = -maxSpeed;
  if (zSpeed > maxSpeed)
    zSpeed = maxSpeed;
  else if (zSpeed < -maxSpeed)
    zSpeed = -maxSpeed;

  x += xSpeed * elapsedTime;
  y += ySpeed * elapsedTime;
  z += zSpeed * elapsedTime;

  ++hcount;
  hcount = hcount % 360;

  hsl2rgb (h, s, l, r, g, b);
  halfr = r * 0.5f;
  halfg = g * 0.5f;
  halfb = b * 0.5f;

  if (gSettings.dTrail)
  {
    float tr, tg, tb;
    hsl2rgb (h, s, l, tr, tg, tb);

    skipTrail++;

    xtrail[trailEndPtr] = x;
    ytrail[trailEndPtr] = y;
    ztrail[trailEndPtr] = z;
    rtrail[trailEndPtr] = tr;
    gtrail[trailEndPtr] = tg;
    btrail[trailEndPtr] = tb;

    trailEndPtr = (trailEndPtr + 1) % gSettings.dTrail;
  }
}

void CBug::render(CBug* bugs, CScreensaverFlocks* base)
{
  int i;
  float scale[4] = { 0.0f };
  sLight light[32];

  base->m_uniformColor = glm::vec4(r, g, b, 1.0f);
  if (gSettings.dGeometry)   // Draw blobs
  {
    glm::mat4 modelMat = base->m_modelMat;
    base->m_modelMat = glm::translate(base->m_modelMat, glm::vec3(x, y, z));

    if (gSettings.dStretch)
    {
      scale[0] = xSpeed * 0.04f;
      scale[1] = ySpeed * 0.04f;
      scale[2] = zSpeed * 0.04f;
      scale[3] = scale[0] * scale[0] + scale[1] * scale[1] + scale[2] * scale[2];

      if (scale[3] > 0.0f)
      {
        scale[3] = float (sqrt (scale[3]));

        scale[0] /= scale[3];
        scale[1] /= scale[3];
        scale[2] /= scale[3];
      }

      scale[3] *= float (gSettings.dStretch) * 0.05f;

      if (scale[3] < 1.0f)
        scale[3] = 1.0f;

      base->m_modelMat = glm::rotate(base->m_modelMat, glm::radians(float(atan2(-scale[0], -scale[2])) * RS_RAD2DEG), glm::vec3(0.0f, 1.0f, 0.0f));
      base->m_modelMat = glm::rotate(base->m_modelMat, glm::radians(float(asin(scale[1])) * RS_RAD2DEG), glm::vec3(1.0f, 0.0f, 0.0f));
      base->m_modelMat = glm::scale(base->m_modelMat, glm::vec3(1.0f, 1.0f, scale[3]));
    }

    base->m_uniformColorUsed = 1;
    base->DrawSphere();
    base->m_uniformColorUsed = 0;
    base->m_modelMat = modelMat;

  }
  else if (gSettings.dCircles)  // Draw circle
  {
    if ((z > 100.0) && (z < 1000.0))
    {
      float rr = r, gg = g, bb = b;

      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_BLEND);
      glDisable(GL_DEPTH_TEST);
      base->m_uniformColorUsed = 1;

      if (gSettings.dRandomColors)
        hsl2rgb(hcount / 360.0f, 1.0f, 1.0f, rr, gg, bb);

      base->m_uniformColor = glm::vec4(rr, gg, bb, 0.1f);
      light[0].vertex = glm::vec3(x, y, 0.0f);
      for (int ii = 0; ii <= 30; ++ii)
        light[ii+1].vertex = glm::vec3(x + cosf(ii / 30.0f * 2 * glm::pi<float>()) * z / 10.0f, y + sinf(ii / 30.0f * 2 * glm::pi<float>()) * z / 10.0f, 0.0f);
      base->DrawEntry(GL_TRIANGLE_FAN, light, 32);

      if (gSettings.dRandomColors)
        hsl2rgb(fmod(hcount / 360.0f + 0.5f, 1.0f), 1.0f, 1.0f, rr, gg, bb);
      else
        hsl2rgb(fmod(h + 0.5f, 1.0f), 1.0f, 1.0f, rr, gg, bb);

      base->m_uniformColor = glm::vec4(rr, gg, bb, 0.5f);
      for (int ii = 0; ii <= 30; ++ii)
        light[ii].vertex = glm::vec3(x + cosf(ii / 30.0f * 2 * glm::pi<float>()) * z / 10.0f, y + sinf(ii / 30.0f * 2 * glm::pi<float>()) * z / 10.0f, 0.0f);
      base->DrawEntry(GL_LINE_STRIP, light, 31);

      base->m_uniformColorUsed = 0;
      glEnable(GL_DEPTH_TEST);
      glDisable(GL_BLEND);
    }
  }
  else    // Draw dots
  {
    if (gSettings.dStretch)
    {
      float size = float (gSettings.dSize) * float (700 - z) * 0.0002f;
      if (size > 0.0f)
      {
        glLineWidth(size);

        scale[0] *= float (gSettings.dStretch);
        scale[1] *= float (gSettings.dStretch);
        scale[2] *= float (gSettings.dStretch);

        base->m_uniformColorUsed = 1;
        light[0].vertex = glm::vec3(x - scale[0], y - scale[1], z - scale[2]);
        light[1].vertex = glm::vec3(x + scale[0], y + scale[1], z + scale[2]);
        base->DrawEntry(GL_LINES, light, 2);
      }
    }
    else
    {
      float size = float (gSettings.dSize) * float (700 - z) * 0.001f;
      if (size > 0.0f)
      {
#if !defined(HAS_GLES)
        glPointSize(size);
#endif
        base->m_uniformColorUsed = 1;
        light[0].vertex = glm::vec3(x, y, z);
        base->DrawEntry(GL_POINTS, light, 1);
      }
    }
  }

  if (gSettings.dConnections && type)   // draw connections
  {
    glLineWidth(1.0f);

    base->m_uniformColorUsed = 0;
    base->m_lightingEnabled = 0;
    light[0].color = glm::vec4(halfr, halfg, halfb, 1.0f);
    light[0].vertex = glm::vec3(x, y, z);
    light[1].color = glm::vec4(bugs[leader].halfr, bugs[leader].halfg, bugs[leader].halfb, 1.0f);
    light[1].vertex = glm::vec3(bugs[leader].x, bugs[leader].y, bugs[leader].z);
    base->DrawEntry(GL_LINES, light, 2);
    base->m_lightingEnabled = gSettings.dGeometry ? 1 : 0;
  }

  if (gSettings.dTrail)
  {
    glLineWidth(3.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

#define ELEMENT(x) x[(trailEndPtr + i) % gSettings.dTrail]
    for (i = 0; i < gSettings.dTrail; i++)
    {
      m_trailLight[i].color = glm::vec4(ELEMENT(rtrail), ELEMENT(gtrail), ELEMENT(btrail), (float)i / gSettings.dTrail);
      m_trailLight[i].vertex = glm::vec3(ELEMENT(xtrail), ELEMENT(ytrail), ELEMENT(ztrail));
    }
    base->m_uniformColorUsed = 0;
    base->m_lightingEnabled = 0;
    base->DrawEntry(GL_LINE_STRIP, m_trailLight, gSettings.dTrail);
    base->m_lightingEnabled = gSettings.dGeometry ? 1 : 0;

    for (i = 0; i < gSettings.dTrail; i++)
    {
      xtrail[i] += bugs[leader].xdrift;
      ytrail[i] += bugs[leader].ydrift;
      ztrail[i] += bugs[leader].zdrift;
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
  }
}

//------------------------------------------------------------------------------

bool CScreensaverFlocks::Start()
{
  gSettings.Load();

  std::string fraqShader = kodi::addon::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/frag.glsl");
  std::string vertShader = kodi::addon::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
    return false;

  glViewport(X(), Y(), Width(), Height());

  // calculate boundaries
  if (Width() > Height())
  {
    m_height = m_depth = 160;
    m_width = m_height * Width() / Height();
  }
  else
  {
    m_width = m_depth = 160;
    m_height = m_width * Height() / Width();
  }

  glEnable(GL_DEPTH_TEST);
  glFrontFace(GL_CCW);
  glEnable(GL_CULL_FACE);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
#if !defined(HAS_GLES)
  glEnable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
#endif

  if (gSettings.dGeometry)  // Setup lights and build blobs
  {
    Sphere(float (gSettings.dSize) * 0.5f, gSettings.dComplexity + 2, gSettings.dComplexity + 1);
    m_lightingEnabled = 1;
  }
  else
  {
    m_lightingEnabled = 0;
  }

  m_projMat = glm::perspective(glm::radians(50.0f), float(Width()) / float(Height()), 0.1f, 2000.0f);
  m_modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -float(m_width * 2)));

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  m_lBugs = new CBug[gSettings.dLeaders];
  m_fBugs = new CBug[gSettings.dFollowers];
  for (int i = 0; i < gSettings.dLeaders; i++)
    m_lBugs[i].initLeader(m_width, m_height, m_depth);
  for (int i = 0; i < gSettings.dFollowers; i++)
    m_fBugs[i].initFollower(m_width, m_height, m_depth);

  glGenBuffers(1, &m_vertexVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
  glGenBuffers(1, &m_indexVBO);

  m_colorFade = float(gSettings.dColorfadespeed) * 0.01f;
  m_lastTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  m_startOK = true;
  m_startClearCnt = 5;

  glGenTextures(1, &m_texture);
  glBindTexture(GL_TEXTURE_2D, m_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width(), Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

  return true;
}

void CScreensaverFlocks::Stop()
{
  if (!m_startOK)
    return;
  m_startOK = false;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &m_vertexVBO);
  m_vertexVBO = 0;
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &m_indexVBO);
  m_indexVBO = 0;
  glDeleteTextures(1, &m_texture);
  m_texture = 0;

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glClearColor (0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#if !defined(HAS_GLES)
  glDisable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
  glReadBuffer(GL_BACK);
  glDrawBuffer(GL_BACK);
#endif
}

void CScreensaverFlocks::Render()
{
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

  glVertexAttribPointer(m_hCoord, 2, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, coord)));
  glEnableVertexAttribArray(m_hCoord);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glBindTexture(GL_TEXTURE_2D, m_texture);

  if (m_startClearCnt)
  {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_startClearCnt--;
  }
  //@}

  double currentTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  float frameTime = static_cast<float>(currentTime - m_lastTime);
  m_lastTime = currentTime;

  int i;
  static float times[10] = { 0.03f, 0.03f, 0.03f, 0.03f, 0.03f,
    0.03f, 0.03f, 0.03f, 0.03f, 0.03f
  };
  static int timeindex = 0;

  if (frameTime > 0)
    times[timeindex] = frameTime;
  else
    times[timeindex] = m_elapsedTime;

  m_elapsedTime = 0.1f * (times[0] + times[1] + times[2] + times[3] + times[4] + times[5] + times[6] + times[7] + times[8] + times[9]);

  if (gSettings.dBlur)     // partially
  {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    m_uniformColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f - (float(sqrt(sqrt(double (gSettings.dBlur * 0.75)))) * 0.15495f));
    m_uniformColorUsed = 1;
    sLight light[4];
    light[0].vertex = glm::vec3(-1000.0f, -1000.0f, 0.0f);
    light[1].vertex = glm::vec3(1000.0f, -1000.0f, 0.0f);
    light[2].vertex = glm::vec3(-1000.0f, 1000.0f, 0.0f);
    light[3].vertex = glm::vec3(1000.0f, 1000.0f, 0.0f);
    DrawEntry(GL_TRIANGLE_STRIP, light, 4);
    m_uniformColorUsed = 0;

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glClear(GL_DEPTH_BUFFER_BIT);
  }
  else if (gSettings.dClear)  // completely
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }
  else
  {
    glm::mat4 projMat = m_projMat;
    glm::mat4 modelMat = m_modelMat;
    m_projMat = glm::mat4(1.0f);
    m_modelMat = glm::mat4(1.0f);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte)*4, m_idx, GL_STATIC_DRAW);
    m_textureUsed = 1;
    glDisable(GL_DEPTH_TEST);

#if !defined(HAS_GLES)
    glReadBuffer(GL_FRONT);
    glDrawBuffer(GL_BACK);
#endif

    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, Width(), Height(), 0);
    EnableShader();
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, 0);
    DisableShader();
    m_textureUsed = 0;

    glEnable(GL_DEPTH_TEST);

    m_modelMat = modelMat;
    m_projMat = projMat;
  }

  // Update and draw leaders
  for (i = 0; i < gSettings.dLeaders; i++)
    m_lBugs[i].update(m_lBugs, m_colorFade, m_elapsedTime);
  // Update and draw followers
  for (i = 0; i < gSettings.dFollowers; i++)
    m_fBugs[i].update(m_lBugs, m_colorFade, m_elapsedTime);

  for (i = 0; i < gSettings.dLeaders; i++)
    m_lBugs[i].render(m_lBugs, this);
  for (i = 0; i < gSettings.dFollowers; i++)
    m_fBugs[i].render(m_lBugs, this);

  glFlush();

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  glDisableVertexAttribArray(m_hCoord);
  glDisableVertexAttribArray(m_hVertex);
  glDisableVertexAttribArray(m_hNormal);
  glDisableVertexAttribArray(m_hColor);
}

void CScreensaverFlocks::DrawEntry(int primitive, const sLight* data, unsigned int size)
{
  m_modelProjMat = m_projMat * m_modelMat;
  m_normalMat = glm::transpose(glm::inverse(glm::mat3(m_modelMat)));
  EnableShader();
  glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*size, data, GL_STATIC_DRAW);
  glDrawArrays(primitive, 0, size);
  DisableShader();
}

void CScreensaverFlocks::DrawSphere()
{
  m_modelProjMat = m_projMat * m_modelMat;
  m_normalMat = glm::transpose(glm::inverse(glm::mat3(m_modelMat)));
  EnableShader();
  glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*m_sphereTriangleFan1.size(), &m_sphereTriangleFan1[0], GL_DYNAMIC_DRAW);
  glDrawArrays(GL_TRIANGLE_FAN, 0, static_cast<GLsizei>(m_sphereTriangleFan1.size()));
  glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*m_sphereTriangleFan2.size(), &m_sphereTriangleFan2[0], GL_DYNAMIC_DRAW);
  glDrawArrays(GL_TRIANGLE_FAN, 0, static_cast<GLsizei>(m_sphereTriangleFan2.size()));
  DisableShader();
}

void CScreensaverFlocks::Sphere(GLfloat radius, GLint slices, GLint stacks)
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
  GLfloat sintemp1 = 0.0f, sintemp2 = 0.0f;
  GLfloat costemp3 = 0.0f;

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

void CScreensaverFlocks::OnCompiledAndLinked()
{
  // Variables passed directly to the Vertex shader
  m_projMatLoc = glGetUniformLocation(ProgramHandle(), "u_projectionMatrix");
  m_modelViewMatLoc = glGetUniformLocation(ProgramHandle(), "u_modelViewMatrix");
  m_modelViewProjectionMatrixLoc = glGetUniformLocation(ProgramHandle(), "u_modelViewProjectionMatrix");
  m_transposeAdjointModelViewMatrixLoc = glGetUniformLocation(ProgramHandle(), "u_transposeAdjointModelViewMatrix");
  m_textureUsedLoc = glGetUniformLocation(ProgramHandle(), "u_textureUsed");
  m_lightingLoc = glGetUniformLocation(ProgramHandle(), "u_lighting");
  m_uniformColorUsedLoc = glGetUniformLocation(ProgramHandle(), "u_uniformColorUsed");
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

  m_hNormal = glGetAttribLocation(ProgramHandle(), "a_normal");
  m_hVertex = glGetAttribLocation(ProgramHandle(), "a_position");
  m_hColor = glGetAttribLocation(ProgramHandle(), "a_color");
  m_hCoord = glGetAttribLocation(ProgramHandle(), "a_coord");
}

bool CScreensaverFlocks::OnEnabled()
{
  // This is called after glUseProgram()

  float ambient[4]  = { 0.25f,  0.25f,  0.25f,  1.0f };
  float diffuse[4]  = { 1.0f,   1.0f,   1.0f,   1.0f };
  float specular[4] = { 1.0f,   1.0f,   1.0f,   1.0f };
  float position[4] = { 500.0f, 500.0f, 500.0f, 1.0f };

  glUniformMatrix4fv(m_projMatLoc, 1, GL_FALSE, glm::value_ptr(m_projMat));
  glUniformMatrix4fv(m_modelViewMatLoc, 1, GL_FALSE, glm::value_ptr(m_modelMat));
  glUniformMatrix4fv(m_modelViewProjectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(m_modelProjMat));
  glUniformMatrix3fv(m_transposeAdjointModelViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(m_normalMat));
  glUniform1i(m_textureUsedLoc, m_textureUsed);
  glUniform1i(m_lightingLoc, m_lightingEnabled);
  glUniform1i(m_uniformColorUsedLoc, m_uniformColorUsed);
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
  glUniform1f(m_material_shininessLoc, 10.0f);

  return true;
}

ADDONCREATOR(CScreensaverFlocks);
