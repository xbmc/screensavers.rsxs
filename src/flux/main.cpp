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

#include <kodi/gui/General.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <Rgbhsl/Rgbhsl.h>
#include <rsMath/rsMath.h>

// Override GL_RED if not present with GL_LUMINANCE, e.g. on Android GLES
#ifndef GL_RED
#define GL_RED GL_LUMINANCE
#endif

// Parameters edited in the dialog box

#define PRESET_AUTO_SELECTION 0
#define PRESET_REGULAR 1
#define PRESET_HYPNOTIC 2
#define PRESET_INSANE 3
#define PRESET_SPARKLERS 4
#define PRESET_PARADIGM 5
#define PRESET_GALACTIC 6
#define PRESET_ADVANCED_SETTINGS -1

#define GEOMETRY_POINTS 0
#define GEOMETRY_SPHERES 1
#define GEOMETRY_LIGHTS 2

namespace {

int gWhichparticle;

struct sFluxSettings
{
  sFluxSettings()
  {
    SetDefaults(PRESET_REGULAR);
  }

  void Load()
  {
    int type = PRESET_REGULAR;
    kodi::addon::CheckSettingInt("general.type", type);
    if (type == PRESET_AUTO_SELECTION)
      SetDefaults(rsRandi(6) + 1);
    else
      SetDefaults(type);

    if (type != PRESET_ADVANCED_SETTINGS &&
        type != kodi::addon::GetSettingInt("general.lastType"))
    {
      kodi::addon::SetSettingInt("general.lastType", type);

      kodi::addon::SetSettingInt("advanced.fluxes", dFluxes);
      kodi::addon::SetSettingInt("advanced.particles", dParticles);
      kodi::addon::SetSettingInt("advanced.trail", dTrail);
      kodi::addon::SetSettingInt("advanced.geometry", dGeometry);
      kodi::addon::SetSettingInt("advanced.size", dSize);
      kodi::addon::SetSettingInt("advanced.complexity", dRandomize);
      kodi::addon::SetSettingInt("advanced.randomize", dExpansion);
      kodi::addon::SetSettingInt("advanced.expansion", dRotation);
      kodi::addon::SetSettingInt("advanced.rotation", dWind);
      kodi::addon::SetSettingInt("advanced.instability", dInstability);
      kodi::addon::SetSettingInt("advanced.blur", dBlur);
    }
  }

  void SetDefaults(int preset)
  {
    switch (preset)
    {
    case PRESET_REGULAR:  // Regular
      dFluxes = 1;
      dParticles = 20;
      dTrail = 40;
      dGeometry = GEOMETRY_LIGHTS;
      dSize = 15;
      dComplexity = 3;
      dRandomize = 0;
      dExpansion = 40;
      dRotation = 30;
      dWind = 20;
      dInstability = 20;
      dBlur = 0;
      break;
    case PRESET_HYPNOTIC:  // Hypnotic
      dFluxes = 2;
      dParticles = 10;
      dTrail = 40;
      dGeometry = GEOMETRY_LIGHTS;
      dSize = 15;
      dRandomize = 80;
      dExpansion = 20;
      dRotation = 0;
      dWind = 40;
      dInstability = 10;
      dBlur = 30;
      break;
    case PRESET_INSANE:  // Insane
      dFluxes = 4;
      dParticles = 30;
      dTrail = 8;
      dGeometry = GEOMETRY_LIGHTS;
      dSize = 25;
      dRandomize = 0;
      dExpansion = 80;
      dRotation = 60;
      dWind = 40;
      dInstability = 100;
      dBlur = 10;
      break;
    case PRESET_SPARKLERS:  // Sparklers
      dFluxes = 3;
      dParticles = 20;
      dTrail = 6;
      dGeometry = GEOMETRY_SPHERES;
      dSize = 20;
      dComplexity = 3;
      dRandomize = 85;
      dExpansion = 60;
      dRotation = 30;
      dWind = 20;
      dInstability = 30;
      dBlur = 0;
      break;
    case PRESET_PARADIGM:  // Paradigm
      dFluxes = 1;
      dParticles = 40;
      dTrail = 40;
      dGeometry = GEOMETRY_LIGHTS;
      dSize = 5;
      dRandomize = 90;
      dExpansion = 30;
      dRotation = 20;
      dWind = 10;
      dInstability = 5;
      dBlur = 10;
      break;
    case PRESET_GALACTIC:  // Galactic
      dFluxes = 3;
      dParticles = 2;
      dTrail = 1500;
      dGeometry = GEOMETRY_LIGHTS;
      dSize = 10;
      dRandomize = 0;
      dExpansion = 5;
      dRotation = 25;
      dWind = 0;
      dInstability = 5;
      dBlur = 0;
      break;
    case PRESET_ADVANCED_SETTINGS:  // Galactic
      dFluxes = kodi::addon::GetSettingInt("advanced.fluxes");
      dParticles = kodi::addon::GetSettingInt("advanced.particles");
      dTrail = kodi::addon::GetSettingInt("advanced.trail");
      dGeometry = kodi::addon::GetSettingInt("advanced.geometry");
      dSize = kodi::addon::GetSettingInt("advanced.size");
      dRandomize = kodi::addon::GetSettingInt("advanced.complexity");
      dExpansion = kodi::addon::GetSettingInt("advanced.randomize");
      dRotation = kodi::addon::GetSettingInt("advanced.expansion");
      dWind = kodi::addon::GetSettingInt("advanced.rotation");
      dInstability = kodi::addon::GetSettingInt("advanced.instability");
      dBlur = kodi::addon::GetSettingInt("advanced.blur");
    }
  }

