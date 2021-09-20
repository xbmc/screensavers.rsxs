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

#include <kodi/addon-instance/Screensaver.h>
#include <kodi/gui/gl/GL.h>
#include <kodi/gui/gl/Shader.h>
#include <rsMath/rsVec.h>
#include <glm/gtc/type_ptr.hpp>

#include "flare.h"
#include "particle.h"
#include "shockwave.h"
#include "smoke.h"
#include "world.h"

struct sSkyRocketSettings
{
  sSkyRocketSettings()
  {
    setDefaults();
  }

  void setDefaults()
  {
    dMaxrockets = 8;
    dSmoke = 10;
    dExplosionsmoke = 0;
    dWind = 20;
    dAmbient = 5;
    dStardensity = 20;
    dFlare = 20;
    dMoonglow = 20;
    dSoundEnabled = false;
    dSound = 100;
    dMoon = true;
    dClouds = true;
    dEarth = true;
    dIllumination = true;

    kodi::CheckSettingInt("general.maxrockets", dMaxrockets);
    kodi::CheckSettingInt("general.smoke", dSmoke);
    kodi::CheckSettingInt("general.explosionsmoke", dExplosionsmoke);
    kodi::CheckSettingInt("general.wind", dWind);
    kodi::CheckSettingInt("general.ambient", dAmbient);
    kodi::CheckSettingInt("general.stardensity", dStardensity);
    kodi::CheckSettingInt("general.flare", dFlare);
    kodi::CheckSettingInt("general.moonglow", dMoonglow);
    kodi::CheckSettingBoolean("general.moon", dMoon);
    kodi::CheckSettingBoolean("general.clouds", dClouds);
    kodi::CheckSettingBoolean("general.earth", dEarth);
    kodi::CheckSettingBoolean("general.illumination", dIllumination);
    kodi::CheckSettingBoolean("general.sound", dSoundEnabled);
    kodi::CheckSettingInt("general.volume", dSound);
  }

  // Parameters edited in the dialog box
  int dMaxrockets;
  int dSmoke;
  int dExplosionsmoke;
  int dWind;
  int dAmbient;
  int dStardensity;
  int dFlare;
  int dMoonglow;
  bool dMoon;
  bool dClouds;
  bool dEarth;
  bool dIllumination;
  bool dSoundEnabled;
  int dSound;
  // Commands given from keyboard
  int kFireworks = 1;
  int kCamera = 1;  // 0 = paused, 1 = autonomous, 2 = mouse control
  int kNewCamera = 0;
  bool kSlowMotion = false;
  int userDefinedExplosion = -1;
};

class CSoundEngine;

