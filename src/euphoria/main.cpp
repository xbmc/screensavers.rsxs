/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2002-2008 Michael Chapman
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
#include "texture.h"

#include <chrono>
#include <math.h>
#include <rsMath/rsMath.h>
#include <Rgbhsl/Rgbhsl.h>
#include <glm/glm.hpp>
#include <kodi/gui/gl/Texture.h>

#define NUMCONSTS 9

// Parameters edited in the dialog box
namespace
{
typedef enum EuphoriaType
{
  PRESET_ADVANCED = -1,
  PRESET_AUTO = 0,
  PRESET_REGULAR = 1,
  PRESET_GRID = 2,
  PRESET_CUBISM = 3,
  PRESET_BADMATH = 4,
  PRESET_MTHEORY = 5,
  PRESET_UHFTEM = 6,
  PRESET_JACK = 7,
  PRESET_OVERDOSE = 8,
  PRESET_NOWHERE = 9,
  PRESET_ECHO = 10,
  PRESET_KALEIDOSCOPE = 11
} EuphoriaType;

typedef enum EuphoriaTexture
{
  TEXTURE_DEFAULT = -1,
  TEXTURE_NONE = 0,
  TEXTURE_PLASMA = 1,
  TEXTURE_STRINGY = 2,
  TEXTURE_LINES = 3,
  TEXTURE_RANDOM = 4,
} EuphoriaTexture;

struct sEuphoriaSettings
{
  void init()
  {
    int mode = PRESET_AUTO;
    kodi::addon::CheckSettingInt("general.type", mode);

    int texture = TEXTURE_DEFAULT;
    kodi::addon::CheckSettingInt("general.texture", texture);

    if (mode == PRESET_AUTO)
    {
      int tries = 1000;
      while (tries--)
      {
        mode = rsRandi(11) + 1;
        if (checkModeEnabled(mode))
          break;
      }

      setDefaults(mode, texture);
    }
    else if (mode == PRESET_ADVANCED)
    {
      dWisps = kodi::addon::GetSettingInt("advanced.wisps");
      dBackground = kodi::addon::GetSettingInt("advanced.bgwisps");
      dDensity = kodi::addon::GetSettingInt("advanced.density");
      dVisibility = kodi::addon::GetSettingInt("advanced.visibility");
      dSpeed = kodi::addon::GetSettingInt("advanced.speed");
      dFeedback = kodi::addon::GetSettingInt("advanced.feedback");
      dFeedbackspeed = kodi::addon::GetSettingInt("advanced.feedbackspeed");
      dFeedbacksize = kodi::addon::GetSettingInt("advanced.feedbacksize");
      dWireframe = kodi::addon::GetSettingBoolean("advanced.wireframe");
      if (texture == TEXTURE_DEFAULT)
        dTexture = TEXTURE_RANDOM;
    }
    else
      setDefaults(mode, texture);
  }

  bool checkModeEnabled(int mode)
  {
    switch (mode)
    {
      case PRESET_REGULAR:
        return kodi::addon::GetSettingBoolean("automodes.PRESET_REGULAR");
      case PRESET_GRID:
        return kodi::addon::GetSettingBoolean("automodes.PRESET_GRID");
      case PRESET_CUBISM:
        return kodi::addon::GetSettingBoolean("automodes.PRESET_CUBISM");
      case PRESET_BADMATH:
        return kodi::addon::GetSettingBoolean("automodes.PRESET_BADMATH");
      case PRESET_MTHEORY:
        return kodi::addon::GetSettingBoolean("automodes.PRESET_MTHEORY");
      case PRESET_UHFTEM:
        return kodi::addon::GetSettingBoolean("automodes.PRESET_UHFTEM");
      case PRESET_JACK:
        return kodi::addon::GetSettingBoolean("automodes.PRESET_JACK");
      case PRESET_OVERDOSE:
        return kodi::addon::GetSettingBoolean("automodes.PRESET_OVERDOSE");
      case PRESET_NOWHERE:
        return kodi::addon::GetSettingBoolean("automodes.PRESET_NOWHERE");
      case PRESET_ECHO:
        return kodi::addon::GetSettingBoolean("automodes.PRESET_ECHO");
      case PRESET_KALEIDOSCOPE:
        return kodi::addon::GetSettingBoolean("automodes.PRESET_KALEIDOSCOPE");
      default:
        return true;
    }
  }

