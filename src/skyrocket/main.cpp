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

#include "main.h"
#include "soundEngine.h"

#include <chrono>
#include <kodi/gui/General.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <rsMath/rsMath.h>

#include "rsTimer.h"
#include "world.h"

// Mouse variables
float mouseIdleTime;
int mouseButtons, mousex, mousey;
float mouseSpeed;

bool CScreensaverSkyRocket::Start()
{
  std::string fraqShader = kodi::addon::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/frag.glsl");
  std::string vertShader = kodi::addon::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
    return false;

  // Initialize pseudorandom number generator
  srand((unsigned)time(nullptr));

  // Window initialization
  m_xsize = Width();
  m_ysize = Height();
  m_centerx = X() + m_xsize / 2;
  m_centery = Y() + m_ysize / 2;
  glViewport(X(), Y(), m_xsize, m_ysize);
  glGetIntegerv(GL_VIEWPORT, m_viewport);

  glViewport(0, 0, X()+m_xsize, Y()+m_ysize);
  m_aspectRatio = float(X()+m_xsize) / float(Y()+m_ysize);
  m_fov = 60.0;
  Reshape();

  // Set OpenGL state
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glDisable(GL_DEPTH_TEST);
  glFrontFace(GL_CCW);
  glEnable(GL_CULL_FACE);

  // Initialize data structures
  m_flare.Init();
  if (m_settings.dSmoke)
    m_smoke.Init();
  m_world.Init();
  m_shockwave.Init();
  if (m_settings.dSound)
    m_soundengine = new CSoundEngine(float(m_settings.dSound) * 0.01f);

  glGenVertexArrays(1, &m_vao);

  glGenBuffers(1, &m_vertexVBO);

  // Change rocket firing rate
  m_rocketTimer = 0.0f;
  // a rocket usually lasts about 10 seconds, so fastest rate is all rockets within 10 seconds
  m_rocketTimeConst = 10.0f / float(m_settings.dMaxrockets);
  m_changeRocketTimeConst = 20.0f;

  m_superFast = rsRandi(1000);
  m_ambientlight = float(m_settings.dAmbient) * 0.01f;
  m_first = true;
  m_lastTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  m_startOK = true;

  return true;
}

void CScreensaverSkyRocket::Stop()
{
  m_startOK = false;

  glDeleteBuffers(1, &m_vertexVBO);
  m_vertexVBO = 0;

  glDeleteVertexArrays(1, &m_vao);

  // Kodi defaults
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  // Free memory
  m_particles.clear();

  // clean up sound data structures
  if (m_settings.dSound)
    delete m_soundengine;
}

