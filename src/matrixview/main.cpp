/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  Copyright (C) 2003 Alexander Zolotov, Eugene Zolotov
 *  Copyright (C) 2008-2014 Vincent Launchbury
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
 *   https://sourceforge.net/projects/matrixgl/
 *   http://rss-glx.sourceforge.net/
 * and reworked to GL 4.0.
 */

#include "main.h"
#include "fonts.h"
#include "images.h"

#include <kodi/gui/General.h>
#include <kodi/gui/gl/Texture.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

bool CScreensaverMatrixView::Start()
{
  srand(time(nullptr));

  std::string fraqShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/frag.glsl");
  std::string vertShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
    return false;

  kodi::CheckSettingInt("color", m_color);
  kodi::CheckSettingInt("intensity", m_rain_intensity);
  kodi::CheckSettingBoolean("classic", m_classic);

  /* Allocations for dynamic width */
  m_text_x = ceil(text_y * ((float)Width()/Height()));

  /* Start at rand pic */
  m_pic_offset=(rtext_x*text_y)*(rand()%m_num_pics);

  /* Initializations */
  if (m_text_x & 1)
    m_text_x++;
  if (m_text_x < 90)
    m_text_x=90; /* Sanity check */
  m_speeds = (unsigned char*)malloc(m_text_x);
  m_glyphs = (glyph*)malloc(sizeof(struct glyph) * (m_text_x * text_y));
  for (int i=0; i<m_text_x*text_y; i++)
  {
    m_glyphs[i].alpha = 253;
    m_glyphs[i].num   = rand()%60;
    m_glyphs[i].z     = 0;
  }

  /* Init the light tables */
  for (int i = 0; i < 500; i++)
  {
    make_change();
    scroll();
  }

  glViewport(X(), Y(), Width(), Height());
  m_projModelMat = glm::perspective(glm::radians(45.0f), (GLfloat)Width()/(GLfloat)Height(), 0.1f, 200.0f) *
                   glm::translate(glm::mat4(1.0), glm::vec3(0.0f,0.0f,-89.0f));

  /* Set up column speeds */
  for (int i = 0; i < m_text_x; i++)
  {
    m_speeds[i] = rand()&1;
    /* If the column on the left is the same speed, go faster */
    if (i && m_speeds[i]==m_speeds[i-1])
      m_speeds[i]=2;
  }

  /* Create texture mipmaps */
  {
    gli::texture Texture(gli::TARGET_2D, gli::FORMAT_R8_UNORM_PACK8, gli::texture::extent_type(512, 256, 1), 1, 1, 1);
    std::memcpy(Texture.data(), font, Texture.size());
    m_texture1 = kodi::gui::gl::Load(Texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, m_color == MATRIX_COLOR_RED ? GL_RED : GL_ZERO);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, m_color == MATRIX_COLOR_GREEN ? GL_RED : GL_ZERO);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, m_color == MATRIX_COLOR_BLUE ? GL_RED : GL_ZERO);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
  }
  {
    gli::texture Texture(gli::TARGET_2D, gli::FORMAT_L8_UNORM_PACK8, gli::texture::extent_type(512, 256, 1), 1, 1, 1);
    std::memcpy(Texture.data(), font, Texture.size());
    m_texture2 = kodi::gui::gl::Load(Texture);
  }
  {
    unsigned char flare[16]= { 0, 0, 0, 0, 0, 180, 0}; /* Node flare texture */
    gli::texture Texture(gli::TARGET_2D, gli::FORMAT_L8_UNORM_PACK8, gli::texture::extent_type(4, 4, 1), 1, 1, 1);
    std::memcpy(Texture.data(), flare, Texture.size());
    m_texture3 = kodi::gui::gl::Load(Texture);
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);

  glGenBuffers(1, &m_vertexVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);

  glGenBuffers(1, &m_indexVBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);

  m_startOK = true;

  return true;
}

void CScreensaverMatrixView::Stop()
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

  glDeleteTextures(1, &m_texture1);
  m_texture1 = 0;
  glDeleteTextures(1, &m_texture2);
  m_texture2 = 0;
  glDeleteTextures(1, &m_texture3);
  m_texture3 = 0;

  free(m_speeds);
  free(m_glyphs);
}