  void setDefaults(int mode, int texture)
  {
    switch(mode)
    {
    case PRESET_REGULAR:  // Regular
      dWisps = 5;
      dBackground = 0;
      dDensity = 35;
      dVisibility = 35;
      dSpeed = 15;
      dFeedback = 0;
      dFeedbackspeed = 1;
      dFeedbacksize = 10;
      dWireframe = false;
      dTexture = texture == TEXTURE_DEFAULT ? TEXTURE_NONE : texture;
      break;
    case PRESET_GRID:  // Grid
      dWisps = 4;
      dBackground = 1;
      dDensity = 30;
      dVisibility = 70;
      dSpeed = 15;
      dFeedback = 0;
      dFeedbackspeed = 1;
      dFeedbacksize = 10;
      dWireframe = true;
      dTexture = texture == TEXTURE_DEFAULT ? TEXTURE_NONE : texture;
      break;
    case PRESET_CUBISM:  // Cubism
      dWisps = 15;
      dBackground = 0;
      dDensity = 4;
      dVisibility = 15;
      dSpeed = 10;
      dFeedback = 0;
      dFeedbackspeed = 1;
      dFeedbacksize = 10;
      dWireframe = false;
      dTexture = texture == TEXTURE_DEFAULT ? TEXTURE_NONE : texture;
      break;
    case PRESET_BADMATH:  // Bad math
      dWisps = 2;
      dBackground = 2;
      dDensity = 20;
      dVisibility = 40;
      dSpeed = 30;
      dFeedback = 40;
      dFeedbackspeed = 5;
      dFeedbacksize = 10;
      dWireframe = true;
      dTexture = texture == TEXTURE_DEFAULT ? TEXTURE_STRINGY : texture;
      break;
    case PRESET_MTHEORY:  /* M-Theory */
      dWisps = 3;
      dBackground = 0;
      dDensity = 35;
      dVisibility = 15;
      dSpeed = 20;
      dFeedback = 40;
      dFeedbackspeed = 20;
      dFeedbacksize = 10;
      dWireframe = false;
      dTexture = texture == TEXTURE_DEFAULT ? TEXTURE_NONE : texture;
      break;
    case PRESET_UHFTEM:  /* ultra high frequency tunneling electron microscope */
      dWisps = 0;
      dBackground = 3;
      dDensity = 35;
      dVisibility = 5;
      dSpeed = 50;
      dFeedback = 0;
      dFeedbackspeed = 1;
      dFeedbacksize = 10;
      dWireframe = false;
      dTexture = texture == TEXTURE_DEFAULT ? TEXTURE_NONE : texture;
      break;
    case PRESET_JACK:  // Jack
      dWisps = 0;
      dBackground = 2;
      dDensity = 25;
      dVisibility = 10;
      dSpeed = 20;
      dFeedback = 90;
      dFeedbackspeed = 3;
      dFeedbacksize = 10;
      dWireframe = false;
      dTexture = texture == TEXTURE_DEFAULT ? TEXTURE_NONE : texture;
      break;
    case PRESET_OVERDOSE:  // Overdose
      dWisps = 2;
      dBackground = 2;
      dDensity = 100;
      dVisibility = 40;
      dSpeed = 40;
      dFeedback = 80;
      dFeedbackspeed = 10;
      dFeedbacksize = 10;
      dWireframe = false;
      dTexture = texture == TEXTURE_DEFAULT ? TEXTURE_RANDOM : texture;
      break;
    case PRESET_NOWHERE:  // Nowhere
      dWisps = 0;
      dBackground = 3;
      dDensity = 30;
      dVisibility = 40;
      dSpeed = 20;
      dFeedback = 80;
      dFeedbackspeed = 10;
      dFeedbacksize = 10;
      dWireframe = true;
      dTexture = texture == TEXTURE_DEFAULT ? TEXTURE_LINES : texture;
      break;
    case PRESET_ECHO:  // Echo
      dWisps = 3;
      dBackground = 0;
      dDensity = 35;
      dVisibility = 30;
      dSpeed = 20;
      dFeedback = 85;
      dFeedbackspeed = 15;
      dFeedbacksize = 10;
      dWireframe = false;
      dTexture = texture == TEXTURE_DEFAULT ? TEXTURE_PLASMA : texture;
      break;
    case PRESET_KALEIDOSCOPE:  // Kaleidoscope
      dWisps = 3;
      dBackground = 0;
      dDensity = 35;
      dVisibility = 40;
      dSpeed = 15;
      dFeedback = 90;
      dFeedbackspeed = 3;
      dFeedbacksize = 10;
      dWireframe = false;
      dTexture = texture == TEXTURE_DEFAULT ? TEXTURE_NONE : texture;
      break;
    }
  }