void CScreensaverSkyRocket::Render()
{
  if (!m_startOK)
    return;

  /*
   * Following Extra work done here in render to prevent problems with controls
   * from Kodi and during window moving.
   * TODO: Maybe add a separate interface call to inform about?
   */
  //@{
  glBindVertexArray(m_vao);

  glDisable(GL_DEPTH_TEST);
  glFrontFace(GL_CCW);
  glEnable(GL_CULL_FACE);

  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);

  glVertexAttribPointer(m_hVertex, 4, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, vertex)));
  glEnableVertexAttribArray(m_hVertex);

  glVertexAttribPointer(m_hColor, 4, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, color)));
  glEnableVertexAttribArray(m_hColor);

  glVertexAttribPointer(m_hCoord, 2, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, coord)));
  glEnableVertexAttribArray(m_hCoord);
  //@}

  double currentTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  m_frameTime = currentTime - m_lastTime;
  m_lastTime = currentTime;

  // super fast easter egg
  if (!m_superFast)
    m_frameTime *= 5.0f;

  ////////////////////////////////
  // update camera
  ///////////////////////////////
  if (m_first)
  {
    RandomLookFrom(1);  // new target position
    RandomLookAt(1);
    // starting camera view is very far away
    m_lookFrom[2] = rsVec(rsRandf(1000.0f) + 6000.0f, 5.0f, rsRandf(4000.0f) - 2000.0f);
    RandomLookAt(2);
    m_first = false;
  }

  // Make new random camera view
  if (m_settings.kNewCamera)
  {
    m_cameraTime[0] = rsRandf(25.0f) + 5.0f;
    m_cameraTime[1] = 0.0f;
    m_cameraTime[2] = 0.0f;
    // choose new positions
    RandomLookFrom(1);  // new target position
    RandomLookAt(1);  // new target position
    // cut to a new view
    RandomLookFrom(2);  // new last position
    RandomLookAt(2);  // new last position
    m_settings.kNewCamera = 0;
  }

  // Update the camera if it is active
  if (m_settings.kCamera == 1)
  {
    if (m_lastCameraMode == 2)  // camera was controlled by mouse last frame
    {
      m_cameraTime[0] = 10.0f;
      m_cameraTime[1] = 0.0f;
      m_cameraTime[2] = 0.0f;
      m_lookFrom[2] = m_lookFrom[0];
      RandomLookFrom(1);  // new target position
      m_lookAt[2] = m_lookAt[0];
      RandomLookAt(1);  // new target position
    }
    m_cameraTime[1] += m_frameTime;
    m_cameraTime[2] = m_cameraTime[1] / m_cameraTime[0];
    if (m_cameraTime[2] >= 1.0f)  // reset camera sequence
    {
      // reset timer
      m_cameraTime[0] = rsRandf(25.0f) + 5.0f;
      m_cameraTime[1] = 0.0f;
      m_cameraTime[2] = 0.0f;
      // choose new positions
      m_lookFrom[2] = m_lookFrom[1];  // last = target
      RandomLookFrom(1);  // new target position
      m_lookAt[2] = m_lookAt[1];  // last = target
      RandomLookAt(1);  // new target position
      if (!rsRandi(4) && m_zoom == 0.0f)  // possibly cut to new view if camera isn't zoomed in
      {
        RandomLookFrom(2);  // new last position
        RandomLookAt(2);
      }
    }
    // change camera position and angle
    float cameraStep = 0.5f * (1.0f - cosf(m_cameraTime[2] * PI));
    m_lookFrom[0] = m_lookFrom[2] + ((m_lookFrom[1] - m_lookFrom[2]) * cameraStep);
    m_lookAt[0] = m_lookAt[2] + ((m_lookAt[1] - m_lookAt[2]) * cameraStep);
    // update variables used for sound and lens flares
    m_cameraVel = m_lookFrom[0] - m_cameraPos;
    m_cameraPos = m_lookFrom[0];
    // find heading and pitch
    FindHeadingAndPitch(m_lookFrom[0], m_lookAt[0], m_headings, m_pitch);

    // zoom in on rockets with camera
    m_zoomTime[0] -= m_frameTime;
    if (m_zoomTime[0] < 0.0f)
    {
      if (m_zoomRocket == ZOOMROCKETINACTIVE)  // try to find a rocket to follow
      {
        for (unsigned int i = 0; i < m_lastParticle; ++i)
        {
          if (m_particles[i].GetType() == ROCKET)
          {
            m_zoomRocket = i;
            if (m_particles[m_zoomRocket].GetTimeRemaining() > 4.0f)
            {
              m_zoomTime[1] = m_particles[m_zoomRocket].GetTimeRemaining();
              // get out of for loop if a suitable rocket has been found
              i = m_lastParticle;
            }
            else
              m_zoomRocket = ZOOMROCKETINACTIVE;
          }
        }
        if (m_zoomRocket == ZOOMROCKETINACTIVE)
          m_zoomTime[0] = 5.0f;
      }
      if (m_zoomRocket != ZOOMROCKETINACTIVE)  // zoom in on this rocket
      {
        m_zoom += m_frameTime * 0.5f;
        if (m_zoom > 1.0f)
          m_zoom = 1.0f;
        m_zoomTime[1] -= m_frameTime;
        float h, p;
        FindHeadingAndPitch(m_lookFrom[0], m_particles[m_zoomRocket].GetXYZ(), h, p);
        // Don't wrap around
        while(h - m_headings < -180.0f)
          h += 360.0f;
        while(h - m_headings > 180.0f)
          h -= 360.0f;
        while(m_zoomHeading - h < -180.0f)
          m_zoomHeading += 360.0f;
        while(m_zoomHeading - h > 180.0f)
          m_zoomHeading -= 360.0f;
        // Make zoomed heading and pitch follow rocket closely but not exactly.
        // It would look weird because the rockets wobble sometimes.
        m_zoomHeading += (h - m_zoomHeading) * 10.0f * m_frameTime;
        m_zoomPitch += (p - m_zoomPitch) * 5.0f * m_frameTime;
        // End zooming
        if (m_zoomTime[1] < 0.0f)
        {
          m_zoomRocket = ZOOMROCKETINACTIVE;
          // Zoom in again no later than 3 minutes from now
          m_zoomTime[0] = rsRandf(175.0f) + 5.0f;
        }
      }
    }
  }

  // Still counting down to zoom in on a rocket,
  // so keep zoomed out.
  if (m_zoomTime[0] > 0.0f)
  {
    m_zoom -= m_frameTime * 0.5f;
    if (m_zoom < 0.0f)
      m_zoom = 0.0f;
  }

  // Control camera with the mouse
  if (m_settings.kCamera == 2)
  {
    // find heading and pitch to compute rotation component of modelview matrix
    m_headings += 100.0f * m_frameTime * m_aspectRatio * float(m_centerx - mousex) / float(m_xsize);
    m_pitch += 100.0f * m_frameTime * float(m_centery - mousey) / float(m_ysize);
    if (m_headings > 180.0f)
      m_headings -= 360.0f;
    if (m_headings < -180.0f)
      m_headings += 360.0f;
    if (m_pitch > 90.0f)
      m_pitch = 90.0f;
    if (m_pitch < -90.0f)
      m_pitch = -90.0f;
//     if (mouseButtons & MK_LBUTTON)
//       mouseSpeed += 400.0f * m_frameTime;
//     if (mouseButtons & MK_RBUTTON)
//       mouseSpeed -= 400.0f * m_frameTime;
//     if ((mouseButtons & MK_MBUTTON) || ((mouseButtons & MK_LBUTTON) && (mouseButtons & MK_RBUTTON)))
//       mouseSpeed = 0.0f;
    if (mouseSpeed > 4000.0f)
      mouseSpeed = 4000.0f;
    if (mouseSpeed < -4000.0f)
      mouseSpeed = -4000.0f;
    // find lookFrom location to compute translation component of modelview matrix
    float ch = cosf(D2R * m_headings);
    float sh = sinf(D2R * m_headings);
    float cp = cosf(D2R * m_pitch);
    float sp = sinf(D2R * m_pitch);
    m_lookFrom[0][0] -= mouseSpeed * sh * cp * m_frameTime;
    m_lookFrom[0][1] += mouseSpeed * sp * m_frameTime;
    m_lookFrom[0][2] -= mouseSpeed * ch * cp * m_frameTime;
    m_cameraPos = m_lookFrom[0];
    // Calculate new lookAt position so that lens flares will be computed correctly
    // and so that transition back to autonomous camera mode is smooth
    m_lookAt[0][0] = m_lookFrom[0][0] - 500.0f * sh * cp;
    m_lookAt[0][1] = m_lookFrom[0][1] + 500.0f * sp;
    m_lookAt[0][2] = m_lookFrom[0][2] - 500.0f * ch * cp;
  }

  // Interpolate fov, heading, and pitch using zoom value
  // zoom of {0,1} maps to fov of {60,6}
  const float t(0.5f * (1.0f - cosf(M_PI * m_zoom)));
  m_fov = 60.0f - 54.0f * t;
  m_headings = m_zoomHeading * t + m_headings * (1.0f - t);
  m_pitch = m_zoomPitch * t + m_pitch * (1.0f - t);

  Reshape();

  // Build modelview matrix
  m_modelMat = glm::rotate(glm::mat4(1.0f), glm::radians(-m_pitch), glm::vec3(1, 0, 0));
  m_modelMat = glm::rotate(m_modelMat, glm::radians(-m_headings), glm::vec3(0, 1, 0));
  m_modelMat = glm::translate(m_modelMat, glm::vec3(-m_lookFrom[0][0], -m_lookFrom[0][1], -m_lookFrom[0][2]));

  // store this frame's camera mode for next frame
  m_lastCameraMode = m_settings.kCamera;
  // Update mouse idle time
  if (m_settings.kCamera == 2)
  {
    mouseIdleTime += m_frameTime;
    if (mouseIdleTime > 300.0f)  // return to autonomous camera mode after 5 minutes
      m_settings.kCamera = 1;
  }

  // update billboard rotation matrix for particles
  m_billboardMat = glm::rotate(glm::mat4(1.0f), glm::radians(m_headings), glm::vec3(0, 1, 0));
  m_billboardMat = glm::rotate(m_billboardMat, glm::radians(m_pitch), glm::vec3(1, 0, 0));

  // clear the screen
  glClear(GL_COLOR_BUFFER_BIT);

  // Slows fireworks, but not camera
  if (m_settings.kSlowMotion)
    m_frameTime *= 0.5f;

  // Make more particles if necessary (always keep 1000 extra).
  // Ordinarily, you would resize as needed during the update loop, probably in the
  // addParticle() function.  But that logic doesn't work with this particle system
  // because particles can spawn other particles.  resizing the vector, and, thus,
  // moving all particle addresses, doesn't work if you are in the middle of
  // updating a particle.
  const unsigned int size(m_particles.size());
  if (m_particles.size() - int(m_lastParticle) < 1000)
  {
    for (unsigned int i = 0; i < 1000; i++)
      m_particles.push_back(this);
  }

  // Pause the animation?
  if (m_settings.kFireworks)
  {
    // update world
    m_world.update(m_frameTime);

    // darken smoke
    for (unsigned int i = 0; i < m_lastParticle; ++i)
    {
      CParticle* darkener(&(m_particles[i]));
      if (darkener->GetType() == SMOKE)
        darkener->GetRGB()[0] = darkener->GetRGB()[1] = darkener->GetRGB()[2] = m_ambientlight;
    }

    m_changeRocketTimeConst -= m_frameTime;
    if (m_changeRocketTimeConst <= 0.0f)
    {
      float temp;
      if (rsRandi(10))
        temp = rsRandf(4.0f);
      else
        temp = 0.0f;
      m_rocketTimeConst = (temp * temp) + (10.0f / float(m_settings.dMaxrockets));
      m_changeRocketTimeConst = rsRandf(30.0f) + 10.0f;
    }
    // add new rocket to list
    m_rocketTimer -= m_frameTime;
    if ((m_rocketTimer <= 0.0f) || (m_settings.userDefinedExplosion >= 0))
    {
      if (m_numRockets < m_settings.dMaxrockets)
      {
        CParticle* rock = AddParticle();
        if (rsRandi(30) || (m_settings.userDefinedExplosion >= 0))  // Usually launch a rocket
        {
          rock->initRocket();
          if (m_settings.userDefinedExplosion >= 0)
            rock->GetExplosionType() = m_settings.userDefinedExplosion;
          else
          {
            if (!rsRandi(2500))  // big ones!
            {
              if (rsRandi(2))
                rock->GetExplosionType() = 19;  // sucker and shockwave
              else
                rock->GetExplosionType() = 20;  // stretcher and bigmama
            }
            else
            {
              // Distribution of regular explosions
              if (rsRandi(2))  // 0 - 2 (all types of spheres)
              {
                if (!rsRandi(10))
                  rock->GetExplosionType() = 2;
                else
                  rock->GetExplosionType() = rsRandi(2);
              }
              else
              {
                if (!rsRandi(3))  //  ring, double sphere, sphere and ring
                  rock->GetExplosionType() = rsRandi(3) + 3;
                else
                {
                  if (rsRandi(2))  // 6, 7, 8, 9, 10, 11
                  {
                    if (rsRandi(2))
                      rock->GetExplosionType() = rsRandi(2) + 6;
                    else
                      rock->GetExplosionType() = rsRandi(4) + 8;
                  }
                  else
                  {
                    if (rsRandi(2))  // 12, 13, 14
                      rock->GetExplosionType() = rsRandi(3) + 12;
                    else  // 15 - 18
                      rock->GetExplosionType() = rsRandi(4) + 15;
                  }
                }
              }
            }
          }
          m_numRockets++;
        }
        else  // sometimes make fountains instead of rockets
        {
          rock->initFountain();
          int num_fountains = rsRandi(3);
          for (int i = 0; i < num_fountains; i++)
          {
            rock = AddParticle();
            rock->initFountain();
          }
        }
      }
      if (m_settings.dMaxrockets)
        m_rocketTimer = rsRandf(m_rocketTimeConst);
      else
        m_rocketTimer = 60.0f;  // arbitrary number since no rockets ever fire
      if (m_settings.userDefinedExplosion >= 0)
      {
        m_settings.userDefinedExplosion = -1;
        m_rocketTimer = 20.0f;  // Wait 20 seconds after user launches a rocket before launching any more
      }
    }

    // update particles
    m_numRockets = 0;
    for (unsigned int i = 0; i < m_lastParticle; i++)
    {
      CParticle* curpart(&(m_particles[i]));
      m_particles[i].update();
      if (curpart->GetType() == ROCKET)
        m_numRockets++;
        curpart->findDepth();
      if (curpart->GetLifeRemaining() <= 0.0f || curpart->GetXYZ()[1] < 0.0f)
      {
        switch(curpart->GetType())
        {
        case ROCKET:
          if (curpart->GetXYZ()[1] <= 0.0f)
          {
            // move above ground for explosion so new particles aren't removed
            curpart->GetXYZ()[1] = 0.1f;
            curpart->GetVelocityVector()[1] *= -0.7f;
          }
          if (curpart->GetExplosionType() == 18)
            curpart->initSpinner();
          else
            curpart->initExplosion();
          break;
        case POPPER:
          switch(curpart->GetExplosionType())
          {
          case STAR:
            curpart->GetExplosionType() = 100;
            curpart->initExplosion();
            break;
          case STREAMER:
            curpart->GetExplosionType() = 101;
            curpart->initExplosion();
            break;
          case METEOR:
            curpart->GetExplosionType() = 102;
            curpart->initExplosion();
            break;
          case POPPER:
            curpart->GetType() = STAR;
            curpart->GetRGB().set(1.0f, 0.8f, 0.6f);
            curpart->GetTimeTotal() = m_particles[i].GetTimeRemaining() = m_particles[i].GetLifeRemaining() = 0.2f;
          }
          break;
        case SUCKER:
          curpart->initShockwave();
          break;
        case STRETCHER:
          curpart->initBigmama();
        }
      }
    }

    // remove particles from list
    for (unsigned int i = 0; i < m_lastParticle; i++)
    {
      CParticle* curpart(&(m_particles[i]));
      if (curpart->GetLifeRemaining() <= 0.0f || curpart->GetXYZ()[1] < 0.0f)
        RemoveParticle(i);
    }

    SortParticles();
  }  // m_settings.kFireworks
  else
  {
    // Only sort particles if they're not being updated (the camera could still be moving)
    for (unsigned int i = 0; i < m_lastParticle; i++)
      m_particles[i].findDepth();
    SortParticles();
  }

  // the world
  m_world.draw();

  // draw particles
  glEnable(GL_BLEND);
  for (unsigned int i = 0; i < m_lastParticle; i++)
    m_particles[i].draw();

  // draw lens flares
  if (m_settings.dFlare)
  {
    MakeFlareList();
    for (int i = 0; i < m_numFlares; ++i)
    {
      m_flare.Flare(m_lensFlares[i].x, m_lensFlares[i].y, m_lensFlares[i].r,
                    m_lensFlares[i].g, m_lensFlares[i].b, m_lensFlares[i].a);
    }
    m_numFlares = 0;
  }

  // do sound stuff
  if (m_soundengine)
  {
    float listenerOri[6];
    listenerOri[0] = float(-glm::value_ptr(m_modelMat)[2]);
    listenerOri[1] = float(-glm::value_ptr(m_modelMat)[6]);
    listenerOri[2] = float(-glm::value_ptr(m_modelMat)[10]);
    listenerOri[3] = float(glm::value_ptr(m_modelMat)[1]);
    listenerOri[4] = float(glm::value_ptr(m_modelMat)[5]);
    listenerOri[5] = float(glm::value_ptr(m_modelMat)[9]);
    m_soundengine->update(m_cameraPos.v, m_cameraVel.v, listenerOri, m_frameTime, m_settings.kSlowMotion);
  }

  glDisableVertexAttribArray(m_hVertex);
  glDisableVertexAttribArray(m_hColor);
  glDisableVertexAttribArray(m_hCoord);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindVertexArray(0);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
}

