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

#include "main.h"
#include "spheremap.h"

#include <kodi/gui/General.h>
#include <kodi/gui/gl/Texture.h>
#include <kodi/tools/Time.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <Rgbhsl/Rgbhsl.h>
#include <Implicit/impCubeVolume.h>
#include <Implicit/impCrawlPoint.h>
#include <Implicit/impSphere.h>

#define LIGHTSIZE 64

// Parameters edited in the dialog box
namespace
{
struct sHeliosSettings
{
  sHeliosSettings()
  {
    setDefaults();
  }

  void setDefaults()
  {
    dIons = 1500;
    dSize = 10;
    dEmitters = 3;
    dAttracters = 3;
    dSpeed = 10;
    dCameraspeed = 10;
    dSurface = true;
    dBlur = 10;
  }

  int dIons;
  int dSize;
  int dEmitters;
  int dAttracters;
  int dSpeed;
  int dCameraspeed;
  bool dSurface;
  int dBlur;
} gHeliosSettings;
}

class CParticle
{
public:
  rsVec pos;
  rsVec rgb;
  float size;
};

// -----------------------------------------------------------------------------

class emitter : public CParticle
{
public:
  rsVec oldpos;
  rsVec targetpos;

  emitter();
  ~emitter(){};
  void settargetpos(rsVec target){oldpos = pos; targetpos = target;};
  void interppos(float n){pos = oldpos * (1.0f - n) + targetpos * n;};
  void update(){};
};

emitter::emitter(){
  pos = rsVec(rsRandf(1000.0f) - 500.0f, rsRandf(1000.0f) - 500.0f, rsRandf(1000.0f) - 500.0f);
}

// -----------------------------------------------------------------------------

class attracter : public CParticle
{
public:
  rsVec oldpos;
  rsVec targetpos;

  attracter();
  ~attracter(){};
  void settargetpos(rsVec target){oldpos = pos; targetpos = target;};
  void interppos(float n){pos = oldpos * (1.0f - n) + targetpos * n;};
  void update(){};
};

attracter::attracter(){
  pos = rsVec(rsRandf(1000.0f) - 500.0f, rsRandf(1000.0f) - 500.0f, rsRandf(1000.0f) - 500.0f);
}

// -----------------------------------------------------------------------------

class ion : public CParticle
{
public:
  float speed;

  ion();
  ~ion(){};
  void start(float frameTime, const rsVec& newRgb, emitter* elist);
  void update(float frameTime, const rsVec& newRgb, emitter* elist, attracter* alist);
  void draw(std::function<void(const sLight* surface, rsVec pos, float size)>(cb));
};

ion::ion()
{
  float temp;

  pos = rsVec(0.0f, 0.0f, 0.0f);
  rgb = rsVec(0.0f, 0.0f, 0.0f);
  temp = rsRandf(2.0f) + 0.4f;
  size = float(gHeliosSettings.dSize) * temp;
  speed = float(gHeliosSettings.dSpeed) * 12.0f / temp;
}

void ion::start(float frameTime, const rsVec& newRgb, emitter* elist)
{
  int i = rsRandi(gHeliosSettings.dEmitters);
  pos = elist[i].pos;
  float offset = frameTime * speed;
  switch(rsRandi(14))
  {
  case 0:
    pos[0] += offset;
    break;
  case 1:
    pos[0] -= offset;
    break;
  case 2:
    pos[1] += offset;
    break;
  case 3:
    pos[1] -= offset;
    break;
  case 4:
    pos[2] += offset;
    break;
  case 5:
    pos[2] -= offset;
    break;
  case 6:
    pos[0] += offset;
    pos[1] += offset;
    pos[2] += offset;
    break;
  case 7:
    pos[0] -= offset;
    pos[1] += offset;
    pos[2] += offset;
    break;
  case 8:
    pos[0] += offset;
    pos[1] -= offset;
    pos[2] += offset;
    break;
  case 9:
    pos[0] -= offset;
    pos[1] -= offset;
    pos[2] += offset;
    break;
  case 10:
    pos[0] += offset;
    pos[1] += offset;
    pos[2] -= offset;
    break;
  case 11:
    pos[0] -= offset;
    pos[1] += offset;
    pos[2] -= offset;
    break;
  case 12:
    pos[0] += offset;
    pos[1] -= offset;
    pos[2] -= offset;
    break;
  case 13:
    pos[0] -= offset;
    pos[1] -= offset;
    pos[2] -= offset;
  }

  rgb = newRgb;
}