  int dWisps;
  int dBackground;
  int dDensity;
  int dVisibility;
  int dSpeed;
  int dFeedback;
  int dFeedbackspeed;
  int dFeedbacksize;
  bool dWireframe;
  int dTexture;
} g_settings;
}

//------------------------------------------------------------------------------

class CWisp
{
public:
  CWisp();
  ~CWisp();
  void update(float frameTime);
  void draw(glm::mat4& modelMat, CScreensaverEuphoria* base);
  void drawAsBackground(glm::mat4& modelMat, CScreensaverEuphoria* base);

private:
  // visibility constants
  const float m_viscon1 = float(g_settings.dVisibility) * 0.01f;
  const float m_viscon2 = 1.0f / m_viscon1;

  float ***m_vertices;
  float m_c[NUMCONSTS];     // constants
  float m_cr[NUMCONSTS];    // constants' radial position
  float m_cv[NUMCONSTS];    // constants' change velocities
  float m_hsl[3];
  float m_rgb[3];
  float m_hueSpeed;
  float m_saturationSpeed;
};

CWisp::CWisp()
{
  int i, j;
  float recHalfDens = 1.0f / (float(g_settings.dDensity) * 0.5f);

  m_vertices = new float**[g_settings.dDensity+1];
  for (i = 0; i <= g_settings.dDensity; i++)
  {
    m_vertices[i] = new float*[g_settings.dDensity+1];
    for (j = 0; j <= g_settings.dDensity; j++)
    {
      m_vertices[i][j] = new float[7];
      m_vertices[i][j][3] = float(i) * recHalfDens - 1.0f;  // x position on grid
      m_vertices[i][j][4] = float(j) * recHalfDens - 1.0f;  // y position on grid
      // distance squared from the center
      m_vertices[i][j][5] = m_vertices[i][j][3] * m_vertices[i][j][3] + m_vertices[i][j][4] * m_vertices[i][j][4];
      m_vertices[i][j][6] = 0.0f;  // intensity
    }
  }

  // initialize constants
  for (i = 0; i < NUMCONSTS; i++)
  {
    m_c[i] = rsRandf(2.0f) - 1.0f;
    m_cr[i] = rsRandf(glm::pi<float>() * 2.0f);
    m_cv[i] = rsRandf(float(g_settings.dSpeed) * 0.03f) + (float(g_settings.dSpeed) * 0.001f);
  }

  // pick color
  m_hsl[0] = rsRandf(1.0f);
  m_hsl[1] = 0.1f + rsRandf(0.9f);
  m_hsl[2] = 1.0f;
  m_hueSpeed = rsRandf(0.1f) - 0.05f;
  m_saturationSpeed = rsRandf(0.04f) + 0.001f;
}

CWisp::~CWisp()
{
  int i, j;

  for (i = 0; i <= g_settings.dDensity; i++)
  {
    for (j = 0; j <= g_settings.dDensity; j++)
    {
      delete[] m_vertices[i][j];
    }
    delete[] m_vertices[i];
  }
  delete[] m_vertices;
}

