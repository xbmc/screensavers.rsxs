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

#include "starBurst.h"
#include "main.h"

#include "wavyNormalCubeMaps.h"
#include "flare.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <rsMath/rsMath.h>

CStarBurst::CStarBurst(CScreensaverHyperspace* base) : m_base(base)
{
  int i, j;
  float vel[3];
  float normalizer;

  // initialize stars
  m_stars = new CStretchedParticle*[SB_NUM_STARS];
  m_stars_active = new bool[SB_NUM_STARS];
  m_stars_velocity = new float*[SB_NUM_STARS];
  for (i = 0; i < SB_NUM_STARS; i++)
  {
    m_stars[i] = new CStretchedParticle(m_base);
    m_stars_active[i] = 0;
    m_stars_velocity[i] = new float[3];
    vel[0] = rsRandf(1.0f) - 0.5f;
    vel[1] = rsRandf(1.0f) - 0.5f;
    vel[2] = rsRandf(1.0f) - 0.5f;
    normalizer = (rsRandf(0.75f) + 0.25f)
      / sqrtf(vel[0] * vel[0] + vel[1] * vel[1] + vel[2] * vel[2]);
    m_stars_velocity[i][0] = vel[0] * normalizer;
    m_stars_velocity[i][1] = vel[1] * normalizer;
    m_stars_velocity[i][2] = vel[2] * normalizer;
  }

  float xyz[3];
  float ci, si, cj, sj, cjj, sjj;
  for (j = 0; j < 32; j++)
  {
    cj = cosf(float(j) * RS_PIx2 / 32.0f);
    sj = sinf(float(j) * RS_PIx2 / 32.0f);
    cjj = cosf(float(j+1) * RS_PIx2 / 32.0f);
    sjj = sinf(float(j+1) * RS_PIx2 / 32.0f);

    for (i = 0; i <= 32; i++)
    {
      ci = cosf(float(i) * RS_PIx2 / 32.0f);
      si = sinf(float(i) * RS_PIx2 / 32.0f);
      xyz[0] = sj * ci;
      xyz[1] = cj;
      xyz[2] = sj * si;
      m_light[j][i*2+0].normal = m_light[j][i*2+0].vertex = sPosition(xyz);

      xyz[0] = sjj * ci;
      xyz[1] = cjj;
      xyz[2] = sjj * si;
      m_light[j][i*2+1].normal = m_light[j][i*2+1].vertex = sPosition(xyz);
    }
  }

  m_size = 4.0f;
}

CStarBurst::~CStarBurst()
{
  delete[] m_stars_active;
  for (int i = 0; i < SB_NUM_STARS; i++)
  {
    delete[] m_stars_velocity[i];
    delete[] m_stars[i];
  }
  delete[] m_stars_velocity;
  delete[] m_stars;
}

void CStarBurst::Restart(float* position)
{
  int i;

  for (i = 0; i < SB_NUM_STARS; i++)  // don't restart if any star is still active
  {
    if (m_stars_active[i])
      return;
  }
  if (m_size < 3.0f)  // or if flare hasn't faded out completely
    return;

  for (i = 0; i < SB_NUM_STARS; i++)
  {
    m_stars_active[i] = 1;
    m_stars[i]->SetPosition(position[0], position[1], position[2]);
    m_stars[i]->SetColor(rsRandf(1.0f), rsRandf(1.0f), rsRandf(1.0f), true);
  }

  m_size = 0.0f;
  m_pos[0] = position[0];
  m_pos[1] = position[1];
  m_pos[2] = position[2];
}

void CStarBurst::DrawStars()
{
  int i;
  float distance;

  // draw stars
  m_base->BindTexture(GL_TEXTURE_2D, m_base->Flare().FlareTex()[0]);
  for (i = 0; i < SB_NUM_STARS; i++)
  {
    m_stars[i]->Position()[0] += m_stars_velocity[i][0] * m_base->FrameTime();
    m_stars[i]->Position()[1] += m_stars_velocity[i][1] * m_base->FrameTime();
    m_stars[i]->Position()[2] += m_stars_velocity[i][2] * m_base->FrameTime();
    distance = sqrtf((m_stars[i]->Position()[0] - m_base->CameraPosition()[0]) * (m_stars[i]->Position()[0] - m_base->CameraPosition()[0])
      + (m_stars[i]->Position()[1] - m_base->CameraPosition()[1]) * (m_stars[i]->Position()[1] - m_base->CameraPosition()[1])
      + (m_stars[i]->Position()[2] - m_base->CameraPosition()[2]) * (m_stars[i]->Position()[2] - m_base->CameraPosition()[2]));
    if (distance > m_base->Depth())
      m_stars_active[i] = 0;
    if (m_stars_active[i])
      m_stars[i]->Draw(m_base->CameraPosition());
  }
}

void CStarBurst::Draw(float lerp)
{
  DrawStars();

  m_size += m_base->FrameTime() * 0.5f;
  if (m_size >= 3.0f)
    return;

  // draw flare
  float brightness = 1.0f - (m_size * 0.333333f);
  if (brightness > 0.0f)
    m_base->Flare().Draw(glm::vec3(m_pos[0], m_pos[1], m_pos[2]), 1.0f, 1.0f, 1.0f, brightness);

  glm::mat4& modelMat = m_base->ModelMatrix();
  glm::mat4 modelMatOld = modelMat;

  modelMat = glm::translate(modelMat, glm::vec3(m_pos[0], m_pos[1], m_pos[2]));
  modelMat = glm::scale(modelMat, glm::vec3(m_size, m_size, m_size));

  // draw sphere
  glActiveTexture(GL_TEXTURE6);
  m_base->BindTexture(GL_TEXTURE_CUBE_MAP, m_base->NebulaTex());
  glActiveTexture(GL_TEXTURE5);
  m_base->BindTexture(GL_TEXTURE_CUBE_MAP, m_base->WavyNormalCubeMaps().Texture()[(m_base->WhichTexture() + 1) % m_base->NumAnimTexFrames()]);
  glActiveTexture(GL_TEXTURE4);
  m_base->BindTexture(GL_TEXTURE_CUBE_MAP, m_base->WavyNormalCubeMaps().Texture()[m_base->WhichTexture()]);
  glActiveTexture(GL_TEXTURE0);

  m_base->ShaderProgram(SHADER_GOO);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);
  for (int i = 0; i < 32; ++i)
    m_base->Draw(sColor(brightness, brightness, brightness, lerp), GL_TRIANGLE_STRIP, m_light[i], SB_LIGHT_SIZE);
  m_base->ShaderProgram(SHADER_NORMAL);

  modelMat = modelMatOld;
}