  int dFluxes;
  int dParticles;
  int dTrail;
  int dGeometry;
  int dSize;
  int dComplexity;
  int dRandomize;
  int dExpansion;
  int dRotation;
  int dWind;
  int dInstability;
  int dBlur;
} gSettings;
}

//------------------------------------------------------------------------------


// This class is poorly named.  It's actually a whole trail of particles.
class CParticle
{
public:
  CParticle();
  ~CParticle() = default;
  void update(float *c, CScreensaverFlux* base);

private:
  std::vector<std::array<float, 5>> m_vertices;
  int m_counter;
  float m_offset[3];

  float m_expander = 1.0f + 0.0005f * float(gSettings.dExpansion);
  float m_blower = 0.001f * float(gSettings.dWind);
  float m_otherxyz[3];
};

CParticle::CParticle()
{
  // Offsets are somewhat like default positions for the head of each
  // particle trail.  Offsets spread out the particle trails and keep
  // them from all overlapping.
  m_offset[0] = cosf(2 * glm::pi<float>() * float(gWhichparticle) / float(gSettings.dParticles));
  m_offset[1] = float(gWhichparticle) / float(gSettings.dParticles) - 0.5f;
  m_offset[2] = sinf(2 * glm::pi<float>() * float(gWhichparticle) / float(gSettings.dParticles));
  gWhichparticle++;

  // Initialize memory and set initial positions out of view of the camera
  m_vertices.resize(gSettings.dTrail);
  for (auto& vertex : m_vertices)
  {
    vertex[0] = 0.0f;
    vertex[1] = 3.0f;
    vertex[2] = 0.0f;
    vertex[3] = 0.0f;
    vertex[4] = 0.0f;
  }

  m_counter = 0;
}