void CWisp::update(float frameTime)
{
  int i, j;
  rsVec up, right, crossvec;

  // update constants
  for (i = 0; i < NUMCONSTS; i++)
  {
    m_cr[i] += m_cv[i] * frameTime;
    if (m_cr[i] > glm::pi<float>() * 2.0f)
      m_cr[i] -= glm::pi<float>() * 2.0f;
    m_c[i] = cosf(m_cr[i]);
  }

  // update vertex positions
  for (i = 0; i <= g_settings.dDensity; i++)
  {
    for (j = 0; j <= g_settings.dDensity; j++)
    {
      m_vertices[i][j][0] = m_vertices[i][j][3] * m_vertices[i][j][3] * m_vertices[i][j][4] * m_c[0]
        + m_vertices[i][j][5] * m_c[1] + 0.5f * m_c[2];
      m_vertices[i][j][1] = m_vertices[i][j][4] * m_vertices[i][j][4] * m_vertices[i][j][5] * m_c[3]
        + m_vertices[i][j][3] * m_c[4] + 0.5f * m_c[5];
      m_vertices[i][j][2] = m_vertices[i][j][5] * m_vertices[i][j][5] * m_vertices[i][j][3] * m_c[6]
        + m_vertices[i][j][4] * m_c[7] + m_c[8];
    }
  }

  // update vertex normals for most of mesh
  for (i=1; i < g_settings.dDensity; i++)
  {
    for (j=1; j<g_settings.dDensity; j++)
    {
      up.set(m_vertices[i][j+1][0] - m_vertices[i][j-1][0],
        m_vertices[i][j+1][1] - m_vertices[i][j-1][1],
        m_vertices[i][j+1][2] - m_vertices[i][j-1][2]);
      right.set(m_vertices[i+1][j][0] - m_vertices[i-1][j][0],
        m_vertices[i+1][j][1] - m_vertices[i-1][j][1],
        m_vertices[i+1][j][2] - m_vertices[i-1][j][2]);
      up.normalize();
      right.normalize();
      crossvec.cross(right, up);
      // Use depth component of normal to compute intensity
      // This way only edges of wisp are bright
      if (crossvec[2] < 0.0f)
        crossvec[2] *= -1.0f;
      m_vertices[i][j][6] = m_viscon2 * (m_viscon1 - crossvec[2]);
      if (m_vertices[i][j][6] > 1.0f)
        m_vertices[i][j][6] = 1.0f;
      if (m_vertices[i][j][6] < 0.0f)
        m_vertices[i][j][6] = 0.0f;
    }
  }

  // update color
  m_hsl[0] += m_hueSpeed * frameTime;
  if (m_hsl[0] < 0.0f)
    m_hsl[0] += 1.0f;
  if (m_hsl[0] > 1.0f)
    m_hsl[0] -= 1.0f;
  m_hsl[1] += m_saturationSpeed * frameTime;
  if (m_hsl[1] <= 0.1f)
  {
    m_hsl[1] = 0.1f;
    m_saturationSpeed = -m_saturationSpeed;
  }
  if (m_hsl[1] >= 1.0f)
  {
    m_hsl[1] = 1.0f;
    m_saturationSpeed = -m_saturationSpeed;
  }
  hsl2rgb(m_hsl[0], m_hsl[1], m_hsl[2], m_rgb[0], m_rgb[1], m_rgb[2]);
}