void CScreensaverSkyRocket::Reshape()
{
  // build viewing matrix
  if (m_aspectRatio > 1.0f)
  {
    m_hFov = 2.0f * RS_RAD2DEG * atanf(tanf(m_fov * 0.5f / RS_RAD2DEG) * m_aspectRatio);
    m_projMat = glm::perspective(glm::radians(m_fov), m_aspectRatio, 1.0f, 40000.0f);
  }
  else
  {
    m_hFov = m_fov;
    m_projMat = glm::perspective(glm::radians(2.0f * RS_RAD2DEG * atanf(tanf(m_fov * 0.5f / RS_RAD2DEG) / m_aspectRatio)), m_aspectRatio, 1.0f, 40000.0f);
  }
}

CParticle* CScreensaverSkyRocket::AddParticle()
{
  // Advance to new particle if there is another in the vector.
  // Otherwise, just overwrite the last particle (this will probably never happen)
  if (m_lastParticle < m_particles.size())
    ++m_lastParticle;

  // Return pointer to new particle
  return &(m_particles[m_lastParticle-1]);
}

void CScreensaverSkyRocket::RemoveParticle(unsigned int rempart)
{
  // copy last particle over particle to be removed
  --m_lastParticle;
  if (rempart != m_lastParticle)
    m_particles[rempart] = m_particles[m_lastParticle];

  // correct zoomRocket index if necessary
  if (m_zoomRocket == m_lastParticle)
    m_zoomRocket = rempart;
}

