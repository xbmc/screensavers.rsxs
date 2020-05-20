/*
 *  Copyright (C) 2005-2020 Team Kodi (https://kodi.tv)
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

#include "flare.h"
#include "main.h"

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/type_ptr.hpp>

CFlare::~CFlare()
{
  glDeleteTextures(4, m_flaretex);
}

// Generate textures for lens flares
// then applies textures to geometry in display lists
void CFlare::Init()
{
  int i, j;
  float x, y;
  float temp;

  glGenTextures(4, m_flaretex);

  // First flare:  basic sphere
  for (i = 0; i < FLARESIZE; i++)
  {
    for (j = 0; j < FLARESIZE; j++)
    {
      m_flare1[i][j][0] = 255;
      m_flare1[i][j][1] = 255;
      m_flare1[i][j][2] = 255;
      x = float(i - FLARESIZE / 2) / float(FLARESIZE / 2);
      y = float(j - FLARESIZE / 2) / float(FLARESIZE / 2);
      temp = 1.0f - ((x * x) + (y * y));
      if(temp > 1.0f)
        temp = 1.0f;
      if(temp < 0.0f)
        temp = 0.0f;
      m_flare1[i][j][3] = (unsigned char)(255.0f * temp * temp * temp * temp);
    }
  }
  glBindTexture(GL_TEXTURE_2D, m_flaretex[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FLARESIZE, FLARESIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_flare1);

  // Second flare:  flattened sphere
  for (i = 0; i < FLARESIZE; i++)
  {
    for (j = 0; j < FLARESIZE; j++)
    {
      m_flare2[i][j][0] = 255;
      m_flare2[i][j][1] = 255;
      m_flare2[i][j][2] = 255;
      x = float(i - FLARESIZE / 2) / float(FLARESIZE / 2);
      y = float(j - FLARESIZE / 2) / float(FLARESIZE / 2);
      temp = 2.5f * (1.0f - ((x * x) + (y * y)));
      if(temp > 1.0f)
        temp = 1.0f;
      if(temp < 0.0f)
        temp = 0.0f;
      //temp = temp * temp * temp * temp;
      m_flare2[i][j][3] = (unsigned char)(255.0f * temp);
    }
  }
  glBindTexture(GL_TEXTURE_2D, m_flaretex[1]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FLARESIZE, FLARESIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_flare2);

  // Third flare:  torus
  for (i = 0; i < FLARESIZE; i++)
  {
    for (j = 0; j < FLARESIZE; j++)
    {
      m_flare3[i][j][0] = 255;
      m_flare3[i][j][1] = 255;
      m_flare3[i][j][2] = 255;
      x = float(i - FLARESIZE / 2) / float(FLARESIZE / 2);
      y = float(j - FLARESIZE / 2) / float(FLARESIZE / 2);
      temp = 4.0f * ((x * x) + (y * y)) * (1.0f - ((x * x) + (y * y)));
      if(temp > 1.0f)
        temp = 1.0f;
      if(temp < 0.0f)
        temp = 0.0f;
      m_flare3[i][j][3] = (unsigned char)(255.0f * temp * temp * temp * temp);
    }
  }
  glBindTexture(GL_TEXTURE_2D, m_flaretex[2]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FLARESIZE, FLARESIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_flare3);

  // Fourth flare:  weird flare
  for (i = 0; i < FLARESIZE; i++)
  {
    for (j = 0; j < FLARESIZE; j++)
    {
      x = float(i - FLARESIZE / 2) / float(FLARESIZE / 2);
      if(x < 0.0f)
        x = -x;
      y = float(j - FLARESIZE / 2) / float(FLARESIZE / 2);
      if(y < 0.0f)
        y = -y;
      m_flare4[i][j][0] = 255;
      m_flare4[i][j][1] = 255;
      temp = 0.14f * (1.0f - std::max(x, y)) / std::max((x * y), 0.05f);
      if(temp > 1.0f)
        temp = 1.0f;
      if(temp < 0.0f)
        temp = 0.0f;
      m_flare4[i][j][2] = (unsigned char)(255.0f * temp);
      temp = 0.1f * (1.0f - std::max(x, y)) / std::max((x * y), 0.1f);
      if(temp > 1.0f)
        temp = 1.0f;
      if(temp < 0.0f)
        temp = 0.0f;
      m_flare4[i][j][3] = (unsigned char)(255.0f * temp);
    }
  }
  glBindTexture(GL_TEXTURE_2D, m_flaretex[3]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FLARESIZE, FLARESIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_flare4);

  // Build display lists
  for (i = 0; i < 4; i++)
  {
    m_flares[i].texture = m_flaretex[i];
    m_flares[i].light[0].coord = sCoord(0.0f, 0.0f);
    m_flares[i].light[0].vertex = sPosition(-0.5f, -0.5f, 0.0f);
    m_flares[i].light[1].coord = sCoord(1.0f, 0.0f);
    m_flares[i].light[1].vertex = sPosition(0.5f, -0.5f, 0.0f);
    m_flares[i].light[2].coord = sCoord(0.0f, 1.0f);
    m_flares[i].light[2].vertex = sPosition(-0.5f, 0.5f, 0.0f);
    m_flares[i].light[3].coord = sCoord(1.0f, 1.0f);
    m_flares[i].light[3].vertex = sPosition(0.5f, 0.5f, 0.0f);
  }
}

// Draw a flare at a specified (x,y) location on the screen
// Screen corners are at (0,0) and (1,1)
// alpha = 0.0 for lowest intensity; alpha = 1.0 for highest intensity
void CFlare::Flare(float x, float y, float red, float green, float blue, float alpha)
{
  float dx, dy;
  float fadewidth, temp;

  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);

  // Fade alpha if source is off edge of screen
  fadewidth = float(m_base->XSize()) / 10.0f;
  if(y < 0)
  {
    temp = fadewidth + y;
    if(temp < 0.0f)
      return;
    alpha *= temp / fadewidth;
  }
  if(y > m_base->YSize())
  {
    temp = fadewidth - y + m_base->YSize();
    if(temp < 0.0f)
      return;
    alpha *= temp / fadewidth;
  }
  if(x < 0)
  {
    temp = fadewidth + x;
    if(temp < 0.0f)
      return;
    alpha *= temp / fadewidth;
  }
  if(x > m_base->XSize())
  {
    temp = fadewidth - x + m_base->XSize();
    if(temp < 0.0f)
      return;
    alpha *= temp / fadewidth;
  }

  // Find lens flare vector
  // This vector runs from the light source through the screen's center
  dx = 0.5f * m_base->AspectRatio() - x;
  dy = 0.5f - y;

  glm::mat4& modelMatrix = m_base->ModelMatrix();
  glm::mat4& projMatrix = m_base->ProjMatrix();
  glm::mat4 modelMatrixOld = modelMatrix;
  glm::mat4 projMatrixOld = projMatrix;

  // Setup projection matrix
  projMatrix = glm::ortho(0.0f, m_base->AspectRatio(), 0.0f, 1.0f);

  // Draw stuff

  // wide flare
  modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
  modelMatrix = glm::scale(modelMatrix, glm::vec3(5.0f * alpha, 0.05f * alpha, 1.0f));
  Draw(FLARE_BASIC_SPHERE, sColor(red * 0.25f, green * 0.25f, blue, alpha));

  modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
  modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 0.2f, 1.0f));
  Draw(FLARE_TORUS, sColor(red, green * 0.4f, blue * 0.4f, alpha * 0.4f));

  modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x + dx * 0.15f, y + dy * 0.15f, 0.0f));
  modelMatrix = glm::scale(modelMatrix, glm::vec3(0.04f, 0.04f, 1.0f));
  Draw(FLARE_FLATTENED_SPHERE, sColor(red * 0.9f, green * 0.9f, blue, alpha * 0.9f));

  modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x + dx * 0.25f, y + dy * 0.25f, 0.0f));
  modelMatrix = glm::scale(modelMatrix, glm::vec3(0.06f, 0.06f, 1.0f));
  Draw(FLARE_FLATTENED_SPHERE, sColor(red * 0.8f, green * 0.8f, blue, alpha * 0.9f));

  modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x + dx * 0.35f, y + dy * 0.35f, 0.0f));
  modelMatrix = glm::scale(modelMatrix, glm::vec3(0.08f, 0.08f, 1.0f));
  Draw(FLARE_FLATTENED_SPHERE, sColor(red * 0.7f, green * 0.7f, blue, alpha * 0.9f));

  modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x + dx * 1.25f, y + dy * 1.25f, 0.0f));
  modelMatrix = glm::scale(modelMatrix, glm::vec3(0.05f, 0.05f, 1.0f));
  Draw(FLARE_FLATTENED_SPHERE, sColor(red, green * 0.6f, blue * 0.6f, alpha * 0.9f));

  modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x + dx * 1.65f, y + dy * 1.65f, 0.0f));
  modelMatrix = glm::rotate(modelMatrix, glm::radians(x), glm::vec3(0.0f, 0.0f, 1.0f));
  modelMatrix = glm::scale(modelMatrix, glm::vec3(0.3f, 0.3f, 1.0f));
  Draw(FLARE_WEIRD, sColor(red, green, blue, alpha));

  modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x + dx * 1.85f, y + dy * 1.85f, 0.0f));
  modelMatrix = glm::scale(modelMatrix, glm::vec3(0.04f, 0.04f, 1.0f));
  Draw(FLARE_FLATTENED_SPHERE, sColor(red, green * 0.6f, blue * 0.6f, alpha * 0.9f));

  modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x + dx * 2.2f, y + dy * 2.2f, 0.0f));
  modelMatrix = glm::scale(modelMatrix, glm::vec3(0.3f, 0.3f, 1.0f));
  Draw(FLARE_FLATTENED_SPHERE, sColor(red, green, blue, alpha * 0.7f));

  modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x + dx * 2.5f, y + dy * 2.5f, 0.0f));
  modelMatrix = glm::scale(modelMatrix, glm::vec3(0.6f, 0.6f, 1.0f));
  Draw(FLARE_WEIRD, sColor(red, green, blue, alpha * 0.8f));

  // Unsetup model and projection matrix
  modelMatrix = modelMatrixOld;
  projMatrix = projMatrixOld;
}

void CFlare::Draw(flareType type, const sColor& color)
{
  m_flares[type].SetColor(color);

  m_base->BindTexture(GL_TEXTURE_2D, m_flares[type].texture);
  m_base->DrawEntry(GL_TRIANGLE_STRIP, m_flares[type].light, 4);
}