void CParticle::update(float *c, CScreensaverFlux* base)
{
  int i, p, growth;
  float rgb[3];
  float cx, cy, cz;  // Containment variables
  float luminosity;
  float depth;

  // Record old position
  int oldc = m_counter;

  m_counter ++;
  if (m_counter >= gSettings.dTrail)
    m_counter = 0;

  // Here's the iterative math for calculating new vertex positions
  // first calculate limiting terms which keep vertices from constantly
  // flying off to infinity
  cx = m_vertices[oldc][0] * (1.0f - 1.0f / (m_vertices[oldc][0] * m_vertices[oldc][0] + 1.0f));
  cy = m_vertices[oldc][1] * (1.0f - 1.0f / (m_vertices[oldc][1] * m_vertices[oldc][1] + 1.0f));
  cz = m_vertices[oldc][2] * (1.0f - 1.0f / (m_vertices[oldc][2] * m_vertices[oldc][2] + 1.0f));
  // then calculate new positions
  m_vertices[m_counter][0] = m_vertices[oldc][0] + c[6] * m_offset[0] - cx
    + c[2] * m_vertices[oldc][1]
    + c[5] * m_vertices[oldc][2];
  m_vertices[m_counter][1] = m_vertices[oldc][1] + c[6] * m_offset[1] - cy
    + c[1] * m_vertices[oldc][2]
    + c[4] * m_vertices[oldc][0];
  m_vertices[m_counter][2] = m_vertices[oldc][2] + c[6] * m_offset[2] - cz
    + c[0] * m_vertices[oldc][0]
    + c[3] * m_vertices[oldc][1];

  // calculate "orbitiness" of particles
  const float xdiff(m_vertices[m_counter][0] - m_vertices[oldc][0]);
  const float ydiff(m_vertices[m_counter][1] - m_vertices[oldc][1]);
  const float zdiff(m_vertices[m_counter][2] - m_vertices[oldc][2]);
  const float distsq(m_vertices[m_counter][0] * m_vertices[m_counter][0]
                   + m_vertices[m_counter][1] * m_vertices[m_counter][1]
                   + m_vertices[m_counter][2] * m_vertices[m_counter][2]);
  const float oldDistsq(m_vertices[oldc][0] * m_vertices[oldc][0]
                      + m_vertices[oldc][1] * m_vertices[oldc][1]
                      + m_vertices[oldc][2] * m_vertices[oldc][2]);
  base->m_orbitiness += (xdiff * xdiff + ydiff * ydiff + zdiff * zdiff)
                          / (2.0f - fabs(distsq - oldDistsq));

  // Pick a hue
  m_vertices[m_counter][3] = cx * cx + cy * cy + cz * cz;
  if (m_vertices[m_counter][3] > 1.0f)
    m_vertices[m_counter][3] = 1.0f;
  m_vertices[m_counter][3] += c[7];
  // Limit the hue (0 - 1)
  if (m_vertices[m_counter][3] > 1.0f)
    m_vertices[m_counter][3] -= 1.0f;
  if (m_vertices[m_counter][3] < 0.0f)
    m_vertices[m_counter][3] += 1.0f;
  // Pick a saturation
  m_vertices[m_counter][4] = c[0] + m_vertices[m_counter][3];
  // Limit the saturation (0 - 1)
  if (m_vertices[m_counter][4] < 0.0f)
    m_vertices[m_counter][4] = -m_vertices[m_counter][4];
  m_vertices[m_counter][4] -= float(int(m_vertices[m_counter][4]));
  m_vertices[m_counter][4] = 1.0f - (m_vertices[m_counter][4] * m_vertices[m_counter][4]);

  // Bring particles back if they escape
  if (!m_counter)
  {
    if (m_vertices[m_counter][0] * m_vertices[m_counter][0]
      + m_vertices[m_counter][1] * m_vertices[m_counter][1]
      + m_vertices[m_counter][2] * m_vertices[m_counter][2] > 100000000.0f)
    {
      m_vertices[m_counter][0] = rsRandf(2.0f) - 1.0f;
      m_vertices[m_counter][1] = rsRandf(2.0f) - 1.0f;
      m_vertices[m_counter][2] = rsRandf(2.0f) - 1.0f;
    }
  }

  // Draw every vertex in particle trail
  p = m_counter;
  growth = 0;
  luminosity = base->m_lumdiff;
  for (i = 0; i < gSettings.dTrail; i++)
  {
    p ++;
    if (p >= gSettings.dTrail)
      p = 0;
    growth++;

    if (m_vertices[p][0] * m_vertices[p][0]
      + m_vertices[p][1] * m_vertices[p][1]
      + m_vertices[p][2] * m_vertices[p][2] < 40000.0f)
    {
      // assign color to particle
      hsl2rgb(m_vertices[p][3], m_vertices[p][4], luminosity, rgb[0], rgb[1], rgb[2]);
      base->m_uniformColor = glm::vec4(rgb[0], rgb[1], rgb[2], 1.0f);

      glm::mat4 modelMatrix = base->m_modelMat;

      switch(gSettings.dGeometry)
      {
        case GEOMETRY_POINTS:  // Points
        {
          depth = base->m_cosCameraAngle * m_vertices[p][2] - base->m_sinCameraAngle * m_vertices[p][0];
          base->m_modelMat = glm::translate(base->m_modelMat, glm::vec3(base->m_cosCameraAngle * m_vertices[p][0] +
                                                                        base->m_sinCameraAngle * m_vertices[p][2], m_vertices[p][1], depth));
#if !defined(HAS_GLES)
          switch(gSettings.dTrail - growth)
          {
          case 0:
            glPointSize(float(gSettings.dSize * (depth + 200.0f) * 0.001036f));
            break;
          case 1:
            glPointSize(float(gSettings.dSize * (depth + 200.0f) * 0.002f));
            break;
          case 2:
            glPointSize(float(gSettings.dSize * (depth + 200.0f) * 0.002828f));
            break;
          case 3:
            glPointSize(float(gSettings.dSize * (depth + 200.0f) * 0.003464f));
            break;
          case 4:
            glPointSize(float(gSettings.dSize * (depth + 200.0f) * 0.003864f));
            break;
          default:
            glPointSize(float(gSettings.dSize * (depth + 200.0f) * 0.004f));
          }
#endif
          base->DrawPoint();
          break;
        }
        case GEOMETRY_SPHERES:  // Spheres
        {
          base->m_modelMat = glm::translate(base->m_modelMat, glm::vec3(m_vertices[p][0], m_vertices[p][1], m_vertices[p][2]));
          switch(gSettings.dTrail - growth)
          {
          case 0:
            base->m_modelMat = glm::scale(base->m_modelMat, glm::vec3(0.259f, 0.259f, 0.259f));
            break;
          case 1:
            base->m_modelMat = glm::scale(base->m_modelMat, glm::vec3(0.5, 0.5, 0.5));
            break;
          case 2:
            base->m_modelMat = glm::scale(base->m_modelMat, glm::vec3(0.707f, 0.707f, 0.707f));
            break;
          case 3:
            base->m_modelMat = glm::scale(base->m_modelMat, glm::vec3(0.866f, 0.866f, 0.866f));
            break;
          case 4:
            base->m_modelMat = glm::scale(base->m_modelMat, glm::vec3(0.966f, 0.966f, 0.966f));
          }
          base->DrawSphere();
          break;
        }
        case GEOMETRY_LIGHTS:  // Lights
        {
          depth = base->m_cosCameraAngle * m_vertices[p][2] - base->m_sinCameraAngle * m_vertices[p][0];
          base->m_modelMat = glm::translate(base->m_modelMat, glm::vec3(base->m_cosCameraAngle * m_vertices[p][0] +
                                                                        base->m_sinCameraAngle * m_vertices[p][2], m_vertices[p][1], depth));
          switch(gSettings.dTrail - growth)
          {
          case 0:
            base->m_modelMat = glm::scale(base->m_modelMat, glm::vec3(0.259f, 0.259f, 0.259f));
            break;
          case 1:
            base->m_modelMat = glm::scale(base->m_modelMat, glm::vec3(0.5, 0.5, 0.5));
            break;
          case 2:
            base->m_modelMat = glm::scale(base->m_modelMat, glm::vec3(0.707f, 0.707f, 0.707f));
            break;
          case 3:
            base->m_modelMat = glm::scale(base->m_modelMat, glm::vec3(0.866f, 0.866f, 0.866f));
            break;
          case 4:
            base->m_modelMat = glm::scale(base->m_modelMat, glm::vec3(0.966f, 0.966f, 0.966f));
          }
          base->DrawTriangles();
        }
      }
      base->m_modelMat = modelMatrix;
    }

    m_vertices[p][0] *= m_expander;
    m_vertices[p][1] *= m_expander;
    m_vertices[p][2] *= m_expander;
    m_vertices[p][2] += m_blower;
    luminosity += base->m_lumdiff;
  }
}