void ion::update(float frameTime, const rsVec& newRgb, emitter* elist, attracter* alist)
{
  int i;
  int startOver = 0;
  static float startOverDistance;
  static rsVec force, tempvec;
  static float length, temp;

  force = rsVec(0.0f, 0.0f, 0.0f);
  for (i = 0; i < gHeliosSettings.dEmitters; i++){
    tempvec = pos - elist[i].pos;
    length = tempvec.normalize();
    if (length > 11000.0f)
      startOver = 1;
    if (length <= 1.0f)
      temp = 1.0f;
    else
      temp = 1.0f / length;
    tempvec *= temp;
    force += tempvec;
  }
  startOverDistance = speed * frameTime;
  for (i = 0; i < gHeliosSettings.dAttracters; i++){
    tempvec = alist[i].pos - pos;
    length = tempvec.normalize();
    if (length < startOverDistance)
      startOver = 1;
    if (length <= 1.0f)
      temp = 1.0f;
    else
      temp = 1.0f / length;
    tempvec *= temp;
    force += tempvec;
  }

  // Start this ion at an emitter if it gets too close to an attracter
  // or too far from an emitter
  if (startOver)
    start(frameTime, newRgb, elist);
  else{
    force.normalize();
    pos += (force * frameTime * speed);
  }
}

void ion::draw(std::function<void(const sLight* surface, rsVec pos, float size)>(cb))
{
  sLight surface[6];
  surface[0].color = rgb.v;
  surface[0].coord = sCoord(0.0f, 0.0f);
  surface[0].vertex = sPosition(-0.5f, -0.5f, 0.0f);
  surface[1].color = rgb.v;
  surface[1].coord = sCoord(1.0f, 0.0f);
  surface[1].vertex = sPosition(0.5f, -0.5f, 0.0f);
  surface[2].color = rgb.v;
  surface[2].coord = sCoord(1.0f, 1.0f);
  surface[2].vertex = sPosition(0.5f, 0.5f, 0.0f);
  surface[3].color = rgb.v;
  surface[3].coord = sCoord(0.0f, 0.0f);
  surface[3].vertex = sPosition(-0.5f, -0.5f, 0.0f);
  surface[4].color = rgb.v;
  surface[4].coord = sCoord(1.0f, 1.0f);
  surface[4].vertex = sPosition(0.5f, 0.5f, 0.0f);
  surface[5].color = rgb.v;
  surface[5].coord = sCoord(0.0f, 1.0f);
  surface[5].vertex = sPosition(-0.5f, 0.5f, 0.0f);
  cb(surface, pos, size);
}

// -----------------------------------------------------------------------------

