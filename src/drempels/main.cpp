/*
 *  Copyright (C) 2005-2019 Team Kodi
 *  Copyright (C) 2001 Ryan M. Geiss <guava at geissworks dot com>
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
 *   http://rss-glx.sourceforge.net/
 * and reworked to GL 4.0.
 */

#include "main.h"
#include "gpoly.h"

#include <chrono>
#include <kodi/Filesystem.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <rsMath/rsMath.h>

namespace {
struct sDrempelsSettings
{
  void Load(TexMgr& textureManager)
  {
    motion_blur = kodi::GetSettingInt("general.blur");
    speed_scale = 20 - kodi::GetSettingInt("general.speed");
    tex_scale = kodi::GetSettingInt("general.scale");
    dCells = 1 << kodi::GetSettingInt("general.cells");
    dCellResolution = 1 << kodi::GetSettingInt("general.cellresolution");

    if (kodi::GetSettingBoolean("general.images-used"))
    {
      std::string path = kodi::GetSettingString("general.images");
      if (path.empty())
        path = kodi::GetAddonPath("resources/default-pictures");

      if (!path.empty())
      {
        if (kodi::vfs::DirectoryExists(path))
          textureManager.setImageDir(path.c_str());
        else
          kodi::Log(ADDON_LOG_ERROR, "In settings \"general.images\" with '%s' is not a directory", path.c_str());
      }
    }

    dTexInterval = kodi::GetSettingInt("general.texinterval");
    dTexFadeInterval = kodi::GetSettingFloat("general.texfadeinterval");
    dGenTexSize = 1 << kodi::GetSettingInt("general.gentexsize");
  }

  unsigned int dCells = 16;
  unsigned int dCellResolution = 16;
  unsigned int dTexInterval = 10;
  float dTexFadeInterval = 1;
  unsigned int dGenTexSize = 256;

  // hidden:
  float warp_factor = 0.22f;      // 0.05 = no warping, 0.25 = good warping, 0.75 = chaos
  float rotational_speed = 0.05f;
  float mode_focus_exponent = 2.9f;  // 1 = simple linear combo (very blendy) / 4 = fairly focused / 16 = 99% singular
  int tex_scale = 10;      // goes from 0 to 20; 10 is normal; 0 is down 4X; 20 is up 4X
  int speed_scale = 10;    // goes from 0 to 20; 10 is normal; 0 is down 8X; 20 is up 8X

  float anim_speed = 1.0f;
  float master_zoom = 1.0f;  // 2 = zoomed in 2x, 0.5 = zoomed OUT 2x.
  float mode_switch_speed_multiplier = 1.0f;    // 1 = normal speed, 2 = double speed, 0.5 = half speed
  int   motion_blur = 7;    // goes from 0 to 10
} gSettings;

//------------------------------------------------------------------------------

inline uint32_t rgbScale(const uint32_t c, const unsigned char scale)
{
  const uint32_t lsb = (((c & 0x00ff00ff) * scale) >> 8) & 0x00ff00ff;
  const uint32_t msb = (((c & 0xff00ff00) >> 8) * scale) & 0xff00ff00;

  return lsb | msb;
}

inline uint32_t rgbLerp(const uint32_t &c0, const uint32_t &c1, const uint16_t &scale)
{
  uint32_t sc0 = rgbScale(c0, 255 - (scale & 0xFF));
  uint32_t sc1 = rgbScale(c1, (scale & 0xFF));

  return sc0 + sc1;
}

} /* namespace */
//------------------------------------------------------------------------------