//------------------------------------------------------------------------------

// This class is a set of particle trails and constants that enter
// into their equations of motion.
class CFlux
{
public:
  CFlux();
  ~CFlux() = default;
  void update(CScreensaverFlux* base);

private:
  std::vector<CParticle> m_particles;
  int m_randomize;
  float m_c[NUMCONSTS];     // constants
  float m_cv[NUMCONSTS];    // constants' change velocities
};

CFlux::CFlux()
{
  int i;

  gWhichparticle = 0;

  m_particles.resize(gSettings.dParticles);
  m_randomize = 1;
  for (i = 0; i < NUMCONSTS; i++)
  {
    m_c[i] = rsRandf(2.0f) - 1.0f;
    m_cv[i] = rsRandf(0.000005f * float(gSettings.dInstability) * float(gSettings.dInstability))
                    + 0.000001f * float(gSettings.dInstability) * float(gSettings.dInstability);
  }
}

void CFlux::update(CScreensaverFlux* base)
{
  // randomize constants
  if (gSettings.dRandomize)
  {
    m_randomize --;
    if (m_randomize <= 0)
    {
      for (int i = 0; i < NUMCONSTS; i++)
        m_c[i] = rsRandf(2.0f) - 1.0f;
      int temp = 101 - gSettings.dRandomize;
      temp = temp * temp;
      m_randomize = temp + rsRandi(temp);
    }
  }

  // update constants
  for (int i = 0; i < NUMCONSTS; i++)
  {
    m_c[i] += m_cv[i];
    if (m_c[i] >= 1.0f){
      m_c[i] = 1.0f;
      m_cv[i] = -m_cv[i];
    }
    if (m_c[i] <= -1.0f){
      m_c[i] = -1.0f;
      m_cv[i] = -m_cv[i];
    }
  }

  base->m_prevOrbitiness = base->m_orbitiness;
  base->m_orbitiness = 0.0f;

  // update all particles in this flux field
  for (int i = 0; i < gSettings.dParticles; i++)
    m_particles[i].update(m_c, base);

  if (base->m_orbitiness < base->m_prevOrbitiness)
  {
    int i = rsRandi(NUMCONSTS - 1);
    m_cv[i] = -m_cv[i];
  }
}