void CScreensaverSkyRocket::SortParticles()
{
  // Sorting doesn't appear to be necessary.  Skyrocket still looks good without it.
}

// Rockets and explosions illuminate smoke
// Only explosions illuminate clouds
void CScreensaverSkyRocket::Illuminate(CParticle* ill)
{
  float temp;
  // desaturate illumination colors
  glm::vec3 newrgb(ill->GetRGB()[0] * 0.6f + 0.4f, ill->GetRGB()[1] * 0.6f + 0.4f, ill->GetRGB()[2] * 0.6f + 0.4f);

  // Smoke illumination
  if ((ill->GetType() == ROCKET) || (ill->GetType() == FOUNTAIN))
  {
    float distsquared;
    for (unsigned int i = 0; i < m_lastParticle; ++i)
    {
      CParticle* smk(&(m_particles[i]));
      if (smk->GetType() == SMOKE)
      {
        distsquared = (ill->GetXYZ()[0] - smk->GetXYZ()[0]) * (ill->GetXYZ()[0] - smk->GetXYZ()[0])
          + (ill->GetXYZ()[1] - smk->GetXYZ()[1]) * (ill->GetXYZ()[1] - smk->GetXYZ()[1])
          + (ill->GetXYZ()[2] - smk->GetXYZ()[2]) * (ill->GetXYZ()[2] - smk->GetXYZ()[2]);
        if (distsquared < 40000.0f)
        {
          temp = (40000.0f - distsquared) * 0.000025f;
          temp = temp * temp * ill->GetBright();
          smk->GetRGB()[0] += temp * newrgb.r;
          if (smk->GetRGB()[0] > 1.0f)
            smk->GetRGB()[0] = 1.0f;
          smk->GetRGB()[1] += temp * newrgb.g;
          if (smk->GetRGB()[1] > 1.0f)
            smk->GetRGB()[1] = 1.0f;
          smk->GetRGB()[2] += temp * newrgb.b;
          if (smk->GetRGB()[2] > 1.0f)
            smk->GetRGB()[2] = 1.0f;
        }
      }
    }
  }
  if (ill->GetType() == EXPLOSION)
  {
    float distsquared;
    for (unsigned int i = 0; i < m_lastParticle; ++i)
    {
      CParticle* smk(&(m_particles[i]));
      if (smk->GetType() == SMOKE)
      {
        distsquared = (ill->GetXYZ()[0] - smk->GetXYZ()[0]) * (ill->GetXYZ()[0] - smk->GetXYZ()[0])
          + (ill->GetXYZ()[1] - smk->GetXYZ()[1]) * (ill->GetXYZ()[1] - smk->GetXYZ()[1])
          + (ill->GetXYZ()[2] - smk->GetXYZ()[2]) * (ill->GetXYZ()[2] - smk->GetXYZ()[2]);
        if (distsquared < 640000.0f)
        {
          temp = (640000.0f - distsquared) * 0.0000015625f;
          temp = temp * temp * ill->GetBright();
          smk->GetRGB()[0] += temp * newrgb.r;
          if (smk->GetRGB()[0] > 1.0f)
            smk->GetRGB()[0] = 1.0f;
          smk->GetRGB()[1] += temp * newrgb.g;
          if (smk->GetRGB()[1] > 1.0f)
            smk->GetRGB()[1] = 1.0f;
          smk->GetRGB()[2] += temp * newrgb.b;
          if (smk->GetRGB()[2] > 1.0f)
            smk->GetRGB()[2] = 1.0f;
        }
      }
    }
  }

  // cloud illumination
  if (ill->GetType() == EXPLOSION && m_settings.dClouds)
  {
    int north, south, west, east;  // limits of cloud indices to inspect
    int halfmesh = CLOUDMESH / 2;
    float distsquared;
    // remember clouds have 20000-foot radius from the World class, hence 0.00005
    // Hardcoded values like this are evil, but oh well
    south = int((ill->GetXYZ()[2] - 1600.0f) * 0.00005f * float(halfmesh)) + halfmesh;
    north = int((ill->GetXYZ()[2] + 1600.0f) * 0.00005f * float(halfmesh) + 0.5f) + halfmesh;
    west = int((ill->GetXYZ()[0] - 1600.0f) * 0.00005f * float(halfmesh)) + halfmesh;
    east = int((ill->GetXYZ()[0] + 1600.0f) * 0.00005f * float(halfmesh) + 0.5f) + halfmesh;
    // bound these values just in case
    if (south < 0) south = 0; if (south > CLOUDMESH-1) south = CLOUDMESH-1;
    if (north < 0) north = 0; if (north > CLOUDMESH-1) north = CLOUDMESH-1;
    if (west < 0) west = 0; if (west > CLOUDMESH-1) west = CLOUDMESH-1;
    if (east < 0) east = 0; if (east > CLOUDMESH-1) east = CLOUDMESH-1;
    //do any necessary cloud illumination
    for (int i = west; i <= east; i++)
    {
      for (int j = south; j <= north; j++)
      {
        distsquared = (m_world.m_clouds[i][j][0] - ill->GetXYZ()[0]) * (m_world.m_clouds[i][j][0] - ill->GetXYZ()[0])
          + (m_world.m_clouds[i][j][1] - ill->GetXYZ()[1]) * (m_world.m_clouds[i][j][1] - ill->GetXYZ()[1])
          + (m_world.m_clouds[i][j][2] - ill->GetXYZ()[2]) * (m_world.m_clouds[i][j][2] - ill->GetXYZ()[2]);
        if (distsquared < 2560000.0f)
        {
          temp = (2560000.0f - distsquared) * 0.000000390625f;
          temp = temp * temp * ill->GetBright();
          m_world.m_clouds[i][j][6] += temp * newrgb.r;
          if (m_world.m_clouds[i][j][6] > 1.0f)
            m_world.m_clouds[i][j][6] = 1.0f;
          m_world.m_clouds[i][j][7] += temp * newrgb.g;
          if (m_world.m_clouds[i][j][7] > 1.0f)
            m_world.m_clouds[i][j][7] = 1.0f;
          m_world.m_clouds[i][j][8] += temp * newrgb.b;
          if (m_world.m_clouds[i][j][8] > 1.0f)
            m_world.m_clouds[i][j][8] = 1.0f;
        }
      }
    }
  }
}