void CWisp::draw(glm::mat4& modelMat, CScreensaverEuphoria* base)
{
  int i, j, ptr = 0;
  static sLight data[100*2+8];

  if (g_settings.dWireframe)
  {
    for (i=1; i < g_settings.dDensity; i++)
    {
      for (j = 0; j <= g_settings.dDensity; j++)
      {
        data[ptr  ].color = glm::vec4(m_rgb[0] + m_vertices[i][j][6] - 1.0f, m_rgb[1] + m_vertices[i][j][6] - 1.0f, m_rgb[2] + m_vertices[i][j][6] - 1.0f, 1.0f);
        data[ptr  ].coord = glm::vec2(m_vertices[i][j][3] - m_vertices[i][j][0], m_vertices[i][j][4] - m_vertices[i][j][1]);
        data[ptr++].vertex = glm::vec3(m_vertices[i][j][0], m_vertices[i][j][1], m_vertices[i][j][2]);
      }
      base->DrawEntry(GL_LINE_STRIP, data, ptr);
      ptr = 0;
    }
    for (j=1; j<g_settings.dDensity; j++)
    {
      for (i = 0; i <= g_settings.dDensity; i++)
      {
        data[ptr  ].color = glm::vec4(m_rgb[0] + m_vertices[i][j][6] - 1.0f, m_rgb[1] + m_vertices[i][j][6] - 1.0f, m_rgb[2] + m_vertices[i][j][6] - 1.0f, 1.0f);
        data[ptr  ].coord = glm::vec2(m_vertices[i][j][3] - m_vertices[i][j][0], m_vertices[i][j][4] - m_vertices[i][j][1]);
        data[ptr++].vertex = glm::vec3(m_vertices[i][j][0], m_vertices[i][j][1], m_vertices[i][j][2]);
      }
      base->DrawEntry(GL_LINE_STRIP, data, ptr);
      ptr = 0;
    }
  }
  else
  {
    for (i = 0; i < g_settings.dDensity; i++)
    {
      for (j = 0; j <= g_settings.dDensity; j++)
      {
        data[ptr  ].color = glm::vec4(m_rgb[0] + m_vertices[i+1][j][6] - 1.0f, m_rgb[1] + m_vertices[i+1][j][6] - 1.0f, m_rgb[2] + m_vertices[i+1][j][6] - 1.0f, 1.0f);
        data[ptr  ].coord = glm::vec2(m_vertices[i+1][j][3] - m_vertices[i+1][j][0], m_vertices[i+1][j][4] - m_vertices[i+1][j][1]);
        data[ptr++].vertex = glm::vec3(m_vertices[i+1][j][0], m_vertices[i+1][j][1], m_vertices[i+1][j][2]);
        data[ptr  ].color = glm::vec4(m_rgb[0] + m_vertices[i][j][6] - 1.0f, m_rgb[1] + m_vertices[i][j][6] - 1.0f, m_rgb[2] + m_vertices[i][j][6] - 1.0f, 1.0f);
        data[ptr  ].coord = glm::vec2(m_vertices[i][j][3] - m_vertices[i][j][0], m_vertices[i][j][4] - m_vertices[i][j][1]);
        data[ptr++].vertex = glm::vec3(m_vertices[i][j][0], m_vertices[i][j][1], m_vertices[i][j][2]);
      }
      base->DrawEntry(GL_TRIANGLE_STRIP, data, ptr);
      ptr = 0;
    }
  }
}

void CWisp::drawAsBackground(glm::mat4& modelMat, CScreensaverEuphoria* base)
{
  int i, j, ptr = 0;
  static sLight data[100*2+8];

  glm::mat4 modelMatTmp = modelMat;
  modelMat = glm::translate(modelMat, glm::vec3(m_c[0] * 0.2f, m_c[1] * 0.2f, 1.6f));

  if (g_settings.dWireframe)
  {
    for (i=1; i < g_settings.dDensity; i++)
    {
      for (j = 0; j <= g_settings.dDensity; j++)
      {
        data[ptr  ].color = glm::vec4(m_rgb[0] + m_vertices[i][j][6] - 1.0f, m_rgb[1] + m_vertices[i][j][6] - 1.0f, m_rgb[2] + m_vertices[i][j][6] - 1.0f, 1.0f);
        data[ptr  ].coord = glm::vec2(m_vertices[i][j][3] - m_vertices[i][j][0], m_vertices[i][j][4] - m_vertices[i][j][1]);
        data[ptr++].vertex = glm::vec3(m_vertices[i][j][3], m_vertices[i][j][4], m_vertices[i][j][6]);
      }
      base->DrawEntry(GL_LINE_STRIP, data, ptr);
      ptr = 0;
    }
    for (j=1; j<g_settings.dDensity; j++)
    {
      for (i = 0; i <= g_settings.dDensity; i++)
      {
        data[ptr  ].color = glm::vec4(m_rgb[0] + m_vertices[i][j][6] - 1.0f, m_rgb[1] + m_vertices[i][j][6] - 1.0f, m_rgb[2] + m_vertices[i][j][6] - 1.0f, 1.0f);
        data[ptr  ].coord = glm::vec2(m_vertices[i][j][3] - m_vertices[i][j][0], m_vertices[i][j][4] - m_vertices[i][j][1]);
        data[ptr++].vertex = glm::vec3(m_vertices[i][j][3], m_vertices[i][j][4], m_vertices[i][j][6]);
      }
      base->DrawEntry(GL_LINE_STRIP, data, ptr);
      ptr = 0;
    }
  }
  else
  {
    for (i = 0; i < g_settings.dDensity; i++)
    {
      for (j = 0; j <= g_settings.dDensity; j++)
      {
        data[ptr  ].color = glm::vec4(m_rgb[0] + m_vertices[i+1][j][6] - 1.0f, m_rgb[1] + m_vertices[i+1][j][6] - 1.0f, m_rgb[2] + m_vertices[i+1][j][6] - 1.0f, 1.0f);
        data[ptr  ].coord = glm::vec2(m_vertices[i+1][j][3] - m_vertices[i+1][j][0], m_vertices[i+1][j][4] - m_vertices[i+1][j][1]);
        data[ptr++].vertex = glm::vec3(m_vertices[i+1][j][3], m_vertices[i+1][j][4], m_vertices[i+1][j][6]);

        data[ptr  ].color = glm::vec4(m_rgb[0] + m_vertices[i][j][6] - 1.0f, m_rgb[1] + m_vertices[i][j][6] - 1.0f, m_rgb[2] + m_vertices[i][j][6] - 1.0f, 1.0f);
        data[ptr  ].coord = glm::vec2(m_vertices[i][j][3] - m_vertices[i][j][0], m_vertices[i][j][4] - m_vertices[i][j][1]);
        data[ptr++].vertex = glm::vec3(m_vertices[i][j][3], m_vertices[i][j][4], m_vertices[i][j][6]);
      }
      base->DrawEntry(GL_TRIANGLE_STRIP, data, ptr);
      ptr = 0;
    }
  }

  modelMat = modelMatTmp;
}