bool CScreensaverHelios::Start()
{
  bool doingPreview = false;

  gHeliosSettings.setDefaults();

  kodi::CheckSettingInt("general.ions", gHeliosSettings.dIons);
  kodi::CheckSettingInt("general.size", gHeliosSettings.dSize);
  kodi::CheckSettingInt("general.emitters", gHeliosSettings.dEmitters);
  kodi::CheckSettingInt("general.attractors", gHeliosSettings.dAttracters);
  kodi::CheckSettingInt("general.speed", gHeliosSettings.dSpeed);
  kodi::CheckSettingInt("general.cameraspeed", gHeliosSettings.dCameraspeed);
  kodi::CheckSettingBoolean("general.isosurface", gHeliosSettings.dSurface);
  kodi::CheckSettingInt("general.blur", gHeliosSettings.dBlur);

  std::string fraqShader = kodi::GetAddonPath("resources/shaders/frag.glsl");
  std::string vertShader = kodi::GetAddonPath("resources/shaders/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
    return false;

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  int i, j;
  float x, y, temp;

  // Seed random number generator
  srand((unsigned)time(NULL));
  m_projMat = glm::perspective(glm::radians(60.0f), (float)Width() / (float)Height(), 0.1f, 10000.0f);

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);

  // Init light texture
  gli::texture Texture1(gli::TARGET_2D, gli::FORMAT_L8_UNORM_PACK8, gli::texture::extent_type(LIGHTSIZE, LIGHTSIZE, 1), 1, 1, 1);
  int ptr = 0;
  for (i = 0; i < LIGHTSIZE; i++)
  {
    for (j = 0; j < LIGHTSIZE; j++)
    {
      x = float(i - LIGHTSIZE / 2) / float(LIGHTSIZE / 2);
      y = float(j - LIGHTSIZE / 2) / float(LIGHTSIZE / 2);
      temp = 1.0f - float(sqrt((x * x) + (y * y)));
      if (temp > 1.0f)
        temp = 1.0f;
      if (temp < 0.0f)
        temp = 0.0f;
      ((unsigned char*)Texture1.data())[ptr++] = (unsigned char)(255.0f * temp * temp);
    }
  }
  m_texture_id[0] = kodi::gui::gl::Load(Texture1);

  gli::texture Texture2(gli::TARGET_2D, gli::FORMAT_RGB8_UNORM_PACK8, gli::texture::extent_type(TEXSIZE, TEXSIZE, 1), 1, 1, 1);
  std::memcpy(Texture2.data(), spheremap, Texture2.size());
  m_texture_id[1] = kodi::gui::gl::Load(Texture2);

  // Initialize particles
  m_elist = new emitter[gHeliosSettings.dEmitters];
  m_alist = new attracter[gHeliosSettings.dAttracters];
  m_ilist = new ion[gHeliosSettings.dIons];

  // Initialize surface
  if (gHeliosSettings.dSurface)
  {
    m_volume = new impCubeVolume();
    // Preview takes to long to initialize because Windows sucks,
    // so initialize smaller surface data structures.
    if (doingPreview)
      m_volume->init(14, 14, 14, 125.0f);
    else
      m_volume->init(70, 70, 70, 25.0f);
    m_volume->function = surfaceFunction;
    m_volume->base = this;
    m_surface = m_volume->getSurface();
    m_spheres = new impSphere[gHeliosSettings.dEmitters + gHeliosSettings.dAttracters];
    float sphereScaleFactor = 1.0f / sqrtf(float(2 * gHeliosSettings.dEmitters + gHeliosSettings.dAttracters));
    for (i = 0; i < gHeliosSettings.dEmitters; i++)
      m_spheres[i].setThickness(400.0f * sphereScaleFactor);
    for (i = 0; i < gHeliosSettings.dAttracters; i++)
      m_spheres[i + gHeliosSettings.dEmitters].setThickness(200.0f * sphereScaleFactor);
  }

  glGenBuffers(1, &m_vertexVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
  glGenBuffers(1, &m_indexVBO);

  glVertexAttribPointer(m_hNormal,  4, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, normal)));
  glEnableVertexAttribArray(m_hNormal);

  glVertexAttribPointer(m_hVertex,  4, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, vertex)));
  glEnableVertexAttribArray(m_hVertex);

  glVertexAttribPointer(m_hColor,  4, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, color)));
  glEnableVertexAttribArray(m_hColor);

  glVertexAttribPointer(m_hCoord, 2, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, coord)));
  glEnableVertexAttribArray(m_hCoord);

  m_lastTime = kodi::time::GetTimeSec<double>();
  m_startOK = true;
  return true;
}

void CScreensaverHelios::Stop()
{
  if (!m_startOK)
    return;

  m_startOK = false;

  glDeleteTextures(2, m_texture_id);
  memset(m_texture_id, 0, sizeof(m_texture_id));
  glDeleteBuffers(1, &m_vertexVBO);
  m_vertexVBO = 0;
  glDeleteBuffers(1, &m_indexVBO);
  m_indexVBO = 0;

  // Free memory
  delete[] m_elist;
  delete[] m_alist;
  delete[] m_ilist;
  if (gHeliosSettings.dSurface)
  {
    delete[] m_spheres;
    delete m_surface;
    delete m_volume;
  }
}