// pulling of other particles
void CScreensaverSkyRocket::Pulling(CParticle* suck)
{
  rsVec diff;
  float pulldistsquared;
  float pullconst = (1.0f - suck->GetLifeRemaining()) * 0.01f * m_frameTime;

  for (unsigned int i = 0; i < m_lastParticle; ++i)
  {
    CParticle* puller(&(m_particles[i]));
    diff = suck->GetXYZ() - puller->GetXYZ();
    pulldistsquared = diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2];
    if (pulldistsquared < 250000.0f && pulldistsquared != 0.0f)
    {
      if (puller->GetType() != SUCKER && puller->GetType() != STRETCHER
        && puller->GetType() != SHOCKWAVE && puller->GetType() != BIGMAMA)
      {
        diff.normalize();
        puller->GetVelocityVector() += diff * ((250000.0f - pulldistsquared) * pullconst);
      }
    }
  }
}

// pushing of other particles
void CScreensaverSkyRocket::Pushing(CParticle* shock)
{
  rsVec diff;
  float pushdistsquared;
  float pushconst = (1.0f - shock->GetLifeRemaining()) * 0.002f * m_frameTime;

  for (unsigned int i = 0; i < m_lastParticle; ++i)
  {
    CParticle* pusher(&(m_particles[i]));
    diff = pusher->GetXYZ() - shock->GetXYZ();
    pushdistsquared = diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2];
    if (pushdistsquared < 640000.0f && pushdistsquared != 0.0f)
    {
      if (pusher->GetType() != SUCKER && pusher->GetType() != STRETCHER
        && pusher->GetType() != SHOCKWAVE && pusher->GetType() != BIGMAMA)
      {
        diff.normalize();
        pusher->GetVelocityVector() += diff * ((640000.0f - pushdistsquared) * pushconst);
      }
    }
  }
}