//------------------------------------------------------------------------------

bool CScreensaverEuphoria::Start()
{
  std::string fraqShader = kodi::addon::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/frag.glsl");
  std::string vertShader = kodi::addon::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
    return false;

  // Initialize pseudorandom number generator
  srand((unsigned)time(nullptr));

  g_settings.init();

  m_viewport.x = X();
  m_viewport.y = Y();
  m_viewport.width = Width();
  m_viewport.height = Height();
  glViewport(m_viewport.x, m_viewport.y, m_viewport.width, m_viewport.height);
  m_aspectRatio = float(m_viewport.width) / float(m_viewport.height);

  // setup regular drawing area just in case feedback isn't used
  m_projMat = glm::perspective(glm::radians(20.0f), m_aspectRatio, 0.01f, 20.0f);
  m_modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);
  glLineWidth(2.0f);
  glGetIntegerv(GL_UNPACK_ROW_LENGTH, &m_glUnpackRowLength);
  glGetIntegerv(GL_READ_BUFFER, &m_glReadBuffer);

  if (g_settings.dTexture)
  {
    int whichtex = g_settings.dTexture;
    if (whichtex == TEXTURE_RANDOM)  // random texture
      whichtex = rsRandi(3) + 1;

    // Initialize texture
    gli::texture Texture(gli::TARGET_2D, gli::FORMAT_L8_UNORM_PACK8, gli::texture::extent_type(TEXSIZE, TEXSIZE, 1), 1, 1, 1);
    switch(whichtex)
    {
    case TEXTURE_PLASMA:
      std::memcpy(Texture.data(), plasmamap, Texture.size());
      break;
    case TEXTURE_STRINGY:
      std::memcpy(Texture.data(), stringymap, Texture.size());
      break;
    case TEXTURE_LINES:
      std::memcpy(Texture.data(), linesmap, Texture.size());
    }
    m_texture = kodi::gui::gl::Load(Texture);
  }

  if (g_settings.dFeedback)
  {
    m_feedbackTexSize = int(powf(2, static_cast<float>(g_settings.dFeedbacksize)));
    // Feedback texture can't be bigger than the window using glCopyTexSubImage2D.
    // (This wouldn't be a limitation if we used FBOs.)
    while(m_feedbackTexSize > m_viewport.width || m_feedbackTexSize > m_viewport.height)
    {
      g_settings.dFeedbacksize -= 1;
      m_feedbackTexSize = int(powf(2, static_cast<float>(g_settings.dFeedbacksize)));
    }

    // feedback texture setup
    glGenTextures(1, &m_feedbackTex);
    BindTexture(GL_TEXTURE_2D, m_feedbackTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_feedbackTexSize, m_feedbackTexSize, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    // feedback velocity variable setup
    m_fv[0] = float(g_settings.dFeedbackspeed) * (rsRandf(0.025f) + 0.025f);
    m_fv[1] = float(g_settings.dFeedbackspeed) * (rsRandf(0.05f) + 0.05f);
    m_fv[2] = float(g_settings.dFeedbackspeed) * (rsRandf(0.05f) + 0.05f);
    m_fv[3] = float(g_settings.dFeedbackspeed) * (rsRandf(0.1f) + 0.1f);
    m_lv[0] = float(g_settings.dFeedbackspeed) * (rsRandf(0.0025f) + 0.0025f);
    m_lv[1] = float(g_settings.dFeedbackspeed) * (rsRandf(0.0025f) + 0.0025f);
    m_lv[2] = float(g_settings.dFeedbackspeed) * (rsRandf(0.0025f) + 0.0025f);
  }

  // Initialize wisps
  m_wisps.resize(g_settings.dWisps);
  m_backwisps.resize(g_settings.dBackground);

  glGenBuffers(1, &m_vertexVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);

  m_feedbackIntensity = float(g_settings.dFeedback) / 101.0f;
  m_lastTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  m_startOK = true;

  return true;
}