void CScreensaverHelios::Render()
{
  if (!m_startOK)
    return;

  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);

  glVertexAttribPointer(m_hNormal,  4, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, normal)));
  glEnableVertexAttribArray(m_hNormal);

  glVertexAttribPointer(m_hVertex,  4, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, vertex)));
  glEnableVertexAttribArray(m_hVertex);

  glVertexAttribPointer(m_hColor,  4, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, color)));
  glEnableVertexAttribArray(m_hColor);

  glVertexAttribPointer(m_hCoord, 2, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, coord)));
  glEnableVertexAttribArray(m_hCoord);

  glEnable(GL_BLEND);

  if (m_firstRender)
  {
    m_firstRender--;
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  double currentTime = kodi::time::GetTimeSec<double>();
  m_frameTime = currentTime - m_lastTime;
  m_lastTime = currentTime;

  int i;

  // Camera movements
  // first do translation (distance from center)
  float cameraInterp;
  m_preCameraInterp += float(gHeliosSettings.dCameraspeed) * m_frameTime * 0.01f;
  cameraInterp = 0.5f - (0.5f * cosf(m_preCameraInterp));
  m_cameraDistance = (1.0f - cameraInterp) * m_oldCameraDistance + cameraInterp * m_targetCameraDistance;
  if (m_preCameraInterp >= M_PI)
  {
    m_oldCameraDistance = m_targetCameraDistance;
    m_targetCameraDistance = -rsRandf(1300.0f) - 200.0f;
    m_preCameraInterp = 0.0f;
  }

  m_modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, m_cameraDistance));

  // then do rotation
  rsVec radialVelDiff = m_targetRadialVel - m_radialVel;
  float changeRemaining = radialVelDiff.normalize();
  float change = float(gHeliosSettings.dCameraspeed) * 0.0002f * m_frameTime;
  if (changeRemaining > change)
  {
    radialVelDiff *= change;
    m_radialVel += radialVelDiff;
  }
  else
  {
    m_radialVel = m_targetRadialVel;
    if (rsRandi(2))
    {
      m_targetRadialVel = rsVec(rsRandf(1.0f), rsRandf(1.0f), rsRandf(1.0f));
      m_targetRadialVel.normalize();
      m_targetRadialVel *= float(gHeliosSettings.dCameraspeed) * rsRandf(0.002f);
    }
    else
      m_targetRadialVel = rsVec(0.0f, 0.0f, 0.0f);
  }
  rsVec tempRadialVel = m_radialVel;
  float angle = tempRadialVel.normalize();
  rsQuat radialQuat;
  radialQuat.make(angle, tempRadialVel[0], tempRadialVel[1], tempRadialVel[2]);
  m_rotQuat.preMult(radialQuat);
  rsMatrix rotMat;
  rotMat.fromQuat(m_rotQuat);

  // make billboard matrix for rotating particles when they are drawn
  rotMat.get(m_billboardMat);

  // Calculate new color
  m_colorInterp += m_frameTime * m_colorChange;
  if (m_colorInterp >= 1.0f)
  {
    if (!rsRandi(3) && gHeliosSettings.dIons >= 100)  // change color suddenly
      m_newHsl = rsVec(rsRandf(1.0f), 1.0f - (rsRandf(1.0f) * rsRandf(1.0f)), 1.0f);
    m_oldHsl = m_newHsl;
    m_targetHsl = rsVec(rsRandf(1.0f), 1.0f - (rsRandf(1.0f) * rsRandf(1.0f)), 1.0f);
    m_colorInterp = 0.0f;
    // amount by which to change m_colorInterp each second
    m_colorChange = rsRandf(0.005f * float(gHeliosSettings.dSpeed)) + (0.002f * float(gHeliosSettings.dSpeed));
  }
  else
  {
    float diff = m_targetHsl[0] - m_oldHsl[0];
    if (diff < -0.5f || (diff > 0.0f && diff < 0.5f))
      m_newHsl[0] = m_oldHsl[0] + m_colorInterp * diff;
    else
      m_newHsl[0] = m_oldHsl[0] - m_colorInterp * diff;
    diff = m_targetHsl[1] - m_oldHsl[1];
      m_newHsl[1] = m_oldHsl[1] + m_colorInterp * diff;
    if (m_newHsl[0] < 0.0f)
      m_newHsl[0] += 1.0f;
    if (m_newHsl[0] > 1.0f)
      m_newHsl[0] -= 1.0f;
    hsl2rgb(m_newHsl[0], m_newHsl[1], 1.0f, m_newRgb[0], m_newRgb[1], m_newRgb[2]);
  }

  // Release ions
  if (m_ionsReleased < gHeliosSettings.dIons)
  {
    m_releaseTime -= m_frameTime;
    while(m_ionsReleased < gHeliosSettings.dIons && m_releaseTime <= 0.0f)
    {
      m_ilist[m_ionsReleased].start(m_frameTime, m_newRgb, m_elist);
      m_ionsReleased ++;
      // all ions released after 2 minutes
      m_releaseTime += 120.0f / float(gHeliosSettings.dIons);
    }
  }

  // Set interpolation value for emitters and attracters
  m_wait -= m_frameTime;
  if (m_wait <= 0.0f)
  {
    m_preinterp += m_frameTime * float(gHeliosSettings.dSpeed) * m_interpconst;
    m_interp = 0.5f - (0.5f * cosf(m_preinterp));
  }
  if (m_preinterp >= M_PI)
  {
    // select new taget points (not the same pattern twice in a row)
    m_lastTarget = m_newTarget;
    m_newTarget = rsRandi(10);
    if (m_newTarget == m_lastTarget)
      m_newTarget ++;
    setTargets(m_newTarget);
    m_preinterp = 0.0f;
    m_interp = 0.0f;
    m_wait = 10.0f;  // pause after forming each new pattern
    m_interpconst = 0.001f;
    if (!rsRandi(4))  // interpolate really fast sometimes
      m_interpconst = 0.1f;
  }

  // Update particles
  for (i = 0; i < gHeliosSettings.dEmitters; i++)
  {
    m_elist[i].interppos(m_interp);
    m_elist[i].update();
  }
  for (i = 0; i < gHeliosSettings.dAttracters; i++)
  {
    m_alist[i].interppos(m_interp);
    m_alist[i].update();
  }
  for (i = 0; i < m_ionsReleased; i++)
    m_ilist[i].update(m_frameTime, m_newRgb, m_elist, m_alist);

  // Calculate surface
  if (gHeliosSettings.dSurface)
  {
    for (i = 0; i < gHeliosSettings.dEmitters; i++)
      m_spheres[i].setPosition(m_elist[i].pos[0], m_elist[i].pos[1], m_elist[i].pos[2]);
    for (i = 0; i < gHeliosSettings.dAttracters; i++)
      m_spheres[gHeliosSettings.dEmitters+i].setPosition(m_alist[i].pos[0], m_alist[i].pos[1], m_alist[i].pos[2]);

    impCrawlPointVector cpv;
    for (i = 0; i < gHeliosSettings.dEmitters+gHeliosSettings.dAttracters; i++)
      m_spheres[i].addCrawlPoint(cpv);
    m_surface->reset();
    m_valuetrig += m_frameTime;
    m_volume->setSurfaceValue(0.45f + 0.05f * cosf(m_valuetrig));
    m_volume->makeSurface(cpv);
  }

  // Draw
  // clear the screen
  if (gHeliosSettings.dBlur)  // partially
  {
    glm::mat4 projMat = m_projMat;
    glm::mat4 modelMat = m_modelMat;
    m_projMat = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f);
    m_modelMat = glm::mat4(1.0f);

    sLight blur[4];
    blur[0].color = blur[1].color = blur[2].color = blur[3].color = sColor(0.0f, 0.0f, 0.0f, 0.5f - (float(sqrtf(sqrtf(float(gHeliosSettings.dBlur)))) * 0.15495f));
    blur[0].vertex = sPosition(0.0f, 0.0f, 0.0f);
    blur[1].vertex = sPosition(1.0f, 0.0f, 0.0f);
    blur[2].vertex = sPosition(0.0f, 1.0f, 0.0f);
    blur[3].vertex = sPosition(1.0f, 1.0f, 0.0f);

    EnableShader();
    glUniform1i(m_hType, 1);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*4, blur, GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    DisableShader();

    m_modelMat = modelMat;
    m_projMat = projMat;
  }
  else  // completely
    glClear(GL_COLOR_BUFFER_BIT);

  // Draw ions
  glBlendFunc(GL_ONE, GL_ONE);
  glBindTexture(GL_TEXTURE_2D, m_texture_id[0]);
  for (i = 0; i < m_ionsReleased; i++)
  {
    m_ilist[i].draw([&](const sLight* surface, rsVec pos, float size)
    {
      // draw the surface
      glm::mat4 modelMat = m_modelMat;
      m_modelMat = glm::translate(modelMat, glm::vec3(pos[0] * m_billboardMat[0] + pos[1] * m_billboardMat[4] + pos[2] * m_billboardMat[8],
                                                      pos[0] * m_billboardMat[1] + pos[1] * m_billboardMat[5] + pos[2] * m_billboardMat[9],
                                                      pos[0] * m_billboardMat[2] + pos[1] * m_billboardMat[6] + pos[2] * m_billboardMat[10]));
      m_modelMat = glm::scale(m_modelMat, glm::vec3(size, size, size));
      EnableShader();
      glUniform1i(m_hType, 2);
      glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*6, surface, GL_DYNAMIC_DRAW);
      glDrawArrays(GL_TRIANGLES, 0, 6);
      DisableShader();
      m_modelMat = modelMat;
    });
  }

  // Draw surfaces
  float brightFactor;
  float surfaceColor[3] = {0.0f, 0.0f, 0.0f};
  if (gHeliosSettings.dSurface)
  {
    // find color for surfaces
    if (gHeliosSettings.dIons >= 100)
    {
      brightFactor = 4.0f / (float(gHeliosSettings.dBlur + 30) * float(gHeliosSettings.dBlur + 30));
      for (i = 0; i < 100; i++)
      {
        surfaceColor[0] += m_ilist[i].rgb[0] * brightFactor;
        surfaceColor[1] += m_ilist[i].rgb[1] * brightFactor;
        surfaceColor[2] += m_ilist[i].rgb[2] * brightFactor;
      }
    }
    else
    {
      brightFactor = 400.0f / (float(gHeliosSettings.dBlur + 30) * float(gHeliosSettings.dBlur + 30));
      surfaceColor[0] = m_newRgb[0] * brightFactor;
      surfaceColor[1] = m_newRgb[1] * brightFactor;
      surfaceColor[2] = m_newRgb[2] * brightFactor;
    }

    m_surface->draw([&](bool compile, const float* vertices, unsigned int vertex_offset,
                                      const unsigned int* indices, unsigned int index_offset)
    {
      int length = vertex_offset/6;
      sLight surface[length];
      for (int i = 0; i < length; ++i)
      {
        surface[i].color.r = surfaceColor[0];
        surface[i].color.g = surfaceColor[1];
        surface[i].color.b = surfaceColor[2];

        surface[i].normal.x = vertices[i*6+0];
        surface[i].normal.y = vertices[i*6+1];
        surface[i].normal.z = vertices[i*6+2];

        surface[i].vertex.x = vertices[i*6+3];
        surface[i].vertex.y = vertices[i*6+4];
        surface[i].vertex.z = vertices[i*6+5];
      }

      // draw the surface
      glm::mat4 modelMat = m_modelMat;
      m_modelMat *= glm::mat4(m_billboardMat[0], m_billboardMat[1], m_billboardMat[2], m_billboardMat[3],
                              m_billboardMat[4], m_billboardMat[5], m_billboardMat[6], m_billboardMat[7],
                              m_billboardMat[8], m_billboardMat[9], m_billboardMat[10], m_billboardMat[11],
                              m_billboardMat[12], m_billboardMat[13], m_billboardMat[14], m_billboardMat[15]);
      m_normalMat = glm::transpose(glm::inverse(glm::mat3(m_modelMat)));
      EnableShader();
      glUniform1i(m_hType, 3);
      glBindTexture(GL_TEXTURE_2D, m_texture_id[1]);
      glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*length, surface, GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_offset * sizeof(GLuint), &(indices[0]), GL_DYNAMIC_DRAW);
      glDrawElements(GL_TRIANGLES, index_offset, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      DisableShader();
      m_modelMat = modelMat;
    });
  }

  glDisable(GL_BLEND);

  glDisableVertexAttribArray(m_hNormal);
  glDisableVertexAttribArray(m_hVertex);
  glDisableVertexAttribArray(m_hColor);
  glDisableVertexAttribArray(m_hCoord);
}

