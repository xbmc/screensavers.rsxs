/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 1999 Andreas Gustafsson
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
#include "colorfire_textures.h"

#include <chrono>
#include <kodi/gui/gl/Texture.h>
#include <rsMath/rsMath.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <gli/gli.hpp>
#include <bzlib.h>

#define LOAD_TEXTURE(dest, src, compressedSize, size) dest = (unsigned char *)malloc(size); BZ2_bzBuffToBuffDecompress((char *)dest, &size, (char *)src, compressedSize, 0, 0);
#define FREE_TEXTURE(tex) free(tex);

// Override GL_RED if not present with GL_LUMINANCE, e.g. on Android GLES
#ifndef GL_RED
#define GL_RED GL_LUMINANCE
#endif

typedef enum
{
  TYPE_AUTO_SELECTION = -1,
  TYPE_NONE = 0,
  TYPE_SMOKE = 1,
  TYPE_RIPPLES = 2,
  TYPE_SMOOTH = 3
} textureType;

bool CScreensaverColorFire::Start()
{
  kodi::CheckSettingInt("general.texture", m_textureType);
  if (m_textureType == TYPE_AUTO_SELECTION)
    m_textureType = rsRandi(3) + 1;

  std::string fraqShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/frag.glsl");
  std::string vertShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
    return false;

  // Window initialization
  glViewport(X(), Y(), Width(), Height());
  m_projMat = glm::perspective(glm::radians(55.0f), (GLfloat)Height() / (GLfloat)Width(), 1.0f, 20.0f);
  m_modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));

  glClearColor(0.0, 0.0, 0.0, 0.0);

  for (int i = 0; i < NR_WAVES; i++)
  {
    InitWave(i);
    m_wtime[i] = 3.0f + rsRandf(1.0f);
  }

  if (m_textureType != TYPE_NONE)
  {
    unsigned char *l_tex;
    switch (m_textureType)
    {
      case TYPE_SMOKE:
      {
        LOAD_TEXTURE(l_tex, smokemap, smokemap_compressedsize, smokemap_size);
        gli::texture Texture(gli::TARGET_2D, gli::FORMAT_RGB8_UNORM_PACK8, gli::texture::extent_type(TEXSIZE, TEXSIZE, 1), 1, 1, 1);
        std::memcpy(Texture.data(), l_tex, Texture.size());
        m_texture = kodi::gui::gl::Load(Texture);
        FREE_TEXTURE(l_tex)
        break;
      }
      case TYPE_RIPPLES:
      {
        LOAD_TEXTURE(l_tex, ripplemap, ripplemap_compressedsize, ripplemap_size);
        gli::texture Texture(gli::TARGET_2D, gli::FORMAT_RGB8_UNORM_PACK8, gli::texture::extent_type(TEXSIZE, TEXSIZE, 1), 1, 1, 1);
        std::memcpy(Texture.data(), l_tex, Texture.size());
        m_texture = kodi::gui::gl::Load(Texture);
        FREE_TEXTURE(l_tex)
        break;
      }
      case TYPE_SMOOTH:
      {
        double x, y, r, d;
        unsigned char texbuf[64][64];

        memset((void *)&texbuf, 0, 4096);

        r = 32;
        for (int i = 0; i < 64; i++)
        {
          for (int j = 0; j < 64; j++)
          {
            x = abs (i - 32);
            y = abs (j - 32);
            d = sqrt (x * x + y * y);

            if (d < r)
            {
              d = 1 - (d / r);
              texbuf[i][j] = char (255 * d * d);
            }
          }
        }

        gli::texture Texture(gli::TARGET_2D, gli::FORMAT_L8_UNORM_PACK8, gli::texture::extent_type(64, 64, 1), 1, 1, 1);
        std::memcpy(Texture.data(), texbuf, Texture.size());
        m_texture = kodi::gui::gl::Load(Texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;
      }
    }
  }
  else
  {
    unsigned char img[] = { 0, 255, 255, 0 };
    gli::texture Texture(gli::TARGET_2D, gli::FORMAT_L8_UNORM_PACK8, gli::texture::extent_type(2, 2, 1), 1, 1, 1);
    std::memcpy(Texture.data(), img, Texture.size());
    m_texture = kodi::gui::gl::Load(Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 2, 2, 0, GL_RED, GL_UNSIGNED_BYTE, img);
  }

  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);

  m_light[0].coord = glm::vec2(0.0f, 0.0f);
  m_light[0].vertex = glm::vec3(-1.0f, -1.0f, 0.0f);
  m_light[1].coord = glm::vec2(1.0f, 0.0f);
  m_light[1].vertex = glm::vec3(1.0f, -1.0f, 0.0f);
  m_light[2].coord = glm::vec2(1.0f, 1.0f);
  m_light[2].vertex = glm::vec3(1.0f, 1.0f, 0.0f);
  m_light[3].coord = glm::vec2(0.0f, 1.0f);
  m_light[3].vertex = glm::vec3(-1.0f, 1.0f, 0.0f);

  glGenBuffers(1, &m_vertexVBO);
  glGenBuffers(1, &m_indexVBO);

  m_lastTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  m_startOK = true;
  return true;
}