void CScreensaverEuphoria::Stop()
{
  if (!m_startOK)
    return;

  m_startOK = false;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &m_vertexVBO);
  m_vertexVBO = 0;

  glViewport(m_viewport.x, m_viewport.y, m_viewport.width, m_viewport.height);
  glDisable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ZERO);
  glLineWidth(1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, m_glUnpackRowLength);
  glReadBuffer(m_glReadBuffer);

  BindTexture(GL_TEXTURE_2D, 0);
  glDeleteTextures(1, &m_feedbackTex);
  m_feedbackTex = 0;
  glDeleteTextures(1, &m_texture);
  m_texture = 0;
}

void CScreensaverEuphoria::Render()
{
  if (!m_startOK)
    return;

  /*
   * Following Extra work done here in render to prevent problems with controls
   * from Kodi and during window moving.
   * TODO: Maybe add a separate interface call to inform about?
   */
  //@{
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);

  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
  glVertexAttribPointer(m_hVertex, 3, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, vertex)));
  glEnableVertexAttribArray(m_hVertex);

  glVertexAttribPointer(m_hColor, 4, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, color)));
  glEnableVertexAttribArray(m_hColor);

  glVertexAttribPointer(m_hCoord, 2, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, coord)));
  glEnableVertexAttribArray(m_hCoord);
  //@}

  double currentTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  float frameTime = static_cast<float>(currentTime - m_lastTime);
  m_lastTime = currentTime;

  int i;

  // Update wisps
  for (auto& wisp : m_wisps)
    wisp.update(frameTime);

  for (auto& wisp : m_backwisps)
    wisp.update(frameTime);

  // Render feedback and copy to texture if necessary
  if (g_settings.dFeedback)
  {
    glm::mat4 modelMat;

    sLight data[4];
    data[0].color = data[1].color = data[2].color = data[3].color = glm::vec4(m_feedbackIntensity, m_feedbackIntensity, m_feedbackIntensity, 1.0f);
    data[0].coord = glm::vec2(-0.5f, -0.5f);
    data[0].vertex = glm::vec3(-m_aspectRatio*2.0f, -2.0f, 1.25f);
    data[1].coord = glm::vec2(1.5f, -0.5f);
    data[1].vertex = glm::vec3(m_aspectRatio*2.0f, -2.0f, 1.25f);
    data[2].coord = glm::vec2(-0.5f, 1.5f);
    data[2].vertex = glm::vec3(-m_aspectRatio*2.0f, 2.0f, 1.25f);
    data[3].coord = glm::vec2(1.5f, 1.5f);
    data[3].vertex = glm::vec3(m_aspectRatio*2.0f, 2.0f, 1.25f);

    // update feedback variables
    for (i = 0; i < 4; i++)
    {
      m_fr[i] += frameTime * m_fv[i];
      if (m_fr[i] > glm::pi<float>() * 2.0f)
        m_fr[i] -= glm::pi<float>() * 2.0f;
    }
    m_f[0] = 30.0f * cosf(m_fr[0]);
    m_f[1] = 0.2f * cosf(m_fr[1]);
    m_f[2] = 0.2f * cosf(m_fr[2]);
    m_f[3] = 0.8f * cosf(m_fr[3]);
    for (i = 0; i < 3; i++)
    {
      m_lr[i] += frameTime * m_lv[i];
      if (m_lr[i] > glm::pi<float>() * 2.0f)
        m_lr[i] -= glm::pi<float>() * 2.0f;
      m_l[i] = cosf(m_lr[i]);
      m_l[i] = m_l[i] * m_l[i];
    }

    // Create drawing area for feedback texture
    glViewport(0, 0, m_feedbackTexSize, m_feedbackTexSize);

    // Draw
    glClear(GL_COLOR_BUFFER_BIT);
    BindTexture(GL_TEXTURE_2D, m_feedbackTex);

    m_projMat = glm::perspective(glm::radians(30.0f), m_aspectRatio, 0.01f, 20.0f);
    modelMat = m_modelMat;
    m_modelMat = glm::translate(m_modelMat, glm::vec3(m_f[1] * m_l[1], m_f[2] * m_l[1], m_f[3] * m_l[2]));
    m_modelMat = glm::rotate(m_modelMat, glm::radians(m_f[0] * m_l[0]), glm::vec3(0.0f, 0.0f, 1.0f));
    DrawEntry(GL_TRIANGLE_STRIP, data, 4);
    m_modelMat = modelMat;

    BindTexture(GL_TEXTURE_2D, m_texture);

    for (auto& wisp : m_backwisps)
      wisp.drawAsBackground(m_modelMat, this);

    for (auto& wisp : m_wisps)
      wisp.draw(m_modelMat, this);

    // readback feedback texture
    glReadBuffer(GL_BACK);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, m_feedbackTexSize);
    BindTexture(GL_TEXTURE_2D, m_feedbackTex);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, m_feedbackTexSize, m_feedbackTexSize);

    // create regular drawing area
    glViewport(m_viewport.x, m_viewport.y, m_viewport.width, m_viewport.height);

    // Draw again
    glClear(GL_COLOR_BUFFER_BIT);

    m_projMat = glm::perspective(glm::radians(20.0f), m_aspectRatio, 0.01f, 20.0f);
    modelMat = m_modelMat;
    m_modelMat = glm::translate(m_modelMat, glm::vec3(m_f[1] * m_l[1], m_f[2] * m_l[1], m_f[3] * m_l[2]));
    m_modelMat = glm::rotate(m_modelMat, glm::radians(m_f[0] * m_l[0]), glm::vec3(0.0f, 0.0f, 1.0f));
    DrawEntry(GL_TRIANGLE_STRIP, data, 4);
    m_modelMat = modelMat;
  }
  // Just clear the screen if feedback is not in use
  else
    glClear(GL_COLOR_BUFFER_BIT);

  // draw regular top layer
  BindTexture(GL_TEXTURE_2D, m_texture);
  for (i = 0; i < g_settings.dBackground; i++)
    m_backwisps[i].drawAsBackground(m_modelMat, this);
  for (i = 0; i < g_settings.dWisps; i++)
    m_wisps[i].draw(m_modelMat, this);

  glDisableVertexAttribArray(m_hVertex);
  glDisableVertexAttribArray(m_hColor);
  glDisableVertexAttribArray(m_hCoord);

  glDisable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ZERO);
}