void CScreensaverHelios::OnCompiledAndLinked()
{
  // Variables passed directly to the Vertex shader
  m_hProj = glGetUniformLocation(ProgramHandle(), "u_projectionMatrix");
  m_hModel = glGetUniformLocation(ProgramHandle(), "u_modelViewMatrix");
  m_normalMatLoc = glGetUniformLocation(ProgramHandle(), "u_transposeAdjointModelViewMatrix");
  m_hType = glGetUniformLocation(ProgramHandle(), "u_type");

  m_hNormal = glGetAttribLocation(ProgramHandle(), "a_normal");
  m_hVertex = glGetAttribLocation(ProgramHandle(), "a_position");
  m_hColor = glGetAttribLocation(ProgramHandle(), "a_color");
  m_hCoord = glGetAttribLocation(ProgramHandle(), "a_coord");
}

bool CScreensaverHelios::OnEnabled()
{
  // This is called after glUseProgram()
  glUniformMatrix4fv(m_hProj, 1, GL_FALSE, glm::value_ptr(m_projMat));
  glUniformMatrix4fv(m_hModel, 1, GL_FALSE, glm::value_ptr(m_modelMat));
  glUniformMatrix3fv(m_normalMatLoc, 1, GL_FALSE, glm::value_ptr(m_normalMat));

  return true;
}

