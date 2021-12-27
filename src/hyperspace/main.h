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
#include "flare.h"

#include <kodi/addon-instance/Screensaver.h>
#include <kodi/gui/gl/GL.h>
#include <kodi/gui/gl/Shader.h>
#include <glm/gtc/type_ptr.hpp>
#include <rsMath/rsMath.h>

typedef enum eShaderProgram
{
  SHADER_NORMAL = 0,
  SHADER_GOO = 1,
  SHADER_TUNNEL = 2
} eShaderProgram;

struct sHyperSpaceSettings
{
  sHyperSpaceSettings()
  {
    SetDefaults();
  }

  void SetDefaults()
  {
    dSpeed = 10;
    dStars = 2000;
    dStarSize = 10;
    dResolution = 10;
    dDepth = 5;
    dFov = 50;
    dUseTunnels = true;
    dUseGoo = true;
  }

  void Load()
  {
    SetDefaults();
    kodi::addon::CheckSettingInt("speed", dSpeed);
    kodi::addon::CheckSettingInt("stars", dStars);
    kodi::addon::CheckSettingInt("starsize", dStarSize);
    kodi::addon::CheckSettingInt("resolution", dResolution);
    kodi::addon::CheckSettingInt("depth", dDepth);
    kodi::addon::CheckSettingInt("fov", dFov);
    kodi::addon::CheckSettingBoolean("usetunnels", dUseTunnels);
    kodi::addon::CheckSettingBoolean("usegoo", dUseGoo);
  }

  int dSpeed;
  int dStars;
  int dStarSize;
  int dResolution;
  int dDepth;
  int dFov;
  bool dUseTunnels;
  bool dUseGoo;
};

class CStarBurst;
class CWavyNormalCubeMaps;
class CCausticTextures;
class CSplinePath;
class CTunnel;
class CGoo;
class CStretchedParticle;