void CScreensaverMatrixView::Render()
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
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);

  glVertexAttribPointer(m_positionLoc, 3, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, vertex)));
  glEnableVertexAttribArray(m_positionLoc);

  glVertexAttribPointer(m_colorLoc, 4, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, color)));
  glEnableVertexAttribArray(m_colorLoc);

  glVertexAttribPointer(m_texCoord0Loc, 2, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, coord)));
  glEnableVertexAttribArray(m_texCoord0Loc);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  //@}

  glClear(GL_COLOR_BUFFER_BIT);

  glBindTexture(GL_TEXTURE_2D, m_texture1);
  draw_text1();

  glBindTexture(GL_TEXTURE_2D, m_texture2);
  draw_text2(0);

  glBindTexture(GL_TEXTURE_2D, m_texture3);
  draw_text2(1);

  make_change();
  scroll();

  glDisableVertexAttribArray(m_positionLoc);
  glDisableVertexAttribArray(m_colorLoc);
  glDisableVertexAttribArray(m_texCoord0Loc);
}

/* Draw character #num on the screen. */
void CScreensaverMatrixView::draw_char(long num, float light, float x, float y, float z)
{
  /* The font texture is a grid of 10x6 characters. Texture coords are
  * normalized to [0,1] and (s,t) is the top-left texel of the character
  * #num. The division by 7 ensures that rows don't evenly line up. */
  float s = (float)(num%10) / 10;
  float t = 1 - (float)(num/10)/7;

  m_light[0].color = glm::vec4(0.9, 0.4, 0.3, light/255);
  m_light[0].coord = glm::vec2(s, t);
  m_light[0].vertex = glm::vec3(x, y, z);

  m_light[1].color = glm::vec4(0.9, 0.4, 0.3, light/255);
  m_light[1].coord = glm::vec2(s + 0.1, t);
  m_light[1].vertex = glm::vec3(x + 1, y, z);

  m_light[2].color = glm::vec4(0.9, 0.4, 0.3, light/255);
  m_light[2].coord = glm::vec2(s + 0.1, t + 0.166);
  m_light[2].vertex = glm::vec3(x + 1, y - 1, z);

  m_light[3].color = glm::vec4(0.9, 0.4, 0.3, light/255);
  m_light[3].coord = glm::vec2(s, t + 0.166);
  m_light[3].vertex = glm::vec3(x, y - 1, z);

  EnableShader();
  glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*4, m_light, GL_DYNAMIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*4, m_indexer, GL_STATIC_DRAW);
  glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);
  DisableShader();
}

/* Draw flare around white characters */
void CScreensaverMatrixView::draw_flare(float x,float y,float z)
{
  m_light[0].color = glm::vec4(0.9f, 0.4f, 0.3f, 0.75f);  // Basic polygon color
  m_light[0].coord = glm::vec2(0, 0);
  m_light[0].vertex = glm::vec3(x - 1, y + 1, z);

  m_light[0].color = glm::vec4(0.9f, 0.4f, 0.3f, 0.75f);  // Basic polygon color
  m_light[1].coord = glm::vec2(0.75, 0);
  m_light[1].vertex = glm::vec3(x + 2, y + 1, z);

  m_light[0].color = glm::vec4(0.9f, 0.4f, 0.3f, 0.75f);  // Basic polygon color
  m_light[2].coord = glm::vec2(0.75, 0.75);
  m_light[2].vertex = glm::vec3(x + 2, y - 2, z);

  m_light[0].color = glm::vec4(0.9f, 0.4f, 0.3f, 0.75f);  // Basic polygon color
  m_light[3].coord = glm::vec2(0, 0.75);
  m_light[3].vertex = glm::vec3(x - 1, y - 2, z);

  EnableShader();
  glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*4, m_light, GL_DYNAMIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*4, m_indexer, GL_STATIC_DRAW);
  glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);
  DisableShader();
}

/* Draw green text on screen */
void CScreensaverMatrixView::draw_text1()
{
   int x, y, i=0, b=0;

   /* For each character, from top-left to bottom-right of screen */
   for (y=text_y/2; y>-text_y/2; y--)
   {
      for (x=-m_text_x/2; x<m_text_x/2; x++, i++)
      {
         int light = glm::clamp(m_glyphs[i].alpha + m_pic_fade, 0, 255);
         int depth = 0;

         /* If the coordinate is in the range of the 3D picture, set depth */
         if (x >= -rtext_x/2 && x<rtext_x/2)
         {
            depth = glm::clamp(pic[b+m_pic_offset]+(m_pic_fade-255), 0, 255);
            b++;

            /* Make far-back pixels darker */
            light -= depth;
            if (light < 0)
              light = 0;
         }

         m_glyphs[i].z = (float)(255-depth)/32; /* Map depth (0-255) to coord */
         draw_char(m_glyphs[i].num, light, x, y, m_glyphs[i].z);
      }
   }
}