class ATTRIBUTE_HIDDEN CScreensaverSkyRocket
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver,
    public kodi::gui::gl::CShaderProgram
{
public:
  CScreensaverSkyRocket()
    : m_flare(this),
      m_shockwave(this),
      m_smoke(this),
      m_world(this)
  { }

  bool Start() override;
  void Stop() override;
  void Render() override;

  void OnCompiledAndLinked() override;
  bool OnEnabled() override;

  CParticle* AddParticle();

  void Illuminate(CParticle* ill);
  void Pulling(CParticle* suck);
  void Pushing(CParticle* shock);
  void Stretching(CParticle* stretch);

  void DrawEntry(int primitive, const sLight* data, unsigned int size);
  ATTRIBUTE_FORCEINLINE void BindTexture(int type, int id)
  {
    // Needed to give shader the presence of a texture
    m_textureUsed = id;
    glBindTexture(type, id);
  }

  ATTRIBUTE_FORCEINLINE glm::mat4& ProjMatrix() { return m_projMat; }
  ATTRIBUTE_FORCEINLINE glm::mat4& ModelMatrix() { return m_modelMat; }
  ATTRIBUTE_FORCEINLINE glm::mat4& BillboardMatrix() { return m_billboardMat; }

  ATTRIBUTE_FORCEINLINE CFlare& Flare() { return m_flare; }
  ATTRIBUTE_FORCEINLINE CShockwave& Shockwave() { return m_shockwave; }
  ATTRIBUTE_FORCEINLINE CSmoke& Smoke() { return m_smoke; }
  ATTRIBUTE_FORCEINLINE CWorld& World() { return m_world; }
  ATTRIBUTE_FORCEINLINE CSoundEngine* SoundEngine() { return m_soundengine; }

  ATTRIBUTE_FORCEINLINE int XSize() { return m_xsize; }
  ATTRIBUTE_FORCEINLINE int YSize() { return m_ysize; }
  ATTRIBUTE_FORCEINLINE int CenterX() { return m_centerx; }
  ATTRIBUTE_FORCEINLINE int CenterY() { return m_centery; }
  ATTRIBUTE_FORCEINLINE float AspectRatio() { return m_aspectRatio; }
  ATTRIBUTE_FORCEINLINE float FrameTime() { return m_frameTime; }
  ATTRIBUTE_FORCEINLINE rsVec& CameraPos() { return m_cameraPos; }

  ATTRIBUTE_FORCEINLINE sSkyRocketSettings& Settings() { return m_settings; }

private:
  void Reshape();
  void RemoveParticle(unsigned int rempart);
  void SortParticles();
  void MakeFlareList();
  void RandomLookFrom(int n);
  void RandomLookAt(int n);
  void FindHeadingAndPitch(rsVec lookFrom, rsVec lookAt, float& heading, float& pitch);

  sSkyRocketSettings m_settings;
  int m_viewport[4];

  // matrix junk for drawing flares in screen space
  glm::mat4 m_projMat;
  glm::mat4 m_modelMat;

  // transformation needed for rendering particles
  glm::mat4 m_billboardMat;

  GLint m_projMatLoc = -1;
  GLint m_modelViewMatLoc = -1;
  GLint m_textureIdLoc = -1;
  GLint m_hVertex = -1;
  GLint m_hCoord = -1;
  GLint m_hColor = -1;

  GLuint m_vertexVBO = 0;

  GLfloat *m_proj = nullptr;
  GLfloat *m_model = nullptr;

  CFlare m_flare;
  CShockwave m_shockwave;
  CSmoke m_smoke;
  CWorld m_world;  // the world
  CSoundEngine* m_soundengine = nullptr;  // the sound engine

  std::vector<CFlare::data> m_lensFlares;
  int m_numFlares = 0;

  // Window variables
  int m_xsize, m_ysize, m_centerx, m_centery;
  float m_aspectRatio;

  float m_fov, m_hFov;

  // Time from one frame to the next
  float m_frameTime = 0.0f;

  // Camera variables
  rsVec m_lookFrom[3];  // 3 = position, target position, last position
  rsVec m_lookAt[3]  // 3 = position, target position, last position
    = {rsVec(0.0f, 1000.0f, 0.0f),
    rsVec(0.0f, 1000.0f, 0.0f),
    rsVec(0.0f, 1000.0f, 0.0f)};

  rsVec m_cameraPos;  // used for positioning sounds (same as lookFrom[0])
  rsVec m_cameraVel;  // used for doppler shift

  std::vector<CParticle> m_particles;
  unsigned int m_lastParticle = 0;
  #define ZOOMROCKETINACTIVE 1000000000
  unsigned int m_zoomRocket = ZOOMROCKETINACTIVE;
  int m_numRockets = 0;

  int m_lastCameraMode = -1;
  float m_cameraTime[3] = {20.0f, 0.0f, 0.0f}; // time, elapsed time, step (1.0 - 0.0)

  float m_zoom = 0.0f;  // For interpolating from regular camera view to zoomed in view
  float m_zoomTime[2] = {300.0f, 0.0f};  // time until next zoom, duration of zoom
  float m_zoomHeading = 0.0f;
  float m_zoomPitch = 0.0f;
  float m_headings, m_pitch;

  bool m_first = true;
  int m_superFast;

  // Change rocket firing rate
  float m_rocketTimer;
  // a rocket usually lasts about 10 seconds, so fastest rate is all rockets within 10 seconds
  float m_rocketTimeConst;
  float m_changeRocketTimeConst;

  float m_ambientlight;

  bool m_startOK = false;
  int m_textureUsed = 0;
  double m_lastTime;
};