void CScreensaverEuphoria::DrawEntry(int primitive, const sLight* data, unsigned int size)
{
  EnableShader();
  glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*size, data, GL_DYNAMIC_DRAW);
  glDrawArrays(primitive, 0, size);
  DisableShader();
}

void CScreensaverEuphoria::OnCompiledAndLinked()
{
  // Variables passed directly to the Vertex shader
  m_projMatLoc = glGetUniformLocation(ProgramHandle(), "u_projectionMatrix");
  m_modelViewMatLoc = glGetUniformLocation(ProgramHandle(), "u_modelViewMatrix");
  m_textureIdLoc = glGetUniformLocation(ProgramHandle(), "u_textureId");

  m_hVertex = glGetAttribLocation(ProgramHandle(), "a_vertex");
  m_hColor = glGetAttribLocation(ProgramHandle(), "a_color");
  m_hCoord = glGetAttribLocation(ProgramHandle(), "a_coord");
}

bool CScreensaverEuphoria::OnEnabled()
{
  // This is called after glUseProgram()
  glUniformMatrix4fv(m_projMatLoc, 1, GL_FALSE, glm::value_ptr(m_projMat));
  glUniformMatrix4fv(m_modelViewMatLoc, 1, GL_FALSE, glm::value_ptr(m_modelMat));
  glUniform1i(m_textureIdLoc, m_textureUsed);
  return true;
}

ADDONCREATOR(CScreensaverEuphoria);