bool CScreensaverDrempels::Start()
{
  gSettings.Load(m_textureManager);

  std::string fraqShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/frag.glsl");
  std::string vertShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
    return false;

  // Window initialization
  glViewport(X(), Y(), Width(), Height());

  m_projMat = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);
  m_modelMat = glm::mat4(1.0f);

  // Keep m_cells * m_cellResolution < resolution of the window
  m_cells = gSettings.dCells;
  m_cellResolution = gSettings.dCellResolution;
  while ((m_cells * m_cellResolution > Width()) || (m_cells * m_cellResolution > Height()))
  {
    if (m_cells > m_cellResolution)
      m_cells = m_cells >> 1;
    else if (m_cellResolution > m_cells)
      m_cellResolution = m_cellResolution >> 1;
    else
      m_cells = m_cells >> 1;
  }

  gSettings.dGenTexSize = 256;
  m_textureManager.setTexSize(256, 256);

  m_fadeBuf = new uint32_t[256 * 256];

  m_textureManager.setGenTexSize(gSettings.dGenTexSize, gSettings.dGenTexSize);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Seamless source texture
  glGenTextures(1, &m_tex);
  glBindTexture(GL_TEXTURE_2D, m_tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // Current texture (for fading)
  glGenTextures(1, &m_ctex);
  glBindTexture(GL_TEXTURE_2D, m_ctex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // Previous texture (for fading)
  glGenTextures(1, &m_ptex);
  glBindTexture(GL_TEXTURE_2D, m_ptex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // Indirect texture coordinate texture
  glGenTextures(1, &m_uvtex);
  glBindTexture(GL_TEXTURE_2D, m_uvtex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // Blurred result texture
  glGenTextures(1, &m_btex);
  glBindTexture(GL_TEXTURE_2D, m_btex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  unsigned char buf = 0;
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, &buf);

  m_quad[0].coord = glm::vec2(0.0f, 0.0f);
  m_quad[0].vertex = glm::vec3(0.0f, 0.0f, 0.0f);
  m_quad[1].coord = glm::vec2(0.0f, 1.0f);
  m_quad[1].vertex = glm::vec3(0.0f, 1.0f, 0.0f);
  m_quad[2].coord = glm::vec2(1.0f, 1.0f);
  m_quad[2].vertex = glm::vec3(1.0f, 1.0f, 0.0f);
  m_quad[3].coord = glm::vec2(1.0f, 0.0f);
  m_quad[3].vertex = glm::vec3(1.0f, 0.0f, 0.0f);

  glGenBuffers(1, &m_vertexVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
  glGenBuffers(1, &m_indexVBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);

  RandomizeStartValues();

  m_textureManager.start();

  m_lastTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  m_startOK = true;

  return true;
}

void CScreensaverDrempels::Stop()
{
  if (!m_startOK)
    return;

  m_startOK = false;
  m_textureManager.stop();

  delete [] m_fadeBuf;
  delete [] m_cell;
  delete [] m_buf;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &m_vertexVBO);
  m_vertexVBO = 0;
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &m_indexVBO);
  m_indexVBO = 0;
  glDeleteTextures(1, &m_tex);
  m_tex = 0;
  glDeleteTextures(1, &m_ptex);
  m_ptex = 0;
  glDeleteTextures(1, &m_ctex);
  m_ctex = 0;
  glDeleteTextures(1, &m_uvtex);
  m_uvtex = 0;
  glDeleteTextures(1, &m_btex);
  m_btex = 0;
}

void CScreensaverDrempels::Render()
{
  if (!m_startOK)
    return;

  /*
   * Following Extra work done here in render to prevent problems with controls
   * from Kodi and during window moving.
   * TODO: Maybe add a separate interface call to inform about?
   */
  //@{
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
  glVertexAttribPointer(m_hVertex, 3, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, vertex)));
  glEnableVertexAttribArray(m_hVertex);

  glVertexAttribPointer(m_hColor, 4, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, color)));
  glEnableVertexAttribArray(m_hColor);

  glVertexAttribPointer(m_hCoord, 2, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, coord)));
  glEnableVertexAttribArray(m_hCoord);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);
  //@}

  double currentTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  float frameTime = static_cast<float>(currentTime - m_lastTime);
  m_lastTime = currentTime;

  const unsigned int UVCELLSX = m_cells + 2;
  const unsigned int UVCELLSY = m_cells + 2;
  const unsigned int FXW = (UVCELLSX - 2) * m_cellResolution;
  const unsigned int FXH = (UVCELLSY - 2) * m_cellResolution;

  if (currentTime > m_lastTexChange + gSettings.dTexInterval)
  {
    if (m_textureManager.getNext())
    {
      m_lastTexChange = currentTime;

      glBindTexture(GL_TEXTURE_2D, m_ctex);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_textureManager.getCurW(), m_textureManager.getCurH(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_textureManager.getCurTex());

      if (m_textureManager.getPrevTex())
      {
        glBindTexture(GL_TEXTURE_2D, m_ptex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_textureManager.getPrevW(), m_textureManager.getPrevH(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_textureManager.getPrevTex());
      }

      m_fadeComplete = false;
    }
  }

  if (m_textureManager.getPrevTex()
    && (currentTime < m_lastTexChange + gSettings.dTexFadeInterval)
    // Can only fade if the window is large enough to render both textures...
    && (m_textureManager.getPrevW() < static_cast<unsigned int>(Width()))
    && (m_textureManager.getPrevH() < static_cast<unsigned int>(Height()))
    && (m_textureManager.getCurW() < static_cast<unsigned int>(Width()))
    && (m_textureManager.getCurH() < static_cast<unsigned int>(Height()))
    )
  {
    const double blend = (currentTime - m_lastTexChange) / gSettings.dTexFadeInterval;

    glViewport(0, 0, m_textureManager.getPrevW(), m_textureManager.getPrevH());

    glEnable(GL_BLEND);

    glBindTexture (GL_TEXTURE_2D, m_ptex);
    DrawQuads(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    glBindTexture (GL_TEXTURE_2D, m_ctex);
    DrawQuads(glm::vec4(1.0f, 1.0f, 1.0f, blend));

    glDisable (GL_BLEND);

    glBindTexture(GL_TEXTURE_2D, m_tex);
    glReadPixels(0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, m_fadeBuf);
  }
  else if (!m_fadeComplete)
  {
    m_fadeComplete = true;
  }

  {
    float fmult = gSettings.anim_speed;
    fmult *= 0.75f;
    fmult *= pow(8.0f, 1.0f - gSettings.speed_scale*0.1f);
    m_animTime += frameTime * fmult;
  }

  if (m_textureManager.getCurTex())
  {
    const float intframe2 = m_animTime*22.5f;
    const float scale = 0.45f + 0.1f*sinf(intframe2*0.01f);
    const float rot = m_animTime*gSettings.rotational_speed*6.28f;

    if (m_cell == NULL)
      m_cell = new td_cellcornerinfo[UVCELLSX * UVCELLSY];

#define CELL(i,j) m_cell[((i) * UVCELLSX) + (j)]

    memset(m_cell, 0, sizeof(td_cellcornerinfo)*(UVCELLSX)*(UVCELLSY));

    #define NUM_MODES 7

    float t[NUM_MODES];
    t[0] = pow(0.50f + 0.50f*sinf(m_animTime*gSettings.mode_switch_speed_multiplier * 0.1216f + m_fRandStart1), 1.0f);
    t[1] = pow(0.48f + 0.48f*sinf(m_animTime*gSettings.mode_switch_speed_multiplier * 0.0625f + m_fRandStart2), 2.0f);
    t[2] = pow(0.45f + 0.45f*sinf(m_animTime*gSettings.mode_switch_speed_multiplier * 0.0253f + m_fRandStart3), 12.0f);
    t[3] = pow(0.50f + 0.50f*sinf(m_animTime*gSettings.mode_switch_speed_multiplier * 0.0916f + m_fRandStart4), 2.0f);
    t[4] = pow(0.50f + 0.50f*sinf(m_animTime*gSettings.mode_switch_speed_multiplier * 0.0625f + m_fRandStart1), 2.0f);
    t[5] = pow(0.70f + 0.50f*sinf(m_animTime*gSettings.mode_switch_speed_multiplier * 0.0466f + m_fRandStart2), 1.0f);
    t[6] = pow(0.50f + 0.50f*sinf(m_animTime*gSettings.mode_switch_speed_multiplier * 0.0587f + m_fRandStart3), 2.0f);
    //t[(intframe/120) % NUM_MODES] += 20.0f;

    // normalize
    {
      float sum = 0.0f;
      for (int i=0; i<NUM_MODES; i++) sum += t[i]*t[i];
      const float mag = 1.0f/sqrt(sum);
      for (int i=0; i<NUM_MODES; i++) t[i] *= mag;
    }

    // keep top dog at 1.0, and scale all others down by raising to some exponent
    for (int i=0; i<NUM_MODES; i++) t[i] = pow(t[i], gSettings.mode_focus_exponent);

    // bias t[1] by bass (stomach)
    //t[1] += max(0, (bass - 1.1f)*2.5f);

    // bias t[2] by treble (crazy)
    //t[2] += max(0, (treb - 1.1f)*1.5f);

    // give bias to original drempels effect
    t[0] += 0.2f;

    // re-normalize
    {
      float sum = 0.0f;
      for (int i=0; i<NUM_MODES; i++) sum += t[i]*t[i];
      const float mag = 1.0f/sqrt(sum);
      for (int i=0; i<NUM_MODES; i++) t[i] *= mag;
    }

    // orig: 1.0-4.5... now: 1.0 + 1.15*[0.0...3.0]
    const float fscale1 = 1.0f + 1.15f*(pow(2.0f, 1.0f + 0.5f*sinf(m_animTime*0.892f) + 0.5f*sinf(m_animTime*0.624f)) - 1.0f);
    const float fscale2 = 4.0f + 1.0f*sinf(m_fRandStart3 + m_animTime*0.517f) + 1.0f*sinf(m_fRandStart4 + m_animTime*0.976f);
    const float fscale3 = 4.0f + 1.0f*sinf(m_fRandStart1 + m_animTime*0.654f) + 1.0f*sinf(m_fRandStart1 + m_animTime*1.044f);
    const float fscale4 = 4.0f + 1.0f*sinf(m_fRandStart2 + m_animTime*0.517f) + 1.0f*sinf(m_fRandStart3 + m_animTime*0.976f);
    const float fscale5 = 4.0f + 1.0f*sinf(m_fRandStart4 + m_animTime*0.654f) + 1.0f*sinf(m_fRandStart2 + m_animTime*1.044f);

    const float t3_uc = 0.3f*sinf(0.217f*(m_animTime+m_fRandStart1)) + 0.2f*sinf(0.185f*(m_animTime+m_fRandStart2));
    const float t3_vc = 0.3f*cosf(0.249f*(m_animTime+m_fRandStart3)) + 0.2f*cosf(0.153f*(m_animTime+m_fRandStart4));
    const float t3_rot = 3.3f*cosf(0.1290f*(m_animTime+m_fRandStart2)) + 2.2f*cosf(0.1039f*(m_animTime+m_fRandStart3));
    const float cos_t3_rot = cosf(t3_rot);
    const float sin_t3_rot = sinf(t3_rot);
    const float t4_uc = 0.2f*sinf(0.207f*(m_animTime+m_fRandStart2)) + 0.2f*sinf(0.145f*(m_animTime+m_fRandStart4));
    const float t4_vc = 0.2f*cosf(0.219f*(m_animTime+m_fRandStart1)) + 0.2f*cosf(0.163f*(m_animTime+m_fRandStart3));
    const float t4_rot = 0.61f*cosf(0.1230f*(m_animTime+m_fRandStart4)) + 0.43f*cosf(0.1009f*(m_animTime+m_fRandStart1));
    const float cos_t4_rot = cosf(t4_rot);
    const float sin_t4_rot = sinf(t4_rot);

    const float u_delta = 0.05f;
    const float v_delta = 0.05f;

    const float u_offset = 0.5f;
    const float v_offset = 0.5f;

    for (unsigned int i=0; i<UVCELLSX; i++)
    for (unsigned int j=0; j<UVCELLSY; j++)
    {
      float base_u = (i/2*2)/(float)(UVCELLSX-2) - u_offset;
      float base_v = (j/2*2)/(float)(UVCELLSY-2) - v_offset;
      if (i & 1) base_u += u_delta;
      if (j & 1) base_v += v_delta;
      base_v *= -1.0f;

      CELL(i,j).u = 0;
      CELL(i,j).v = 0;

      // correct for aspect ratio:
      base_u *= 1.333f;

      //------------------------------ v1.0 code
      {
        float u = base_u;
        float v = base_v;
        u += gSettings.warp_factor*0.65f*sinf(intframe2*m_warp_w[0] + (base_u*m_warp_uscale[0] + base_v*m_warp_vscale[0])*6.28f + m_warp_phase[0]);
        v += gSettings.warp_factor*0.65f*sinf(intframe2*m_warp_w[1] + (base_u*m_warp_uscale[1] - base_v*m_warp_vscale[1])*6.28f + m_warp_phase[1]);
        u += gSettings.warp_factor*0.35f*sinf(intframe2*m_warp_w[2] + (base_u*m_warp_uscale[2] - base_v*m_warp_vscale[2])*6.28f + m_warp_phase[2]);
        v += gSettings.warp_factor*0.35f*sinf(intframe2*m_warp_w[3] + (base_u*m_warp_uscale[3] + base_v*m_warp_vscale[3])*6.28f + m_warp_phase[3]);
        u /= scale;
        v /= scale;

        const float ut = u;
        const float vt = v;
        u = ut*cosf(rot) - vt*sinf(rot);
        v = ut*sinf(rot) + vt*cosf(rot);

        // NOTE: THIS MULTIPLIER WAS --2.7-- IN THE ORIGINAL DREMPELS 1.0!!!
        u += 2.0f*sinf(intframe2*0.00613f);
        v += 2.0f*cosf(intframe2*0.0138f);

        CELL(i,j).u += u * t[0];
        CELL(i,j).v += v * t[0];
      }
      //------------------------------ v1.0 code

      {
        // stomach
        float u = base_u;
        float v = base_v;

        float rad = sqrt(u*u + v*v);
        float ang = atan2(u, v);

        rad *= 1.0f + 0.3f*sinf(m_animTime * 0.53f + ang*1.0f + m_fRandStart2);
        ang += 0.9f*sinf(m_animTime * 0.45f + rad*4.2f + m_fRandStart3);

        u = rad*cosf(ang)*1.7f;
        v = rad*sinf(ang)*1.7f;

        CELL(i,j).u += u * t[1];
        CELL(i,j).v += v * t[1];
      }


      {
        // crazy
        float u = base_u;
        float v = base_v;

        float rad = sqrt(u*u + v*v);
        float ang = atan2(u, v);

        rad *= 1.0f + 0.3f*sinf(m_animTime * 1.59f + ang*20.4f + m_fRandStart3);
        ang += 1.8f*sinf(m_animTime * 1.35f + rad*22.1f + m_fRandStart4);

        u = rad*cosf(ang);
        v = rad*sinf(ang);

        CELL(i,j).u += u * t[2];
        CELL(i,j).v += v * t[2];
      }

      {
        // rotation
        //float u = (i/(float)UVCELLSX)*1.6f - 0.5f - t3_uc;
        //float v = (j/(float)UVCELLSY)*1.6f - 0.5f - t3_vc;
        float u = base_u*1.6f - t3_uc;
        float v = base_v*1.6f - t3_vc;
        float u2 = u*cos_t3_rot - v*sin_t3_rot + t3_uc;
        float v2 = u*sin_t3_rot + v*cos_t3_rot + t3_vc;

        CELL(i,j).u += u2 * t[3];
        CELL(i,j).v += v2 * t[3];
      }

      {
        // zoom out & minor rotate (to keep it interesting, if isolated)
        //float u = i/(float)UVCELLSX - 0.5f - t4_uc;
        //float v = j/(float)UVCELLSY - 0.5f - t4_vc;
        float u = base_u - t4_uc;
        float v = base_v - t4_vc;

        u = u*fscale1 + t4_uc - t3_uc;
        v = v*fscale1 + t4_vc - t3_uc;

        float u2 = u*cos_t4_rot - v*sin_t4_rot + t3_uc;
        float v2 = u*sin_t4_rot + v*cos_t4_rot + t3_vc;

        CELL(i,j).u += u2 * t[4];
        CELL(i,j).v += v2 * t[4];
      }

      {
        // SWIRLIES!
        float u = base_u*1.4f;
        float v = base_v*1.4f;
        float offset = 0;//((u+2.0f)*(v-2.0f) + u*u + v*v)*50.0f;

        float u2 = u + 0.03f*sinf(u*(fscale2 + 2.0f) + v*(fscale3 + 2.0f) + m_fRandStart4 + m_animTime*1.13f + 3.0f + offset);
        float v2 = v + 0.03f*cosf(u*(fscale4 + 2.0f) - v*(fscale5 + 2.0f) + m_fRandStart2 + m_animTime*1.03f - 7.0f + offset);
        u2 += 0.024f*sinf(u*(fscale3*-0.1f) + v*(fscale5*0.9f) + m_fRandStart3 + m_animTime*0.53f - 3.0f);
        v2 += 0.024f*cosf(u*(fscale2*0.9f) + v*(fscale4*-0.1f) + m_fRandStart1 + m_animTime*0.58f + 2.0f);

        CELL(i,j).u += u2*1.25f * t[5];
        CELL(i,j).v += v2*1.25f * t[5];
      }


      {
        // tunnel
        float u = base_u*1.4f - t4_vc;
        float v = base_v*1.4f - t4_uc;

        float rad = sqrt(u*u + v*v);
        float ang = atan2(u, v);

        u = rad + 3.0f*sinf(m_animTime*0.133f + m_fRandStart1) + t4_vc;
        v = rad*0.5f * 0.1f*cosf(ang + m_animTime*0.079f + m_fRandStart4) + t4_uc;

        CELL(i,j).u += u * t[6];
        CELL(i,j).v += v * t[6];
      }
    }

    {
      float inv_master_zoom = 1.0f / (gSettings.master_zoom * 1.8f);
      inv_master_zoom *= pow(4.0f, 1.0f - gSettings.tex_scale*0.1f);
      const float int_scalar = 256.0f*(INTFACTOR);
      for (unsigned int j=0; j<UVCELLSY; j++)
      for (unsigned int i=0; i<UVCELLSX; i++)
      {
        CELL(i,j).u *= inv_master_zoom;
        CELL(i,j).v *= inv_master_zoom;
        CELL(i,j).u += 0.5f;
        CELL(i,j).v += 0.5f;
        CELL(i,j).u *= int_scalar;
        CELL(i,j).v *= int_scalar;
      }
    }

    for (unsigned int j=0; j<UVCELLSY; j++)
    for (unsigned int i=0; i<UVCELLSX-1; i+=2)
    {
      CELL(i,j).r = (CELL(i+1,j).u - CELL(i,j).u) / (u_delta*FXW);
      CELL(i,j).s = (CELL(i+1,j).v - CELL(i,j).v) / (v_delta*FXW);
    }

    for (unsigned int j=0; j<UVCELLSY-1; j+=2)
    for (unsigned int i=0; i<UVCELLSX; i+=2)
    {
      CELL(i,j).dudy = (CELL(i,j+1).u - CELL(i,j).u) / (u_delta*FXH);
      CELL(i,j).dvdy = (CELL(i,j+1).v - CELL(i,j).v) / (v_delta*FXH);
      CELL(i,j).drdy = (CELL(i,j+1).r - CELL(i,j).r) / (u_delta*FXH);
      CELL(i,j).dsdy = (CELL(i,j+1).s - CELL(i,j).s) / (v_delta*FXH);
    }

    if (m_buf == nullptr)
      m_buf = new unsigned short [FXW * FXH * 2];
    for (unsigned int jj = 0; jj < UVCELLSY - 2; jj += 2)
    {
      for (unsigned int ii = 0; ii < UVCELLSX - 2; ii += 2)
      {
        const unsigned int x0 = ii * m_cellResolution;
        const unsigned int y0 = jj * m_cellResolution;
        const unsigned int x1 = (ii + 2) * m_cellResolution;
        const unsigned int y1 = (jj + 2) * m_cellResolution;

        Warp(CELL(ii,jj), CELL(ii + 2,jj), CELL(ii,jj + 2), CELL(ii + 2,jj + 2), x1 - x0, y1 - y0, &m_buf[(y0 * FXW + x0) * 2], FXW * 2);
      }
    }

    const float blurAmount = 0.97f*pow(gSettings.motion_blur*0.1f, 0.27f);

    // Can't linearly interpolate the (u,v) mapping texture as it "overflows" from 1 to 0,
    // so render it one to one first, and then linearly scaled
    glViewport(0, 0, FXW, FXH);

    glBindTexture(GL_TEXTURE_2D, m_btex);

    DrawQuads(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    glEnable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, m_tex);

    unsigned short *uvbuf = m_buf;
    uint32_t *texbuf = m_fadeComplete ? m_textureManager.getCurTex() : m_fadeBuf;
    uint32_t *outbuf = (uint32_t *)m_buf;
    for (unsigned int ii = 0; ii < FXW * FXH; ++ii)
    {
      const uint16_t u0 = *uvbuf++;
      const uint16_t v0 = *uvbuf++;
      const uint16_t u1 = u0 + 256;
      const uint16_t v1 = v0 + 256;

      const uint32_t tl = texbuf[(v0 & 0xff00) | (u0 >> 8)];
      const uint32_t tr = texbuf[(v0 & 0xff00) | (u1 >> 8)];
      const uint32_t bl = texbuf[(v1 & 0xff00) | (u0 >> 8)];
      const uint32_t br = texbuf[(v1 & 0xff00) | (u1 >> 8)];

      const uint32_t l = rgbLerp(tl, bl, v0);
      const uint32_t r = rgbLerp(tr, br, v0);

      *outbuf++ = rgbLerp(l, r, u0);
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FXW, FXH, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_buf);

    DrawQuads(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f - blurAmount));

    glDisable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, m_btex);

    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, FXW, FXH, 0);

    glViewport(X(), Y(), Width(), Height());

    DrawQuads(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
  }

  glDisableVertexAttribArray(m_hVertex);
  glDisableVertexAttribArray(m_hColor);
  glDisableVertexAttribArray(m_hCoord);

  glBlendFunc(GL_ONE, GL_ZERO);
}