// vertical stretching of other particles (x, z sucking; y pushing)
void CScreensaverSkyRocket::Stretching(CParticle* stretch)
{
  rsVec diff;
  float stretchdistsquared, temp;
  float stretchconst = (1.0f - stretch->GetLifeRemaining()) * 0.002f * m_frameTime;

  for (unsigned int i = 0; i < m_lastParticle; ++i)
  {
    CParticle* stretcher(&(m_particles[i]));
    diff = stretch->GetXYZ() - stretcher->GetXYZ();
    stretchdistsquared = diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2];
    if (stretchdistsquared < 640000.0f && stretchdistsquared != 0.0f && stretcher->GetType() != STRETCHER)
    {
      diff.normalize();
      temp = (640000.0f - stretchdistsquared) * stretchconst;
      stretcher->GetVelocityVector()[0] += diff[0] * temp * 5.0f;
      stretcher->GetVelocityVector()[1] -= diff[1] * temp;
      stretcher->GetVelocityVector()[2] += diff[2] * temp * 5.0f;
    }
  }
}

// Makes list of lens flares.  Must be a called even when action is paused
// because camera might still be moving.
void CScreensaverSkyRocket::MakeFlareList()
{
  rsVec cameraDir, partDir;
  const float shine(float(m_settings.dFlare) * 0.01f);

  cameraDir = m_lookAt[0] - m_lookFrom[0];
  cameraDir.normalize();
  for (unsigned int i = 0; i < m_lastParticle; ++i)
  {
    CParticle* curlight(&(m_particles[i]));
    if (curlight->GetType() == EXPLOSION || curlight->GetType() == SUCKER
     || curlight->GetType() == SHOCKWAVE || curlight->GetType() == STRETCHER
     || curlight->GetType() == BIGMAMA)
    {
      glm::vec3 win = glm::project(glm::vec3(curlight->GetXYZ()[0], curlight->GetXYZ()[1], curlight->GetXYZ()[2]), m_modelMat, m_projMat,
                                   glm::ivec4(m_viewport[0], m_viewport[1], m_viewport[2], m_viewport[3]));

      partDir = curlight->GetXYZ() - m_cameraPos;
      if (partDir.dot(cameraDir) > 1.0f)  // is light source in front of camera?
      {
        if (m_numFlares == m_lensFlares.size())
          m_lensFlares.resize(m_lensFlares.size() + 10);
        m_lensFlares[m_numFlares].x = (win.x / float(m_xsize)) * m_aspectRatio;
        m_lensFlares[m_numFlares].y = win.y / float(m_ysize);
        rsVec vec = curlight->GetXYZ() - m_cameraPos;  // find distance attenuation factor
        if (curlight->GetType() == EXPLOSION)
        {
          m_lensFlares[m_numFlares].r = curlight->GetRGB()[0];
          m_lensFlares[m_numFlares].g = curlight->GetRGB()[1];
          m_lensFlares[m_numFlares].b = curlight->GetRGB()[2];
          float distatten = (10000.0f - vec.length()) * 0.0001f;
          if (distatten < 0.0f)
            distatten = 0.0f;
          m_lensFlares[m_numFlares].a = curlight->GetBright() * shine * distatten;
        }
        else
        {
          m_lensFlares[m_numFlares].r = 1.0f;
          m_lensFlares[m_numFlares].g = 1.0f;
          m_lensFlares[m_numFlares].b = 1.0f;
          float distatten = (20000.0f - vec.length()) * 0.00005f;
          if (distatten < 0.0f)
            distatten = 0.0f;
          m_lensFlares[m_numFlares].a = curlight->GetBright() * 2.0f * shine * distatten;
        }
        m_numFlares++;
      }
    }
  }
}