//------------------------------------------------------------------------------

bool CScreensaverFlux::Start()
{
  srand((unsigned)time(nullptr));

  gSettings.Load();

  std::string fraqShader = kodi::addon::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/frag.glsl");
  std::string vertShader = kodi::addon::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
    return false;

  glViewport(X(), Y(), Width(), Height());
  m_projMat = glm::perspective(glm::radians(100.0f), float(Width()) / float(Height()), 0.01f, 200.0f);
  m_modelMat = glm::mat4(1.0f);

  glFrontFace(GL_CCW);
  glEnable(GL_CULL_FACE);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (gSettings.dGeometry == GEOMETRY_SPHERES)  // Spheres and their lighting
  {
    Sphere(0.005f * float(gSettings.dSize), gSettings.dComplexity + 2, gSettings.dComplexity + 1);
    m_lightingEnabled = 1;
  }
  else
    m_lightingEnabled = 0;

  if (gSettings.dGeometry == GEOMETRY_POINTS ||
      gSettings.dGeometry == GEOMETRY_LIGHTS)  // Init lights or points
  {
    m_textureUsed = 1;

    float x, y, temp;
    for (int i = 0; i < LIGHTSIZE; i++)
    {
      for (int j = 0; j < LIGHTSIZE; j++)
      {
        x = float(i - LIGHTSIZE / 2) / float(LIGHTSIZE / 2);
        y = float(j - LIGHTSIZE / 2) / float(LIGHTSIZE / 2);
        temp = 1.0f - float(sqrt((x * x) + (y * y)));
        if (temp > 1.0f)
          temp = 1.0f;
        if (temp < 0.0f)
          temp = 0.0f;
        m_lightTexture[i][j] = static_cast<unsigned char>(255.0f * temp * temp);
      }
    }
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, LIGHTSIZE, LIGHTSIZE, 0, GL_RED, GL_UNSIGNED_BYTE, m_lightTexture);

    temp = float(gSettings.dSize) * 0.005f;
    if (gSettings.dGeometry == GEOMETRY_POINTS)
    {
      m_textureTriangles[0].vertex = glm::vec3(-temp, -temp, 0.0f);
      m_textureTriangles[0].coord = glm::vec2(0.0f, 0.0f);
      m_textureTriangles[1].vertex = glm::vec3(temp, -temp, 0.0f);
      m_textureTriangles[1].coord = glm::vec2(1.0f, 0.0f);
      m_textureTriangles[2].vertex = glm::vec3(-temp, temp, 0.0f);
      m_textureTriangles[2].coord = glm::vec2(0.0f, 1.0f);
      m_textureTriangles[3].vertex = glm::vec3(temp, temp, 0.0f);
      m_textureTriangles[3].coord = glm::vec2(1.0f, 1.0f);
    }
    else
    {
      m_textureTriangles[0].coord = glm::vec2(0.0f, 0.0f);
      m_textureTriangles[0].vertex = glm::vec3(-temp, -temp, 0.0f);
      m_textureTriangles[1].coord = glm::vec2(1.0f, 0.0f);
      m_textureTriangles[1].vertex = glm::vec3(temp, -temp, 0.0f);
      m_textureTriangles[2].coord = glm::vec2(1.0f, 1.0f);
      m_textureTriangles[2].vertex = glm::vec3(temp, temp, 0.0f);
      m_textureTriangles[3].coord = glm::vec2(0.0f, 0.0f);
      m_textureTriangles[3].vertex = glm::vec3(-temp, -temp, 0.0f);
      m_textureTriangles[4].coord = glm::vec2(1.0f, 1.0f);
      m_textureTriangles[4].vertex = glm::vec3(temp, temp, 0.0f);
      m_textureTriangles[5].coord = glm::vec2(0.0f, 1.0f);
      m_textureTriangles[5].vertex = glm::vec3(-temp, temp, 0.0f);
    }
  }
  else
    m_textureUsed = 0;

  m_blur[0].vertex = glm::vec3(0.0f, 0.0f, 0.0f);
  m_blur[1].vertex = glm::vec3(1.0f, 0.0f, 0.0f);
  m_blur[2].vertex = glm::vec3(0.0f, 1.0f, 0.0f);
  m_blur[3].vertex = glm::vec3(1.0f, 1.0f, 0.0f);

  // Initialize luminosity difference
  m_lumdiff = 1.0f / float(gSettings.dTrail);

  // Initialize flux fields
  m_fluxes.resize(gSettings.dFluxes);

  glGenBuffers(1, &m_vertexVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);

  m_cameraAngle = 0.0f;
  m_startOK = true;
  m_startClearCnt = 5;
  return true;
}

