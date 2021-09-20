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

#include "flare.h"
#include "main.h"

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <rsMath/rsMath.h>

CFlare::~CFlare()
{
  glDeleteTextures(4, m_flaretex);
}

void CFlare::Init(CScreensaverHyperspace* base)
{
  int i, j;
  float x, y;
  float temp;

  m_base = base;

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
      temp = temp * temp;
      m_flare1[i][j][3] = (unsigned char)(255.0f * temp);
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
      temp = temp * temp * temp * temp;
      m_flare3[i][j][3] = (unsigned char)(255.0f * temp);
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
  m_light[0].coord = sCoord(0.0f, 0.0f);
  m_light[0].vertex = sPosition(-0.5f, -0.5f, 0.0f);
  m_light[1].coord = sCoord(1.0f, 0.0f);
  m_light[1].vertex = sPosition(0.5f, -0.5f, 0.0f);
  m_light[2].coord = sCoord(0.0f, 1.0f);
  m_light[2].vertex = sPosition(-0.5f, 0.5f, 0.0f);
  m_light[3].coord = sCoord(1.0f, 1.0f);
  m_light[3].vertex = sPosition(0.5f, 0.5f, 0.0f);
}

void CFlare::Draw(glm::vec3 const& pos, float red, float green, float blue, float alpha)
{
  int xsize = m_base->Height();
  int ysize = m_base->Width();
  glm::vec3 win = glm::project(pos, m_base->ModelMatrix(), m_base->ProjectionMatrix(), m_base->ViewPort());  // in screen coordinates

  float x = (float(win.x) / float(xsize)) * m_base->AspectRatio();
  float y = float(win.y) / float(ysize);
  float diff[3] = {
    pos[0] - m_base->CameraPosition()[0],
    pos[1] - m_base->CameraPosition()[1],
    pos[2] - m_base->CameraPosition()[2]
  };
  if(diff[0] * glm::value_ptr(m_base->BillboardMatrix())[8] + diff[1] *
               glm::value_ptr(m_base->BillboardMatrix())[9] + diff[2] *
               glm::value_ptr(m_base->BillboardMatrix())[10] > 0.0f)
    return;

  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);

  // Fade alpha if source is off edge of screen
  float temp;
  float fadewidth = float(xsize) / 10.0f;
  if(y < 0)
  {
    temp = fadewidth + y;
    if(temp < 0.0f)
      return;
    alpha *= temp / fadewidth;
  }
  if(y > ysize)
  {
    temp = fadewidth - y + ysize;
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
  if(x > xsize)
  {
    temp = fadewidth - x + xsize;
    if(temp < 0.0f)
      return;
    alpha *= temp / fadewidth;
  }

  // Find lens flare vector
  // This vector runs from the light source through the screen's center
  float dx = 0.5f * m_base->AspectRatio() - x;
  float dy = 0.5f - y;

  glm::mat4& projMat = m_base->ProjectionMatrix();
  glm::mat4 projMatOld = projMat;
  glm::mat4& modelMat = m_base->ModelMatrix();
  glm::mat4 modelMatOld = modelMat;

  // Setup projection matrix
  projMat = glm::ortho(0.0f, m_base->AspectRatio(), 0.0f, 1.0f);

  // Update fractal flickering
  m_flicker += m_base->FrameTime() * (rsRandf(2.0f) - 1.0f);
  if(m_flicker < 0.9f)
    m_flicker = 0.9f;
  if(m_flicker > 1.1f)
    m_flicker = 1.1f;
  alpha *= m_flicker;

  // Draw stuff

  glBlendFunc(GL_SRC_ALPHA, GL_ONE);

  m_base->BindTexture(GL_TEXTURE_2D, m_flaretex[0]);
  modelMat = glm::translate(glm::mat4(1.0), glm::vec3(x, y, 0.0f));
  modelMat = glm::scale(modelMat, glm::vec3(0.1f * m_flicker, 0.1f * m_flicker, 1.0f));
  m_base->Draw(sColor(red, green, blue * 0.8f, alpha), GL_TRIANGLE_STRIP, m_light, 4);

  // wide flare
  m_base->BindTexture(GL_TEXTURE_2D, m_flaretex[0]);
  modelMat = glm::translate(glm::mat4(1.0), glm::vec3(x, y, 0.0f));
  modelMat = glm::scale(modelMat, glm::vec3(5.0f * alpha, 0.05f * alpha, 1.0f));
  m_base->Draw(sColor(red * 0.3f, green * 0.3f, blue, alpha), GL_TRIANGLE_STRIP, m_light, 4);

  // torus
  m_base->BindTexture(GL_TEXTURE_2D, m_flaretex[2]);
  modelMat = glm::translate(glm::mat4(1.0), glm::vec3(x, y, 0.0f));
  modelMat = glm::scale(modelMat, glm::vec3(0.5f, 0.2f, 1.0f));
  m_base->Draw(sColor(red, green * 0.5f, blue * 0.5f, alpha * 0.4f), GL_TRIANGLE_STRIP, m_light, 4);

  // 3 blueish dots
  m_base->BindTexture(GL_TEXTURE_2D, m_flaretex[1]);
  modelMat = glm::translate(glm::mat4(1.0), glm::vec3(x + dx * 0.35f, y + dy * 0.35f, 0.0f));
  modelMat = glm::scale(modelMat, glm::vec3(0.06f, 0.06f, 1.0f));
  m_base->Draw(sColor(red * 0.85f, green * 0.85f, blue, alpha * 0.5f), GL_TRIANGLE_STRIP, m_light, 4);

  m_base->BindTexture(GL_TEXTURE_2D, m_flaretex[1]);
  modelMat = glm::translate(glm::mat4(1.0), glm::vec3(x + dx * 0.45f, y + dy * 0.45f, 0.0f));
  modelMat = glm::scale(modelMat, glm::vec3(0.09f, 0.09f, 1.0f));
  m_base->Draw(sColor(red * 0.7f, green * 0.7f, blue, alpha * 0.4f), GL_TRIANGLE_STRIP, m_light, 4);

  m_base->BindTexture(GL_TEXTURE_2D, m_flaretex[1]);
  modelMat = glm::translate(glm::mat4(1.0), glm::vec3(x + dx * 0.55f, y + dy * 0.55f, 0.0f));
  modelMat = glm::scale(modelMat, glm::vec3(0.12f, 0.12f, 1.0f));
  m_base->Draw(sColor(red * 0.55f, green * 0.55f, blue, alpha * 0.3f), GL_TRIANGLE_STRIP, m_light, 4);

  // 4 more dots
  m_base->BindTexture(GL_TEXTURE_2D, m_flaretex[3]);
  modelMat = glm::translate(glm::mat4(1.0), glm::vec3(x + dx * 0.75f, y + dy * 0.75f, 0.0f));
  modelMat = glm::scale(modelMat, glm::vec3(0.14f, 0.07f, 1.0f));
  m_base->Draw(sColor(red * 0.3f, green * 0.3f, blue * 0.3f, alpha), GL_TRIANGLE_STRIP, m_light, 4);

  m_base->BindTexture(GL_TEXTURE_2D, m_flaretex[1]);
  modelMat = glm::translate(glm::mat4(1.0), glm::vec3(x + dx * 0.78f, y + dy * 0.78f, 0.0f));
  modelMat = glm::scale(modelMat, glm::vec3(0.06f, 0.06f, 1.0f));
  m_base->Draw(sColor(red * 0.3f, green * 0.4f, blue * 0.4f, alpha * 0.5f), GL_TRIANGLE_STRIP, m_light, 4);

  m_base->BindTexture(GL_TEXTURE_2D, m_flaretex[1]);
  modelMat = glm::translate(glm::mat4(1.0), glm::vec3(x + dx * 1.25f, y + dy * 1.25f, 0.0f));
  modelMat = glm::scale(modelMat, glm::vec3(0.1f, 0.1f, 1.0f));
  m_base->Draw(sColor(red * 0.3f, green * 0.4f, blue * 0.3f, alpha * 0.5f), GL_TRIANGLE_STRIP, m_light, 4);

  m_base->BindTexture(GL_TEXTURE_2D, m_flaretex[1]);
  modelMat = glm::translate(glm::mat4(1.0), glm::vec3(x + dx * 1.3f, y + dy * 1.3f, 0.0f));
  modelMat = glm::scale(modelMat, glm::vec3(0.07f, 0.07f, 1.0f));
  m_base->Draw(sColor(red * 0.6f, green * 0.45f, blue * 0.3f, alpha * 0.5f), GL_TRIANGLE_STRIP, m_light, 4);

  // stretched weird flare
  m_base->BindTexture(GL_TEXTURE_2D, m_flaretex[3]);
  modelMat = glm::translate(glm::mat4(1.0), glm::vec3(x + dx * 1.45f, y + dy * 1.45f, 0.0f));
  modelMat = glm::scale(modelMat, glm::vec3(0.8f, 0.2f, 1.0f));
  modelMat = glm::rotate(modelMat, glm::radians(x * 70.0f), glm::vec3(0, 0, 1));
  m_base->Draw(sColor(red, green, blue, alpha * 0.4f), GL_TRIANGLE_STRIP, m_light, 4);

  // circle
  m_base->BindTexture(GL_TEXTURE_2D, m_flaretex[1]);
  modelMat = glm::translate(glm::mat4(1.0), glm::vec3(x + dx * 2.0f, y + dy * 2.0f, 0.0f));
  modelMat = glm::scale(modelMat, glm::vec3(0.3f, 0.3f, 1.0f));
  m_base->Draw(sColor(red, green, blue, alpha * 0.2f), GL_TRIANGLE_STRIP, m_light, 4);

  // big weird flare
  m_base->BindTexture(GL_TEXTURE_2D, m_flaretex[3]);
  modelMat = glm::translate(glm::mat4(1.0), glm::vec3(x + dx * 2.4f, y + dy * 2.4f, 0.0f));
  modelMat = glm::rotate(modelMat, glm::radians(y * 40.0f), glm::vec3(0, 0, 1));
  modelMat = glm::scale(modelMat, glm::vec3(0.7f, 0.7f, 1.0f));
  m_base->Draw(sColor(red, green, blue, alpha * 0.3f), GL_TRIANGLE_STRIP, m_light, 4);

  projMat = projMatOld;
  modelMat = modelMatOld;
}