void CScreensaverSkyRocket::RandomLookFrom(int n)
{
  m_lookFrom[n][0] = rsRandf(6000.0f) - 3000.0f;
  m_lookFrom[n][1] = rsRandf(1200.0f) + 5.0f;
  m_lookFrom[n][2] = rsRandf(6000.0f) - 3000.0f;
}

void CScreensaverSkyRocket::RandomLookAt(int n)
{
  // look left or right some amount within HFov.  This way, if there is a really
  // wide FOV due to a wide screen or multiple monitors, the action will appear off
  // to the sides sometimes.
  float shift_angle = (m_hFov * 0.5f) - 15.0f;
  if (shift_angle < 0.0f)
    shift_angle = 0.0f;
  const float shift = tanf(shift_angle / RS_RAD2DEG);
  const float shift_x = -m_lookFrom[n][2] * shift;
  const float shift_z = m_lookFrom[n][0] * shift;
  m_lookAt[n][0] = rsRandf(shift_x * 2.0f) - shift_x;
  m_lookAt[n][1] = rsRandf(800.0f) + 200.0f;
  m_lookAt[n][2] = rsRandf(shift_z * 2.0f) - shift_z;
}

void CScreensaverSkyRocket::FindHeadingAndPitch(rsVec lookFrom, rsVec lookAt, float& heading, float& pitch)
{
  const float diffx(lookAt[0] - lookFrom[0]);
  const float diffy(lookAt[1] - lookFrom[1]);
  const float diffz(lookAt[2] - lookFrom[2]);
  const float radius(sqrtf(diffx * diffx + diffz * diffz));
  pitch = R2D * atan2f(diffy, radius);
  heading = R2D * atan2f(-diffx, -diffz);
}