void CScreensaverHelios::setTargets(int whichTarget)
{
  int i;

  switch (whichTarget)
  {
  case 0:  // random
    for (i = 0; i < gHeliosSettings.dEmitters; i++)
      m_elist[i].settargetpos(rsVec(rsVec(rsRandf(1000.0f) - 500.0f, rsRandf(1000.0f) - 500.0f, rsRandf(1000.0f) - 500.0f)));
    for (i = 0; i < gHeliosSettings.dAttracters; i++)
      m_alist[i].settargetpos(rsVec(rsVec(rsRandf(1000.0f) - 500.0f, rsRandf(1000.0f) - 500.0f, rsRandf(1000.0f) - 500.0f)));
    break;
  case 1:  // line (all emitters on one side, all attracters on the other)
  {
    float position = -500.0f, change = 1000.0f / float(gHeliosSettings.dEmitters + gHeliosSettings.dAttracters - 1);
    for (i = 0; i < gHeliosSettings.dEmitters; i++)
    {
      m_elist[i].settargetpos(rsVec(rsVec(position, position * 0.5f, 0.0f)));
      position += change;
    }
    for (i = 0; i < gHeliosSettings.dAttracters; i++)
    {
      m_alist[i].settargetpos(rsVec(rsVec(position, position * 0.5f, 0.0f)));
      position += change;
    }
    break;
  }
  case 2:  // line (emitters and attracters staggered)
  {
    float change;
    if (gHeliosSettings.dEmitters > gHeliosSettings.dAttracters)
      change = 1000.0f / float(gHeliosSettings.dEmitters * 2 - 1);
    else
      change = 1000.0f / float(gHeliosSettings.dAttracters * 2 - 1);
    float position = -500.0f;
    for (i = 0; i < gHeliosSettings.dEmitters; i++)
    {
      m_elist[i].settargetpos(rsVec(rsVec(position, position * 0.5f, 0.0f)));
      position += change * 2.0f;
    }
    position = -500.0f + change;
    for (i = 0; i < gHeliosSettings.dAttracters; i++)
    {
      m_alist[i].settargetpos(rsVec(rsVec(position, position * 0.5f, 0.0f)));
      position += change * 2.0f;
    }
    break;
  }
  case 3:  // 2 lines (parallel)
  {
    float change = 1000.0f / float(gHeliosSettings.dEmitters * 2 - 1);
    float position = -500.0f;
    float height = -525.0f + float(gHeliosSettings.dEmitters * 25);
    for (i = 0; i < gHeliosSettings.dEmitters; i++)
    {
      m_elist[i].settargetpos(rsVec(rsVec(position, height, -50.0f)));
      position += change * 2.0f;
    }
    change = 1000.0f / float(gHeliosSettings.dAttracters * 2 - 1);
    position = -500.0f;
    height = 525.0f - float(gHeliosSettings.dAttracters * 25);
    for (i = 0; i < gHeliosSettings.dAttracters; i++)
    {
      m_alist[i].settargetpos(rsVec(rsVec(position, height, 50.0f)));
      position += change * 2.0f;
    }
    break;
  }
  case 4:  // 2 lines (skewed)
  {
    float change = 1000.0f / float(gHeliosSettings.dEmitters * 2 - 1);
    float position = -500.0f;
    float height = -525.0f + float(gHeliosSettings.dEmitters * 25);
    for (i = 0; i < gHeliosSettings.dEmitters; i++)
    {
      m_elist[i].settargetpos(rsVec(rsVec(position, height, 0.0f)));
      position += change * 2.0f;
    }
    change = 1000.0f / float(gHeliosSettings.dAttracters * 2 - 1);
    position = -500.0f;
    height = 525.0f - float(gHeliosSettings.dAttracters * 25);
    for (i = 0; i < gHeliosSettings.dAttracters; i++)
    {
      m_alist[i].settargetpos(rsVec(rsVec(10.0f, height, position)));
      position += change * 2.0f;
    }
    break;
  }
  case 5:  // random distribution across a plane
    for (i = 0; i < gHeliosSettings.dEmitters; i++)
      m_elist[i].settargetpos(rsVec(rsVec(rsRandf(1000.0f) - 500.0f, 0.0f, rsRandf(1000.0f) - 500.0f)));
    for (i = 0; i < gHeliosSettings.dAttracters; i++)
      m_alist[i].settargetpos(rsVec(rsVec(rsRandf(1000.0f) - 500.0f, 0.0f, rsRandf(1000.0f) - 500.0f)));
    break;
  case 6:  // random distribution across 2 planes
  {
    float height = -525.0f + float(gHeliosSettings.dEmitters * 25);
    for (i = 0; i < gHeliosSettings.dEmitters; i++)
      m_elist[i].settargetpos(rsVec(rsVec(rsRandf(1000.0f) - 500.0f, height, rsRandf(1000.0f) - 500.0f)));
    height = 525.0f - float(gHeliosSettings.dAttracters * 25);
    for (i = 0; i < gHeliosSettings.dAttracters; i++)
      m_alist[i].settargetpos(rsVec(rsVec(rsRandf(1000.0f) - 500.0f, height, rsRandf(1000.0f) - 500.0f)));
    break;
  }
  case 7:  // 2 rings (1 inside and 1 outside)
  {
    float angle = 0.5f, cosangle, sinangle;
    float change = M_PI*2 / float(gHeliosSettings.dEmitters);
    for (i = 0; i < gHeliosSettings.dEmitters; i++)
    {
      angle += change;
      cosangle = cosf(angle) * 200.0f;
      sinangle = sinf(angle) * 200.0f;
      m_elist[i].settargetpos(rsVec(rsVec(cosangle, sinangle, 0.0f)));
    }
    angle = 1.5f;
    change = M_PI*2 / float(gHeliosSettings.dAttracters);
    for (i = 0; i < gHeliosSettings.dAttracters; i++)
    {
      angle += change;
      cosangle = cosf(angle) * 500.0f;
      sinangle = sinf(angle) * 500.0f;
      m_alist[i].settargetpos(rsVec(rsVec(cosangle, sinangle, 0.0f)));
    }
    break;
  }
  case 8:  // ring (all emitters on one side, all attracters on the other)
  {
    float angle = 0.5f, cosangle, sinangle;
    float change = M_PI*2 / float(gHeliosSettings.dEmitters + gHeliosSettings.dAttracters);
    for (i = 0; i < gHeliosSettings.dEmitters; i++)
    {
      angle += change;
      cosangle = cosf(angle) * 500.0f;
      sinangle = sinf(angle) * 500.0f;
      m_elist[i].settargetpos(rsVec(rsVec(cosangle, sinangle, 0.0f)));
    }
    for (i = 0; i < gHeliosSettings.dAttracters; i++)
    {
      angle += change;
      cosangle = cosf(angle) * 500.0f;
      sinangle = sinf(angle) * 500.0f;
      m_alist[i].settargetpos(rsVec(rsVec(cosangle, sinangle, 0.0f)));
    }
    break;
  }
  case 9:  // ring (emitters and attracters staggered)
  {
    float change;
    if (gHeliosSettings.dEmitters > gHeliosSettings.dAttracters)
      change = M_PI*2 / float(gHeliosSettings.dEmitters * 2);
    else
      change = M_PI*2 / float(gHeliosSettings.dAttracters * 2);
    float angle = 0.5f, cosangle, sinangle;
    for (i = 0; i < gHeliosSettings.dEmitters; i++)
    {
      cosangle = cosf(angle) * 500.0f;
      sinangle = sinf(angle) * 500.0f;
      m_elist[i].settargetpos(rsVec(rsVec(cosangle, sinangle, 0.0f)));
      angle += change * 2.0f;
    }
    angle = 0.5f + change;
    for (i = 0; i < gHeliosSettings.dAttracters; i++)
    {
      cosangle = cosf(angle) * 500.0f;
      sinangle = sinf(angle) * 500.0f;
      m_alist[i].settargetpos(rsVec(rsVec(cosangle, sinangle, 0.0f)));
      angle += change * 2.0f;
    }
    break;
  }
  case 10:  // 2 points
    for (i = 0; i < gHeliosSettings.dEmitters; i++)
      m_elist[i].settargetpos(rsVec(rsVec(500.0f, 100.0f, 50.0f)));
    for (i = 0; i < gHeliosSettings.dAttracters; i++)
      m_alist[i].settargetpos(rsVec(rsVec(-500.0f, -100.0f, -50.0f)));
    break;
  }
}

float CScreensaverHelios::surfaceFunction(void* base, float* position)
{
  int points = gHeliosSettings.dEmitters + gHeliosSettings.dAttracters;

  float value = 0.0f;
  for (int i = 0; i < points; i++)
    value += static_cast<CScreensaverHelios*>(base)->m_spheres[i].value(position);

  return(value);
}

ADDONCREATOR(CScreensaverHelios);