void CScreensaverFlux::Stop()
{
  if (!m_startOK)
    return;
  m_startOK = false;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &m_vertexVBO);
  m_vertexVBO = 0;

  if (gSettings.dGeometry == GEOMETRY_POINTS ||
      gSettings.dGeometry == GEOMETRY_LIGHTS)
  {
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &m_texture);
    m_texture = 0;
  }

  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#if defined(HAS_GL) || (defined(HAS_GLES) && HAS_GLES == 3)
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
}

void CScreensaverFlux::Render()
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

  glVertexAttribPointer(m_hCoord, 2, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, coord)));
  glEnableVertexAttribArray(m_hCoord);

  glEnable(GL_CULL_FACE);
  glBindTexture(GL_TEXTURE_2D, m_texture);

  if (m_startClearCnt)
  {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_startClearCnt--;
  }
  //@}

  // clear the screen
  if (gSettings.dBlur)  // partially
  {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    m_uniformColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f - (float(sqrtf(sqrtf(float(gSettings.dBlur)))) * 0.15495f));
    m_modelProjMat = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f) * glm::mat4(1.0f);
    EnableShader();
    glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*4, m_blur, GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    DisableShader();
  }
  else  // completely
    glClear(GL_COLOR_BUFFER_BIT);

  m_modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.5f));

  // Rotate camera
  m_cameraAngle += 0.01f * float(gSettings.dRotation);
  if (m_cameraAngle >= 360.0f)
    m_cameraAngle -= 360.0f;
  if (gSettings.dGeometry == GEOMETRY_SPHERES)  // Only rotate for spheres
    m_modelMat = glm::rotate(m_modelMat, glm::radians(m_cameraAngle), glm::vec3(0.0f, 1.0f, 0.0f));
  else
  {
    m_cosCameraAngle = cosf(m_cameraAngle * RS_DEG2RAD);
    m_sinCameraAngle = sinf(m_cameraAngle * RS_DEG2RAD);
  }

  // set up state for rendering particles
  switch (gSettings.dGeometry)
  {
  case GEOMETRY_POINTS:  // Blending for points
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_BLEND);
    m_textureUsed = 1;
    break;
  case GEOMETRY_SPHERES:  // No blending for spheres, but we need z-buffering
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    break;
  case GEOMETRY_LIGHTS:  // Blending for lights
    glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_BLEND);
    m_textureUsed = 1;
  }

  // Update particles
  for (int i = 0; i < gSettings.dFluxes; i++)
    m_fluxes[i].update(this);

  glDisable(GL_CULL_FACE);
  glDisableVertexAttribArray(m_hCoord);
  glDisableVertexAttribArray(m_hVertex);
  glDisableVertexAttribArray(m_hNormal);
}