void CScreensaverSkyRocket::DrawEntry(int primitive, const sLight* data, unsigned int size)
{
  EnableShader();
  glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*size, data, GL_DYNAMIC_DRAW);
  glDrawArrays(primitive, 0, size);
  DisableShader();
}

void CScreensaverSkyRocket::OnCompiledAndLinked()
{
  // Variables passed directly to the Vertex shader
  m_projMatLoc = glGetUniformLocation(ProgramHandle(), "u_projectionMatrix");
  m_modelViewMatLoc = glGetUniformLocation(ProgramHandle(), "u_modelViewMatrix");
  m_textureIdLoc = glGetUniformLocation(ProgramHandle(), "u_textureId");

  m_hVertex = glGetAttribLocation(ProgramHandle(), "a_vertex");
  m_hColor = glGetAttribLocation(ProgramHandle(), "a_color");
  m_hCoord = glGetAttribLocation(ProgramHandle(), "a_coord");
}

bool CScreensaverSkyRocket::OnEnabled()
{
  // This is called after glUseProgram()
  glUniformMatrix4fv(m_projMatLoc, 1, GL_FALSE, glm::value_ptr(m_projMat));
  glUniformMatrix4fv(m_modelViewMatLoc, 1, GL_FALSE, glm::value_ptr(m_modelMat));
  glUniform1i(m_textureIdLoc, m_textureUsed);
  return true;
}

ADDONCREATOR(CScreensaverSkyRocket);