void CScreensaverDrempels::RandomizeStartValues()
{
  m_fRandStart1 = 6.28f * (rand() % 4096) / 4096.0f;
  m_fRandStart2 = 6.28f * (rand() % 4096) / 4096.0f;
  m_fRandStart3 = 6.28f * (rand() % 4096) / 4096.0f;
  m_fRandStart4 = 6.28f * (rand() % 4096) / 4096.0f;

  // randomomize rotational direction
  if (rand()%2)
    gSettings.rotational_speed *= -1.0f;

  // randomomize warping parameters
  for (int i = 0; i < 4; i++)
  {
    m_warp_w[i]      = 0.02f + 0.015f * (rand() % 4096) / 4096.0f;
    m_warp_uscale[i] = 0.23f + 0.120f * (rand() % 4096) / 4096.0f;
    m_warp_vscale[i] = 0.23f + 0.120f * (rand() % 4096) / 4096.0f;
    m_warp_phase[i]  = 6.28f * (rand() % 4096) / 4096.0f;
    if (rand() % 2)
      m_warp_w[i] *= -1;
    if (rand() % 2)
      m_warp_uscale[i] *= -1;
    if (rand() % 2)
      m_warp_vscale[i] *= -1;
    if (rand() % 2)
      m_warp_phase[i] *= -1;
  }
}