class ATTR_DLL_LOCAL CScreensaverHyperspace
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver,
    public kodi::gui::gl::CShaderProgram
{
public:
  CScreensaverHyperspace() = default;

  bool Start() override;
  void Stop() override;
  void Render() override;

  void OnCompiledAndLinked() override;
  bool OnEnabled() override;

  ATTR_FORCEINLINE const sHyperSpaceSettings& Settings() const { return m_settings; }
  ATTR_FORCEINLINE float FrameTime() { return m_frameTime; }
  ATTR_FORCEINLINE float AspectRatio() { return m_aspectRatio; }
  ATTR_FORCEINLINE const glm::vec3& CameraPosition() const { return m_camPos; }
  ATTR_FORCEINLINE const glm::ivec4& ViewPort() const { return m_viewport; }
  ATTR_FORCEINLINE int NumAnimTexFrames() { return m_numAnimTexFrames; }
  ATTR_FORCEINLINE float Depth() { return m_depth; }
  ATTR_FORCEINLINE float Unroll() { return m_unroll; }

  ATTR_FORCEINLINE int WhichTexture() { return m_whichTexture; }
  ATTR_FORCEINLINE GLuint SpeckleTex() { return m_speckletex; }
  ATTR_FORCEINLINE GLuint SphereTex() { return m_spheretex; }
  ATTR_FORCEINLINE GLuint NebulaTex() { return m_nebulatex; }

  ATTR_FORCEINLINE glm::mat4& ProjectionMatrix() { return m_projMat; }
  ATTR_FORCEINLINE glm::mat4& ModelMatrix() { return m_modelMat; }
  ATTR_FORCEINLINE glm::mat4& BillboardMatrix() { return m_billboardMat; }

  ATTR_FORCEINLINE CFlare& Flare() { return  m_flare; }
  ATTR_FORCEINLINE CWavyNormalCubeMaps& WavyNormalCubeMaps() { return *m_theWNCM; }
  ATTR_FORCEINLINE CCausticTextures& CausticTextures() { return *m_theCausticTextures; }

  ATTR_FORCEINLINE void ShaderProgram(eShaderProgram program) { m_activeShader = program; }

  inline void BindTexture(int type, int id)
  {
    // Needed to give shader the presence of a texture
    m_textureUsed = id != 0;
    glBindTexture(type, id);
  }

  void Draw(int primitive, const sLight* data, unsigned int size);
  void Draw(const sColor& color, int primitive, const sLight* data, unsigned int size);
  void Draw(const sColor& color, const float* vertices, unsigned int vertex_offset, const unsigned int* indices, unsigned int index_offset);

private:
  sHyperSpaceSettings m_settings;
  CFlare m_flare;
  CStarBurst* m_theStarBurst = nullptr;
  CWavyNormalCubeMaps* m_theWNCM = nullptr;
  CCausticTextures* m_theCausticTextures = nullptr;
  CSplinePath* m_thePath;
  CTunnel* m_theTunnel;
  CGoo* m_theGoo;
  CStretchedParticle** m_stars;
  CStretchedParticle* m_sunStar;

  //----------------------------------------------------------------------------

  glm::mat4 m_projMat;
  glm::mat4 m_modelMat;
  glm::mat4 m_billboardMat;

  // Uniforms
  GLint m_hProj = -1;
  GLint m_hModel = -1;
  GLint u_uUniformColor = -1;
  GLint m_uUniformColorUsed = -1;
  GLint m_uFogUsed = -1;
  GLint m_uFogColor = -1;
  GLint m_ufogStart = -1;
  GLint m_ufogEnd = -1;
  GLint m_uTextureUsed = -1;

  GLint m_uTexUnit0 = -1;
  GLint m_uTexUnit1 = -1;
  GLint m_uTexUnit2 = -1;

  GLint m_uNormaltex0 = -1;
  GLint m_uNormaltex1 = -1;
  GLint m_uTex = -1;

  GLint m_uActiveShader = -1;

  // attributes
  GLint m_aNormal = -1;
  GLint m_aPosition = -1;
  GLint m_aCoord = -1;
  GLint m_aColor = -1;

  GLuint m_vertexVBO = 0;
  GLuint m_indexVBO = 0;

  GLint m_textureUsed = 0;
  GLuint m_speckletex, m_spheretex, m_nebulatex;

  GLint m_fogUsed = 1;
  GLint m_uniformColorUsed = 0;
  GLint m_activeShader = SHADER_NORMAL;
  sColor m_uniformColor;

  //----------------------------------------------------------------------------

  float m_aspectRatio;
  glm::ivec4 m_viewport;
  glm::vec3 m_camPos;
  int m_numAnimTexFrames = 20;
  float m_depth;
  float m_unroll;
  int m_whichTexture = 0;

  float m_textureTime = 0.0f;
  float m_starBurstTime = 300.0f;  // burst after 5 minutes

  // Camera movements
  float m_camHeading[3] = {0.0f, 0.0f, 0.0f};  // current, target, and last
  bool m_changeCamHeading = true;
  float m_camHeadingChangeTime[2] = {20.0f, 0.0f};  // total, elapsed
  float m_camRoll[3] = {0.0f, 0.0f, 0.0f};  // current, target, and last
  bool m_changeCamRoll = true;
  float m_camRollChangeTime[2] = {1.0f, 0.0f};  // total, elapsed

  // calculate color
  float m_goo_rgb_phase[3] = {-0.1f, -0.1f, -0.1f};
  float m_goo_rgb_speed[3] = {rsRandf(0.02f) + 0.02f, rsRandf(0.02f) + 0.02f, rsRandf(0.02f) + 0.02f};

  float m_pathDir[3] = {0.0f, 0.0f, -1.0f};
  
  std::vector<sLight> m_surface;

  bool m_first = true;
  bool m_doingPreview = false; // Preview unused here
  bool m_startOK = false;
  float m_frameTime;
  double m_lastTime;
};