void CScreensaverFlux::DrawPoint()
{
  m_modelProjMat = m_projMat * m_modelMat;
  m_normalMat = glm::transpose(glm::inverse(glm::mat3(m_modelMat)));
  EnableShader();
  glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*4, &m_textureTriangles[0], GL_DYNAMIC_DRAW);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  DisableShader();
}

void CScreensaverFlux::DrawTriangles()
{
  m_modelProjMat = m_projMat * m_modelMat;
  m_normalMat = glm::transpose(glm::inverse(glm::mat3(m_modelMat)));
  EnableShader();
  glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*6, &m_textureTriangles[0], GL_DYNAMIC_DRAW);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  DisableShader();
}

void CScreensaverFlux::DrawSphere()
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

void CScreensaverFlux::Sphere(GLfloat radius, GLint slices, GLint stacks)
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
  light.vertex = glm::vec3(0.0, 0.0, radius);
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
  light.vertex = glm::vec3(0.0, 0.0, -radius);
  m_sphereTriangleFan2.push_back(light);
  for (i = 0; i <= slices; i++)
  {
    light.normal = glm::vec3(sinCache2a[i] * sintemp2, cosCache2a[i] * sintemp2, costemp3);
    light.vertex = glm::vec3(sintemp1 * sinCache1a[i], sintemp1 * cosCache1a[i], zHigh);
    m_sphereTriangleFan2.push_back(light);
  }
}

void CScreensaverFlux::OnCompiledAndLinked()
{
  // Variables passed directly to the Vertex shader
  m_projMatLoc = glGetUniformLocation(ProgramHandle(), "u_projectionMatrix");
  m_modelViewMatLoc = glGetUniformLocation(ProgramHandle(), "u_modelViewMatrix");
  m_modelViewProjectionMatrixLoc = glGetUniformLocation(ProgramHandle(), "u_modelViewProjectionMatrix");
  m_transposeAdjointModelViewMatrixLoc = glGetUniformLocation(ProgramHandle(), "u_transposeAdjointModelViewMatrix");
  m_textureUsedLoc = glGetUniformLocation(ProgramHandle(), "u_textureUsed");
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

  m_hNormal = glGetAttribLocation(ProgramHandle(), "a_normal");
  m_hVertex = glGetAttribLocation(ProgramHandle(), "a_position");
  m_hCoord = glGetAttribLocation(ProgramHandle(), "a_coord");
}

bool CScreensaverFlux::OnEnabled()
{
  // This is called after glUseProgram()
  float ambient[4]  = { 0.0f,   0.0f,   0.0f,   0.0f };
  float diffuse[4]  = { 1.0f,   1.0f,   1.0f,   0.0f };
  float specular[4] = { 1.0f,   1.0f,   1.0f,   0.0f };
  float position[4] = { 500.0f, 500.0f, 500.0f, 0.0f };

  glUniformMatrix4fv(m_projMatLoc, 1, GL_FALSE, glm::value_ptr(m_projMat));
  glUniformMatrix4fv(m_modelViewMatLoc, 1, GL_FALSE, glm::value_ptr(m_modelMat));
  glUniformMatrix4fv(m_modelViewProjectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(m_modelProjMat));
  glUniformMatrix3fv(m_transposeAdjointModelViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(m_normalMat));
  glUniform1i(m_textureUsedLoc, m_textureUsed);
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
  glUniform1f(m_material_shininessLoc, 0.0f);

  return true;
}

ADDONCREATOR(CScreensaverFlux);