void CScreensaverDrempels::DrawQuads(const glm::vec4& color)
{
  m_quad[0].color = m_quad[1].color = m_quad[2].color = m_quad[3].color = color;
  EnableShader();
  glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*4, m_quad, GL_STATIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte)*4, m_idx, GL_STATIC_DRAW);
  glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, 0);
  DisableShader();
}

void CScreensaverDrempels::OnCompiledAndLinked()
{
  // Variables passed directly to the Vertex shader
  m_projMatLoc = glGetUniformLocation(ProgramHandle(), "u_projectionMatrix");
  m_modelViewMatLoc = glGetUniformLocation(ProgramHandle(), "u_modelViewMatrix");

  m_hVertex = glGetAttribLocation(ProgramHandle(), "a_vertex");
  m_hColor = glGetAttribLocation(ProgramHandle(), "a_color");
  m_hCoord = glGetAttribLocation(ProgramHandle(), "a_coord");
}

bool CScreensaverDrempels::OnEnabled()
{
  // This is called after glUseProgram()
  glUniformMatrix4fv(m_projMatLoc, 1, GL_FALSE, glm::value_ptr(m_projMat));
  glUniformMatrix4fv(m_modelViewMatLoc, 1, GL_FALSE, glm::value_ptr(m_modelMat));
  return true;
}

ADDONCREATOR(CScreensaverDrempels);