/* Draw white characters and flares for each column */
void CScreensaverMatrixView::draw_text2(int mode)
{
   int x, y, i=0;

   /* For each character from top-left to bottom-right of screen,
    * excluding the bottom-most row. */
   for (y=text_y/2-1; y>-text_y/2; y--)
   {
      for (x=-m_text_x/2; x<m_text_x/2; x++, i++)
      {
         /* Highlight visible characters directly above a black stream */
         if (m_glyphs[i].alpha && !m_glyphs[i+m_text_x].alpha)
         {
            if (!mode)
            {
               /* White character */
               draw_char(m_glyphs[i].num, 127.5, x, y, m_glyphs[i].z);
            }
            else
            {
               draw_flare(x, y, m_glyphs[i].z);
            }
         }
      }
   }
}

void CScreensaverMatrixView::scroll()
{
  int i, speed, col=0;
  static char odd=0;

  /* Only scroll the slowest columns every second scroll() */
  odd = !odd;

  /* Scroll columns */
  for (speed=odd; speed <= 2; speed++)
  {
    for (i=m_text_x*text_y-1; i>=m_text_x; i--)
    {
      if (m_speeds[col] >= speed)
        m_glyphs[i].alpha=m_glyphs[i-m_text_x].alpha;
      if (++col >=m_text_x)
        col=0;
    }
  }

  /* Clear top line in light table */
  for(i=0; i<m_text_x; i++)
    m_glyphs[i].alpha=253;

  /* Make black bugs in top line */
  for(col=0,i=(m_text_x*text_y)/2; i<(m_text_x*text_y); i++)
  {
    if (m_glyphs[i].alpha==255)
      m_glyphs[col].alpha=m_glyphs[col+m_text_x].alpha>>1;
    if (++col >=m_text_x) col=0;
  }

  /* 3D picture transitions */
  if (!m_classic)
  {
    m_timer++;

    if (m_timer < 250)
    {
      /* Fading in */
      if ((m_pic_fade+=3)>255)
        m_pic_fade=255;
    }
    else
    {
      /* Fading out */
      if ((m_pic_fade-=3) < 0)
        m_pic_fade = 0;

      /* Transition credits from knoppix.ru -> doublecreations */
      if (m_pic_offset==(m_num_pics+1)*(rtext_x*text_y))
      {
        m_pic_offset+=rtext_x*text_y;
        m_timer=120;
      }
    }

    /* Go to next picture */
    if (m_timer>400)
    {
      m_pic_offset+=rtext_x*text_y;
      m_pic_offset%=(rtext_x*text_y)*m_num_pics;
      m_timer=0;
    }
  }
}

void CScreensaverMatrixView::make_change()
{
  for (int i=0; i<m_rain_intensity; i++)
  {
    /* Random character changes */
    int r=rand() % (m_text_x * text_y);
    m_glyphs[r].num = rand()%60;

    /* White nodes (1 in 5 chance of doing anything) */
    r=rand() % (m_text_x * 5);
    if (r<m_text_x && m_glyphs[r].alpha!=0)
      m_glyphs[r].alpha=255;
  }
}

void CScreensaverMatrixView::OnCompiledAndLinked()
{
  // Variables passed directly to the Vertex shader
  m_projModelMatLoc = glGetUniformLocation(ProgramHandle(), "u_projModelMatrix");

  m_positionLoc = glGetAttribLocation(ProgramHandle(), "a_position");
  m_texCoord0Loc = glGetAttribLocation(ProgramHandle(), "a_coord");
  m_colorLoc = glGetAttribLocation(ProgramHandle(), "a_color");
}

bool CScreensaverMatrixView::OnEnabled()
{
  // This is called after glUseProgram()
  glUniformMatrix4fv(m_projModelMatLoc, 1, GL_FALSE, glm::value_ptr(m_projModelMat));

  return true;
}

ADDONCREATOR(CScreensaverMatrixView);