void CScreensaverColorFire::Stop()
{
  if (!m_startOK)
    return;

  m_startOK = false;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &m_vertexVBO);
  m_vertexVBO = 0;
  glDeleteBuffers(1, &m_indexVBO);
  m_indexVBO = 0;
  glDeleteTextures(1, &m_texture);
  m_texture = 0;
}

void CScreensaverColorFire::Render()
{
  if (!m_startOK)
    return;

  /*
   * Following Extra work done here in render to prevent problems with controls
   * from Kodi and during window moving.
   * TODO: Maybe add a separate interface call to inform about?
   */
  //@{
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);

  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);
  glBindTexture(GL_TEXTURE_2D, m_texture);

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

  glClear(GL_COLOR_BUFFER_BIT);
  glm::mat4 modelMat = m_modelMat;
  m_modelMat = glm::rotate(m_modelMat, glm::radians(m_v1), glm::vec3(1.0f, 0.0f, 0.0f));
  m_modelMat = glm::rotate(m_modelMat, glm::radians(m_v2), glm::vec3(0.0f, 1.0f, 0.0f));
  m_modelMat = glm::rotate(m_modelMat, glm::radians(m_v3), glm::vec3(0.0f, 0.0f, 1.0f));
  m_v1 += frameTime * 5;
  if (m_v1 > 360)
    m_v1 -= 360;
  m_v2 += frameTime * 10;
  if (m_v2 > 360)
    m_v2 -= 360;
  m_v3 += frameTime * 7;
  if (m_v3 > 360)
    m_v3 -= 360;
  for (int i = 0; i < NR_WAVES; i++)
    DrawWave(i, frameTime);
  m_modelMat = modelMat;

  glDisableVertexAttribArray(m_hVertex);
  glDisableVertexAttribArray(m_hColor);
  glDisableVertexAttribArray(m_hCoord);

  glBlendFunc(GL_ONE, GL_ZERO);
  glDisable(GL_BLEND);
}

void CScreensaverColorFire::InitWave(int nr)
{
  m_wtime[nr] = 0;
  m_wrot[nr] = rsRandf(360);
  m_wr[nr] = rsRandf(1.0);
  m_wg[nr] = rsRandf(1.0);
  m_wb[nr] = rsRandf(1.0);
  m_wspd[nr] = 0.5f + rsRandf(0.3f);
  m_wmax[nr] = 1.0f + rsRandf(1.0f);
}

void CScreensaverColorFire::DrawWave(int nr, float fDeltaTime)
{
  float colMod = 1.0;

  glm::mat4 modelMat = m_modelMat;
  m_modelMat = glm::scale(m_modelMat, glm::vec3(m_wtime[nr], m_wtime[nr], m_wtime[nr]));
  m_modelMat = glm::rotate(m_modelMat, glm::radians(m_wrot[nr]), glm::vec3(1.0f, 0.0f, 0.0f));
  m_modelMat = glm::rotate(m_modelMat, glm::radians(m_wrot[nr]), glm::vec3(0.0f, 1.0f, 0.0f));
  m_modelMat = glm::rotate(m_modelMat, glm::radians(m_wrot[nr]), glm::vec3(0.0f, 0.0f, 1.0f));

  if (m_wtime[nr] > (m_wmax[nr] - 1.0))
    colMod = m_wmax[nr] - m_wtime[nr];

  m_light[0].color = m_light[1].color = m_light[2].color = m_light[3].color = glm::vec4(colMod * m_wr[nr], colMod * m_wg[nr], colMod * m_wb[nr], 1.0f);

  EnableShader();
  glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*4, m_light, GL_DYNAMIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte)*4, m_idx, GL_STATIC_DRAW);
  glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, 0);
  DisableShader();

  m_wtime[nr] += fDeltaTime * m_wspd[nr];
  if (m_wtime[nr] > m_wmax[nr])
    InitWave(nr);
  m_modelMat = modelMat;
}

void CScreensaverColorFire::OnCompiledAndLinked()
{
  // Variables passed directly to the Vertex shader
  m_projMatLoc = glGetUniformLocation(ProgramHandle(), "u_projectionMatrix");
  m_modelViewMatLoc = glGetUniformLocation(ProgramHandle(), "u_modelViewMatrix");
  m_textureTypeLoc = glGetUniformLocation(ProgramHandle(), "u_type");

  m_hVertex = glGetAttribLocation(ProgramHandle(), "a_vertex");
  m_hColor = glGetAttribLocation(ProgramHandle(), "a_color");
  m_hCoord = glGetAttribLocation(ProgramHandle(), "a_coord");
}

bool CScreensaverColorFire::OnEnabled()
{
  // This is called after glUseProgram()
  glUniformMatrix4fv(m_projMatLoc, 1, GL_FALSE, glm::value_ptr(m_projMat));
  glUniformMatrix4fv(m_modelViewMatLoc, 1, GL_FALSE, glm::value_ptr(m_modelMat));
  glUniform1i(m_textureTypeLoc, m_textureType);

  return true;
}

ADDONCREATOR(CScreensaverColorFire);
