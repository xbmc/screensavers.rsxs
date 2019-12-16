/*
 *  Copyright (C) 2005-2019 Team Kodi
 *  Copyright (C) 2010 Terence M. Welsh
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
 * and reworked to GL 4.0.
 */

#include "main.h"
#include "mirrorBox.h"
#include "brain.h"
#include "cube.h"
#include "cubesAndCapsules.h"
#include "cylinder.h"
#include "flower.h"
#include "grinder.h"
#include "knotAndSpheres.h"
#include "knotAndTorus.h"
#include "kube.h"
#include "kube2.h"
#include "kube3.h"
#include "kube4.h"
#include "metaballs.h"
#include "octahedron.h"
#include "orbit.h"
#include "ringOfTori.h"
#include "rings.h"
#include "spheresAndCapsules.h"
#include "stringOfEllipsoids.h"
#include "tennis.h"
#include "tetrahedron.h"
#include "torusBox.h"
#include "triangleOfSpheres.h"
#include "ufo.h"
#include "texture1d.h"

#include <algorithm>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <rsMath/rsMath.h>

CScreensaverMicrocosm::CScreensaverMicrocosm()
  : m_camera(this),
    m_mirrorbox(this)
{
}

CScreensaverMicrocosm::~CScreensaverMicrocosm()
{
}

bool CScreensaverMicrocosm::Start()
{
  std::string fraqShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/frag.glsl");
  std::string vertShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
    return false;

  m_settings.Load();

  glGenBuffers(1, &m_vertexVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
  glGenBuffers(1, &m_indexVBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);

  srand((unsigned)time(nullptr));

  glViewport(X(), Y(), Width(), Height());
  m_aspectRatio = float(Width()) / float(Height());

  // calculate fov and set up projection matrix
  const float fov = m_settings.dFov * RS_DEG2RAD;
  if (m_aspectRatio > 1.0f)
  {
    m_hFov = fov;
    m_vFov = 2.0f * atanf(tanf(fov * 0.5f) / m_aspectRatio);
  }
  else
  {
    m_hFov = 2.0f * atanf(tanf(fov * 0.5f) * m_aspectRatio);
    m_vFov = fov;
  }
  m_camera.setProjectionMatrix(m_vFov, m_aspectRatio, 0.01f, 50.0f);

  // Limit memory consumption because the Windows previewer is just too darn slow
  if (m_doingPreview)
  {
    m_settings.dResolution = 20;
    if (m_settings.dDepth > 2)
     m_settings.dDepth = 2;
  }

  m_modelMat = glm::mat4(1.0f);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  glFrontFace(GL_CCW);
  glEnable(GL_CULL_FACE);
#if !defined(HAS_GLES) && !defined(TARGET_DARWIN)
  glEnable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
#endif

  // pre-multiplied alpha.  Diffuse color must be multiplied by diffuse alpha in shader
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  //glEnable(GL_BLEND);

  for (int i = 0; i < 4; i++)
  {
    const float mult = (i==3) ? 1.0f : float(m_settings.dBackground) * 0.01f;
    m_dimAmbient[i] = m_ambient[i] * mult;
    m_dimDiffuse0[i] = m_diffuse0[i] * mult;
    m_dimSpecular0[i] = m_specular0[i] * mult;
    m_dimDiffuse1[i] = m_diffuse1[i] * mult;
    m_dimSpecular1[i] = m_specular1[i] * mult;
  }

  // initialize m_gizmos  m_gizmos.clear();
  { Metaballs* gizmo = new Metaballs(this, 7);  m_gizmos.push_back(gizmo); }
  { TriangleOfSpheres* gizmo = new TriangleOfSpheres(this, 5);  m_gizmos.push_back(gizmo); }
  { TriangleOfSpheres* gizmo = new TriangleOfSpheres(this, 7);  m_gizmos.push_back(gizmo); }
  { TriangleOfSpheres* gizmo = new TriangleOfSpheres(this, 8);  m_gizmos.push_back(gizmo); }
  { StringOfEllipsoids* gizmo = new StringOfEllipsoids(this, 5, 0.7f);  m_gizmos.push_back(gizmo); }
  { StringOfEllipsoids* gizmo = new StringOfEllipsoids(this, 6, 0.7f);  m_gizmos.push_back(gizmo); }
  { StringOfEllipsoids* gizmo = new StringOfEllipsoids(this, 7, 1.0f);  m_gizmos.push_back(gizmo); }
  { StringOfEllipsoids* gizmo = new StringOfEllipsoids(this, 8, 0.7f);  m_gizmos.push_back(gizmo); }
  { StringOfEllipsoids* gizmo = new StringOfEllipsoids(this, 8, 1.2f);  m_gizmos.push_back(gizmo); }
  { StringOfEllipsoids* gizmo = new StringOfEllipsoids(this, 10, 0.9f);  m_gizmos.push_back(gizmo); }
  { StringOfEllipsoids* gizmo = new StringOfEllipsoids(this, 10, 2.0f);  m_gizmos.push_back(gizmo); }
  { Brain* gizmo = new Brain(this, 3);  m_gizmos.push_back(gizmo); }
  { Brain* gizmo = new Brain(this, 4);  m_gizmos.push_back(gizmo); }
  { Brain* gizmo = new Brain(this, 5);  m_gizmos.push_back(gizmo); }
  { Brain* gizmo = new Brain(this, 6);  m_gizmos.push_back(gizmo); }
  { Flower* gizmo = new Flower(this, 10, 0.2f);  m_gizmos.push_back(gizmo); }
  { Flower* gizmo = new Flower(this, 14, 0.2f);  m_gizmos.push_back(gizmo); }
  { Flower* gizmo = new Flower(this, 14, 1.0f);  m_gizmos.push_back(gizmo); }
  { Flower* gizmo = new Flower(this, 18, 0.2f);  m_gizmos.push_back(gizmo); }
  { KnotAndSpheres* gizmo = new KnotAndSpheres(this, 2, 3, 3);  m_gizmos.push_back(gizmo); }
  { KnotAndSpheres* gizmo = new KnotAndSpheres(this, 2, 5, 3);  m_gizmos.push_back(gizmo); }
  { KnotAndSpheres* gizmo = new KnotAndSpheres(this, 3, 2, 2);  m_gizmos.push_back(gizmo); }
  { KnotAndSpheres* gizmo = new KnotAndSpheres(this, 3, 4, 2);  m_gizmos.push_back(gizmo); }
  { KnotAndSpheres* gizmo = new KnotAndSpheres(this, 3, 5, 2);  m_gizmos.push_back(gizmo); }
  { KnotAndTorus* gizmo = new KnotAndTorus(this, 2, 3);  m_gizmos.push_back(gizmo); }
  { KnotAndTorus* gizmo = new KnotAndTorus(this, 3, 2);  m_gizmos.push_back(gizmo); }
  { KnotAndTorus* gizmo = new KnotAndTorus(this, 3, 4);  m_gizmos.push_back(gizmo); }
  { Orbit* gizmo = new Orbit(this);  m_gizmos.push_back(gizmo); }
  { TorusBox* gizmo = new TorusBox(this);  m_gizmos.push_back(gizmo); }
  { RingOfTori* gizmo = new RingOfTori(this, 3);  m_gizmos.push_back(gizmo); }
  { RingOfTori* gizmo = new RingOfTori(this, 4);  m_gizmos.push_back(gizmo); }
  { RingOfTori* gizmo = new RingOfTori(this, 5);  m_gizmos.push_back(gizmo); }
  { SpheresAndCapsules* gizmo = new SpheresAndCapsules(this, 4);  m_gizmos.push_back(gizmo); }
  { SpheresAndCapsules* gizmo = new SpheresAndCapsules(this, 5);  m_gizmos.push_back(gizmo); }
  { SpheresAndCapsules* gizmo = new SpheresAndCapsules(this, 6);  m_gizmos.push_back(gizmo); }
  { SpheresAndCapsules* gizmo = new SpheresAndCapsules(this, 7);  m_gizmos.push_back(gizmo); }
  { CubesAndCapsules* gizmo = new CubesAndCapsules(this, 6, 0.6f);  m_gizmos.push_back(gizmo); }
  { CubesAndCapsules* gizmo = new CubesAndCapsules(this, 6, 1.0472f);  m_gizmos.push_back(gizmo); }
  { UFO* gizmo = new UFO(this, 7);  m_gizmos.push_back(gizmo); }
  { UFO* gizmo = new UFO(this, 10);  m_gizmos.push_back(gizmo); }
  { Rings* gizmo = new Rings(this, 5);  m_gizmos.push_back(gizmo); }
  { Rings* gizmo = new Rings(this, 6);  m_gizmos.push_back(gizmo); }
  { Rings* gizmo = new Rings(this, 7);  m_gizmos.push_back(gizmo); }
  { Grinder* gizmo = new Grinder(this, 4);  m_gizmos.push_back(gizmo); }
  { Grinder* gizmo = new Grinder(this, 5);  m_gizmos.push_back(gizmo); }
  { Grinder* gizmo = new Grinder(this, 6);  m_gizmos.push_back(gizmo); }  // slowest
  { Kube* gizmo = new Kube(this);  m_gizmos.push_back(gizmo); }
  { Kube2* gizmo = new Kube2(this);  m_gizmos.push_back(gizmo); }
  { Kube3* gizmo = new Kube3(this);  m_gizmos.push_back(gizmo); }
  { Kube4* gizmo = new Kube4(this);  m_gizmos.push_back(gizmo); }
  { Cube* gizmo = new Cube(this);  m_gizmos.push_back(gizmo); }
  { Cylinder* gizmo = new Cylinder(this);  m_gizmos.push_back(gizmo); }
  { Octahedron* gizmo = new Octahedron(this);  m_gizmos.push_back(gizmo); }  // 2nd slowest
  { Tetrahedron* gizmo = new Tetrahedron(this);  m_gizmos.push_back(gizmo); }
  { Tennis* gizmo = new Tennis(this);  m_gizmos.push_back(gizmo); }

  chooseGizmo();

  // initialize surfaces
  m_volSurface0[0] = new impSurface;
  m_volSurface0[1] = new impSurface;
  m_volSurface1[0] = new impSurface;
  m_volSurface1[1] = new impSurface;
  m_volSurface2[0] = new impSurface;
  m_volSurface2[1] = new impSurface;
  // Pointers to surfaces that can be used for drawing
  m_drawSurface0 = m_volSurface0[0];
  m_drawSurface1 = m_volSurface1[0];
  m_drawSurface2 = m_volSurface2[0];

  // initialize m_volumes
  m_volume0 = new impCubeVolume(this);
  m_volume0->init(m_settings.dResolution, m_settings.dResolution, m_settings.dResolution, 1.0f / float(m_settings.dResolution));
  m_volume0->useFastNormals(false);
  m_volume0->setCrawlFromSides(true);
  m_volume0->setSurface(m_volSurface0[0]);

  int v1res = m_settings.dResolution * 2 / 3;
  if (v1res < 18)
    v1res = 18;
  m_volume1 = new impCubeVolume(this);
  m_volume1->init(v1res, v1res, v1res, 1.0f / float(v1res));
  m_volume1->useFastNormals(false);
  m_volume1->setCrawlFromSides(true);
  m_volume1->setSurface(m_volSurface1[0]);

  int v2res = m_settings.dResolution / 3;
  if (v2res < 16)
    v2res = 16;
  m_volume2 = new impCubeVolume(this);
  m_volume2->init(v2res, v2res, v2res, 1.0f / float(v2res));
  m_volume2->useFastNormals(false);
  m_volume2->setCrawlFromSides(true);
  m_volume2->setSurface(m_volSurface2[0]);

  m_tex1d = new Texture1D(this);

  // which mode to start in
  if (m_settings.dKaleidoscopeTime > 0)
  {
    if (m_settings.dSingleTime > 0)
      m_mode = rsRandi(2);
    else
      m_mode = 1;
  }

  // test code for starting up in a specific mode with a specific gizmo
  //m_mode = 0;
  //chooseGizmo(0);

  m_lastTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  m_startOK = true;

  // threading
  if (m_useThreads)
  {
    // Conditional variables require their associated mutexes to start out locked
    m_t0EndMutex.lock();
    m_t1EndMutex.lock();

    // Create threads
    m_thread0 = new std::thread(&CScreensaverMicrocosm::threadFunction0, this);
    m_thread1 = new std::thread(&CScreensaverMicrocosm::threadFunction1, this);

    // Block until signal is received.  Mutex is unlocked while waiting.
    // This tells us that the threads have started successfully and that they have locked
    // the "start" mutexes.  If we don't wait here, the threads might not lock the "start"
    // mutexes in time and we'll get a deadlock in draw().
    m_t0End.wait(m_t0EndMutex);
    m_t1End.wait(m_t1EndMutex);
  }

  return true;
}

void CScreensaverMicrocosm::Stop()
{
  if (!m_startOK)
    return;

  m_startOK = false;

  // threading
  if (m_useThreads)
  {
    // Unblock thread0 to let it end
    m_t0StartMutex.lock();
    m_t0StartMutex.unlock();
    m_t0Start.notify_all();

    // Unblock thread1 to let it end
    m_t1StartMutex.lock();
    m_t1StartMutex.unlock();
    m_t1Start.notify_all();

    if (m_thread0->joinable())
      m_thread0->join();
    if (m_thread1->joinable())
      m_thread1->join();

    delete m_thread0;
    delete m_thread1;
  }

  for (const auto& gizmo : m_gizmos)
    delete gizmo;
  m_gizmos.clear();

  delete m_volSurface0[0];
  m_volSurface0[0] = nullptr;
  delete m_volSurface0[1];
  m_volSurface0[1] = nullptr;
  delete m_volSurface1[0];
  m_volSurface1[0] = nullptr;
  delete m_volSurface1[1];
  m_volSurface1[0] = nullptr;
  delete m_volSurface2[0];
  m_volSurface2[0] = nullptr;
  delete m_volSurface2[1];
  m_volSurface2[1] = nullptr;

  delete m_tex1d;
  m_tex1d = nullptr;
  delete m_volume0;
  m_volume0 = nullptr;
  delete m_volume1;
  m_volume1 = nullptr;
  delete m_volume2;
  m_volume2 = nullptr;

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &m_indexVBO);
  m_indexVBO = 0;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &m_vertexVBO);
  m_vertexVBO = 0;

  // Reset from addon changed GL values for Kodi's work (also done here to make
  // sure it is Kodi's default
#if !defined(HAS_GLES)
  glBindTexture(GL_TEXTURE_1D, 0);
#else
  glBindTexture(GL_TEXTURE_2D, 0);
#endif
  glDisable(GL_CULL_FACE);
#if !defined(HAS_GLES) && !defined(TARGET_DARWIN)
  glDisable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
#endif

  // Kodi defaults
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
}

void CScreensaverMicrocosm::Render()
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

  glVertexAttribPointer(m_hVertex, 4, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, vertex)));
  glEnableVertexAttribArray(m_hVertex);
  glVertexAttribPointer(m_hNormal, 4, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, normal)));
  glEnableVertexAttribArray(m_hNormal);

  // Set GL to addon needed parts
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
#if !defined(HAS_GLES) && !defined(TARGET_DARWIN)
  glEnable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
#endif

  // pre-multiplied alpha.  Diffuse color must be multiplied by diffuse alpha in shader
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  //glEnable(GL_BLEND);
  //@}

  double currentTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  m_frameTime = currentTime - m_lastTime;
  m_lastTime = currentTime;

  // count-down available time for pressing second digit
  if (m_numberInputTimer > 0.0f)
    m_numberInputTimer -= m_frameTime;

  // camera variables
  static float rotPhase[3] = {rsRandf(RS_PIx2), rsRandf(RS_PIx2), rsRandf(RS_PIx2)};
  static float rotRate[3] = {0.075f + rsRandf(0.05f), 0.075f + rsRandf(0.05f), 0.075f + rsRandf(0.05f)};
  static float rot[3];
  static rsVec cam0Start;
  static rsVec cam0End(rsRandf(2.0f) - 1.0f, rsRandf(2.0f) - 1.0f, -2.0f / tanf(0.5f * min(m_hFov, m_vFov)));
  static rsVec cam0Pos = cam0End;
  static rsMatrix cam0Background;
  static rsVec cam1Pos(0.1f, 0.2f, 0.3f);
  static float cam0t = 1.0f;

  // This is an Easter egg gizmo that shouldn't show up right when the saver
  // starts, so we'll make it available a bit later.
  static float easterEggTime = 0.0f;
  if (!m_tennisAvailable)
  {
    easterEggTime += m_frameTime;
    if (easterEggTime >= 1200.0f)
      m_tennisAvailable = true;
  }

  // countdown to transition
  static float transitionTime = 0.0f;
  transitionTime += m_frameTime;
  const float ttime = (m_mode == 0) ? float(m_settings.dSingleTime) : float(m_settings.dKaleidoscopeTime);
  if ((m_modeTransition > 1.0f && transitionTime > ttime) || m_specificGizmo >= 0)
  {
    if (m_modeTransition > 1.0f)
      m_modeTransition = 1.0f;
    m_modeTransitionDir = -1.0f;
    transitionTime = 0.0f;
  }

  // transition
  if (m_modeTransition <= 1.0f)
    m_modeTransition += m_modeTransitionDir * m_frameTime * 0.5f;
  if (m_modeTransition < 0.0f)
  {
    m_modeTransitionDir = 1.0f;
    m_modeTransition = 0.0f;
    // Switch modes if both are in use and we are not choosing a specific gizmo by keyboard press
    if (m_settings.dSingleTime > 0 && m_settings.dKaleidoscopeTime > 0 && m_specificGizmo < 0)
    {
      m_mode = (m_mode + 1) % 2;
      if (m_mode == 0)
        chooseGizmo();
      else
      {
        if (rsRandi(4) == 0)
        {
          rotPhase[0] = rotPhase[1] = rotPhase[2] = 0.0f;
          cam1Pos.set(0.0f, 0.0f, 0.0f);
        }
      }
    }
    else
    {
      if (m_specificGizmo >= 0){
        chooseGizmo(m_specificGizmo);
        m_specificGizmo = -1;
      }
      else
        chooseGizmo();
    }
  }

  // camera movement
  for (int i = 0; i < 3; i++)
  {
    rotPhase[i] += rotRate[i] * 0.1f * float(m_settings.dCameraSpeed) * m_frameTime;
    if (rotPhase[i] >= RS_PIx2)
      rotPhase[i] -= RS_PIx2;
    rot[i] += sinf(rotPhase[i]) * 0.012f * float(m_settings.dCameraSpeed) * m_frameTime;
  }

  rsMatrix rotMat, camMat;
  rotMat.makeRotate(rot[0], 1.0f, 0.0f, 0.0f);
  rotMat.rotate(rot[1], 0.0f, 1.0f, 0.0f);
  rotMat.rotate(rot[2], 0.0f, 0.0f, 1.0f);
  if (m_mode == 0)
  {
    cam0t += m_frameTime * 0.005f * float(m_settings.dCameraSpeed);
    // chooose new point to move to
    float x, y, z;
    z = -0.6f / tanf(0.5f * min(m_hFov, m_vFov));
    if (cam0t >= 1.0f)
    {
      cam0t -= 1.0f;
      cam0Start = cam0End;
      if (m_aspectRatio >= 1.0f)
      {
        const float drift = m_aspectRatio - 0.5f;
        x = rsRandf(drift) - drift * 0.5f;
        y = rsRandf(0.4f) - 0.2f;
      }
      else
      {
        const float drift = (1.0f / m_aspectRatio) - 0.5f;
        x = rsRandf(0.4f) - 0.2f;
        y = rsRandf(drift) - drift * 0.5f;
      }
      cam0End.set(x, y, z);
    }
    // move camera
    float t = 0.5f * (1.0f - cosf(RS_PI * cam0t));
    cam0Pos = cam0Start * (1.0f - t) + cam0End * t;
    camMat.makeTranslate(cam0Pos);
    camMat.preMult(rotMat);
    m_camera.setViewMatrix(camMat);
    // background matrix
    rsMatrix backgroundrotmat;
    backgroundrotmat.makeRotate(2.0f, 1.0f, 1.0f, 1.0f);
    cam0Background.makeTranslate(0.0f, 0.0f, -4.0f);
    cam0Background.preMult(rotMat);
    cam0Background.preMult(backgroundrotmat);
  }
  else
  {
    cam1Pos[0] -= rotMat[2] * 0.06f * float(m_settings.dCameraSpeed) * m_frameTime;
    cam1Pos[1] -= rotMat[6] * 0.06f * float(m_settings.dCameraSpeed) * m_frameTime;
    cam1Pos[2] -= rotMat[10] * 0.06f * float(m_settings.dCameraSpeed) * m_frameTime;
    if (cam1Pos[0] < -1.0f)
      cam1Pos[0] += 2.0f;
    if (cam1Pos[0] > 1.0f)
      cam1Pos[0] -= 2.0f;
    if (cam1Pos[1] < -1.0f)
      cam1Pos[1] += 2.0f;
    if (cam1Pos[1] > 1.0f)
      cam1Pos[1] -= 2.0f;
    if (cam1Pos[2] < -1.0f)
      cam1Pos[2] += 2.0f;
    if (cam1Pos[2] > 1.0f)
      cam1Pos[2] -= 2.0f;
    camMat.makeTranslate(-cam1Pos[0], -cam1Pos[1], -cam1Pos[2]);
    camMat.postMult(rotMat);
    m_camera.setViewMatrix(camMat);
  }

  // Breathing (suface value going up and down slightly)
  // Commented out because you really can't see the effect with everything else that's going on.
  // Also, it increases the chances of a surface getting accidentally clipped at the edge of an
  // impCubeVolume during single mode.  That would be ugly.
  /*static float breathe_phase = 0.0f;
  breathe_phase += m_frameTime;
  if (breathe_phase >= RS_PIx2)
    breathe_phase -= RS_PIx2;
  const float surface_value = 0.5f + 0.05f * cosf(breathe_phase);
  m_volume0->setSurfaceValue(surface_value);
  m_volume1->setSurfaceValue(surface_value);
  m_volume2->setSurfaceValue(surface_value);*/

  // store eye position values for surface function
  m_sfEyeX = fabsf(cam1Pos[0]) - 0.5f;
  m_sfEyeY = fabsf(cam1Pos[1]) - 0.5f;
  m_sfEyeZ = fabsf(cam1Pos[2]) - 0.5f;

  m_mirrorbox.update(m_frameTime);

  m_gizmos[m_gizmoIndex]->update(m_frameTime);

  // collect crawl points
  m_crawlpoints.clear();
  m_gizmos[m_gizmoIndex]->addCrawlPoints(m_crawlpoints);

  if (m_mode == 0)
  {
    if (m_modeTransition < 1.0f)
      m_volume0->function = surfaceFunctionTransition0;
    else
      m_volume0->function = surfaceFunction0;
    // m_volume1 and m_volume2 are not used in mode 0
  }
  else
  {
    if (m_modeTransition < 1.0f)
    {
      m_volume0->function = surfaceFunctionTransition1;
      m_volume1->function = surfaceFunctionTransition1;
      m_volume2->function = surfaceFunctionTransition1;
    }
    else
    {
      m_volume0->function = surfaceFunction1;
      m_volume1->function = surfaceFunction1;
      m_volume2->function = surfaceFunction1;
    }
  }

  // Signal the worker threads to create implicit surfaces
  static int whichsurface = 0;
  if (m_useThreads)
  {
    // Note that in this multithreaded mode, the comptute and draw happen simultaneously,
    // so what is being drawn by the main thread is always one frame behind the compute threads.
    // swap double-buffer impSurfaces
    m_drawSurface0 = m_volSurface0[whichsurface];
    m_drawSurface1 = m_volSurface1[whichsurface];
    m_drawSurface2 = m_volSurface2[whichsurface];
    whichsurface = (whichsurface + 1) % 2;
    m_volume0->setSurface(m_volSurface0[whichsurface]);
    m_volume1->setSurface(m_volSurface1[whichsurface]);
    m_volume2->setSurface(m_volSurface2[whichsurface]);

    // Block until thread0 is ready to be signaled again, then signal it.
    m_t0StartMutex.lock();
    m_t0StartMutex.unlock();
    m_t0Start.notify_all();
    if (m_mode == 1)  // only need low-LOD geometry for kaleidoscope mode
    {
      // Block until thread1 is ready to be signaled again, then signal it.
      m_t1StartMutex.lock();
      m_t1StartMutex.unlock();
      m_t1Start.notify_all();
    }
  }
  else
  {
    m_volume0->makeSurface(m_crawlpoints);
    // In kaleidoscope mode, also compute low-LOD surfaces.
    // Low-LOD surfaces take some load off the graphics card because it won't have to draw as many triangles.
    if (m_mode == 1)
    {
      m_volume1->makeSurface(m_crawlpoints);
      m_volume2->makeSurface(m_crawlpoints);
    }
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  m_camera.apply();

  // animate texture
  static float tcphase[3] = {rsRandf(RS_PIx2), rsRandf(RS_PIx2), rsRandf(RS_PIx2)};
  tcphase[0] += m_frameTime * 0.0181f * float(m_settings.dColorSpeed);  // oscillator for object space texcoord
  tcphase[1] += m_frameTime * 0.0221f * float(m_settings.dColorSpeed);  // oscillator for eye space texcoord
  tcphase[2] += m_frameTime * 0.0037f * float(m_settings.dColorSpeed);  // oscillator for moving between object and eye space texcoords
  for (int i = 0; i < 3; i++)
  {
    if (tcphase[i] >= RS_PIx2)
      tcphase[i] -= RS_PIx2;
  }
  m_tcmix[0] = sinf(tcphase[0]) * 0.5f + 0.5f;
  m_tcmix[1] = sinf(tcphase[1]) * 0.6f + 0.5f;
  m_tcmix[2] = sinf(tcphase[2]) + 0.5f;
  for (int i = 0; i < 3; i++)
  {
    if (m_tcmix[i] < 0.0f)
      m_tcmix[i] = 0.0f;
    if (m_tcmix[i] > 1.0f)
      m_tcmix[i] = 1.0f;
  }

#if !defined(HAS_GLES)
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
#else
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
#endif
  m_tex1d->update();

  if (m_mode == 0)
  {
    if (m_settings.dBackground)
    {
      // render gizmo as background
      m_dimLightUsed = true;

      if (m_settings.dFog)
      {
        m_fogEnabled = true;
        m_fogStart = 0.0f;
        m_fogEnd = 20.0f;
      }

      float fov = 10.0f;
      if (m_aspectRatio > 1.0f)
        fov /= m_aspectRatio;

      glm::mat4 projMatOld = m_projMat;
      glm::mat4 modelMatOld = m_modelMat;
      m_projMat = glm::perspective(glm::radians(fov), m_aspectRatio, 0.01f, 50.0f);
      m_modelMat = glm::mat4(cam0Background[0], cam0Background[1], cam0Background[2], cam0Background[3],
                             cam0Background[4], cam0Background[5], cam0Background[6], cam0Background[7],
                             cam0Background[8], cam0Background[9], cam0Background[10], cam0Background[11],
                             cam0Background[12], cam0Background[13], cam0Background[14], cam0Background[15]);

      m_drawSurface0->draw([&](bool compile, const float* vertices, unsigned int vertex_offset,
                              const unsigned int* indices, unsigned int index_offset)
      {
        Draw(vertices, vertex_offset, indices, index_offset);
      });

      m_modelMat = modelMatOld;
      m_projMat = projMatOld;
      glClear(GL_DEPTH_BUFFER_BIT);
    }

    // render gizmo normally
    m_dimLightUsed = false;
    m_drawSurface0->draw([&](bool compile, const float* vertices, unsigned int vertex_offset,
                             const unsigned int* indices, unsigned int index_offset)
    {
      Draw(vertices, vertex_offset, indices, index_offset);
    });
  }
  else
  {
#if USE_CLIP_PLANES
    glEnable(GL_CLIP_PLANE0);
    glEnable(GL_CLIP_PLANE1);
    glEnable(GL_CLIP_PLANE2);
    glEnable(GL_CLIP_PLANE3);
    glEnable(GL_CLIP_PLANE4);
    glEnable(GL_CLIP_PLANE5);
#endif

    if (m_settings.dFog)
    {
      m_fogEnabled = true;
      m_fogStart = float(m_settings.dDepth) * 0.667f;
      m_fogEnd = float(m_settings.dDepth) * 2.0f;
    }

    m_dimLightUsed = false;

    for (int k =- m_settings.dDepth; k <= m_settings.dDepth; k++)
    {
      for (int j =- m_settings.dDepth; j <= m_settings.dDepth; j++)
      {
        for (int i =- m_settings.dDepth; i <= m_settings.dDepth; i++)
        {
          const float x(float(2 * i));
          const float y(float(2 * j));
          const float z(float(2 * k));
          if (m_camera.inViewVolume(rsVec(x, y, z), 1.7320508f))
          {
            const float eye_x(x - cam1Pos[0]);
            const float eye_y(y - cam1Pos[1]);
            const float eye_z(z - cam1Pos[2]);
            m_mirrorbox.draw(x, y, z, eye_x, eye_y, eye_z);
          }
        }
      }
    }

#if USE_CLIP_PLANES
    glDisable(GL_CLIP_PLANE0);
    glDisable(GL_CLIP_PLANE1);
    glDisable(GL_CLIP_PLANE2);
    glDisable(GL_CLIP_PLANE3);
    glDisable(GL_CLIP_PLANE4);
    glDisable(GL_CLIP_PLANE5);
#endif
  }

  m_fogEnabled = false;

  m_camera.revoke();

  // Pause here until worker threads are finished.  This prevents draw() from starting over
  // and changing the surface parameters until the worker threads are done using them.
  if (m_useThreads)
  {
    // Block until signal is received.  Mutex is unlocked while waiting.
    if (m_mode == 1)
      m_t1End.wait(m_t1EndMutex);
    m_t0End.wait(m_t0EndMutex);
  }

  // Reset from addon changed GL values for Kodi's work
  glDisable(GL_CULL_FACE);
#if !defined(HAS_GLES)
  glDisable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
#endif
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

  // Kodi defaults
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);          // Turn Blending On
  glDisable(GL_DEPTH_TEST);

  glDisableVertexAttribArray(m_hNormal);
  glDisableVertexAttribArray(m_hVertex);
}

void CScreensaverMicrocosm::Draw(const float* vertices, unsigned int vertex_offset, const unsigned int* indices, unsigned int index_offset)
{
  m_normalMat = glm::transpose(glm::inverse(glm::mat3(m_modelMat)));

  int length = vertex_offset/6;
  m_surface.resize(length);
  for (int i = 0; i < length; ++i)
  {
    m_surface[i].normal.x = vertices[i*6+0];
    m_surface[i].normal.y = vertices[i*6+1];
    m_surface[i].normal.z = vertices[i*6+2];

    m_surface[i].vertex.x = vertices[i*6+3];
    m_surface[i].vertex.y = vertices[i*6+4];
    m_surface[i].vertex.z = vertices[i*6+5];
  }

  EnableShader();
  glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*length, m_surface.data(), GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_offset * sizeof(GLuint), &(indices[0]), GL_DYNAMIC_DRAW);
  glDrawElements(GL_TRIANGLES, index_offset, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  DisableShader();
}

void CScreensaverMicrocosm::OnCompiledAndLinked()
{
  // Variables passed directly to the Vertex shader
  m_projMatLoc = glGetUniformLocation(ProgramHandle(), "u_projectionMatrix");
  m_modelViewMatLoc = glGetUniformLocation(ProgramHandle(), "u_modelViewMatrix");
  m_uTexCoordMix = glGetUniformLocation(ProgramHandle(), "u_texCoordMix");
  m_normalMatLoc = glGetUniformLocation(ProgramHandle(), "u_transposeAdjointModelViewMatrix");

  m_light0_ambientLoc = glGetUniformLocation(ProgramHandle(), "u_light0.ambient");
  m_light0_diffuseLoc = glGetUniformLocation(ProgramHandle(), "u_light0.diffuse");
  m_light0_specularLoc = glGetUniformLocation(ProgramHandle(), "u_light0.specular");
  m_light0_positionLoc = glGetUniformLocation(ProgramHandle(), "u_light0.position");

  m_light1_ambientLoc = glGetUniformLocation(ProgramHandle(), "u_light1.ambient");
  m_light1_diffuseLoc = glGetUniformLocation(ProgramHandle(), "u_light1.diffuse");
  m_light1_specularLoc = glGetUniformLocation(ProgramHandle(), "u_light1.specular");
  m_light1_positionLoc = glGetUniformLocation(ProgramHandle(), "u_light1.position");

  m_fogEnabledLoc = glGetUniformLocation(ProgramHandle(), "u_fogEnabled");
  m_fogColorLoc = glGetUniformLocation(ProgramHandle(), "u_fogColor");
  m_fogStartLoc = glGetUniformLocation(ProgramHandle(), "u_fogStart");
  m_fogEndLoc = glGetUniformLocation(ProgramHandle(), "u_fogEnd");

  m_hNormal = glGetAttribLocation(ProgramHandle(), "a_normal");
  m_hVertex = glGetAttribLocation(ProgramHandle(), "a_position");
}

bool CScreensaverMicrocosm::OnEnabled()
{
  // This is called after glUseProgram()
  glUniformMatrix4fv(m_projMatLoc, 1, GL_FALSE, glm::value_ptr(m_projMat));
  glUniformMatrix4fv(m_modelViewMatLoc, 1, GL_FALSE, glm::value_ptr(m_modelMat));
  glUniformMatrix3fv(m_normalMatLoc, 1, GL_FALSE, glm::value_ptr(m_normalMat));
  glUniform3f(m_uTexCoordMix, m_tcmix[0], m_tcmix[1], m_tcmix[2]);

  if (m_dimLightUsed)
  {
    glUniform4f(m_light0_ambientLoc, m_dimAmbient[0], m_dimAmbient[1], m_dimAmbient[2], m_dimAmbient[3]);
    glUniform4f(m_light0_diffuseLoc, m_dimDiffuse0[0], m_dimDiffuse0[1], m_dimDiffuse0[2], m_dimDiffuse0[3]);
    glUniform4f(m_light0_specularLoc, m_dimSpecular0[0], m_dimSpecular0[1], m_dimSpecular0[2], m_dimSpecular0[3]);
    glUniform4f(m_light0_positionLoc, m_light0Position[0], m_light0Position[1], m_light0Position[2], m_light0Position[3]);

    glUniform4f(m_light1_ambientLoc, m_noAmbient[0], m_noAmbient[1], m_noAmbient[2], m_noAmbient[3]);
    glUniform4f(m_light1_diffuseLoc, m_dimDiffuse1[0], m_dimDiffuse1[1], m_dimDiffuse1[2], m_dimDiffuse1[3]);
    glUniform4f(m_light1_specularLoc, m_dimSpecular1[0], m_dimSpecular1[1], m_dimSpecular1[2], m_dimSpecular1[3]);
    glUniform4f(m_light1_positionLoc, m_light1Position[0], m_light1Position[1], m_light1Position[2], m_light1Position[3]);
  }
  else
  {
    glUniform4f(m_light0_ambientLoc, m_ambient[0], m_ambient[1], m_ambient[2], m_ambient[3]);
    glUniform4f(m_light0_diffuseLoc, m_diffuse0[0], m_diffuse0[1], m_diffuse0[2], m_diffuse0[3]);
    glUniform4f(m_light0_specularLoc, m_specular0[0], m_specular0[1], m_specular0[2], m_specular0[3]);
    glUniform4f(m_light0_positionLoc, m_light0Position[0], m_light0Position[1], m_light0Position[2], m_light0Position[3]);

    glUniform4f(m_light1_ambientLoc, m_noAmbient[0], m_noAmbient[1], m_noAmbient[2], m_noAmbient[3]);
    glUniform4f(m_light1_diffuseLoc, m_diffuse1[0], m_diffuse1[1], m_diffuse1[2], m_diffuse1[3]);
    glUniform4f(m_light1_specularLoc, m_specular1[0], m_specular1[1], m_specular1[2], m_specular1[3]);
    glUniform4f(m_light1_positionLoc, m_light1Position[0], m_light1Position[1], m_light1Position[2], m_light1Position[3]);
  }

  glUniform1i(m_fogEnabledLoc, m_fogEnabled);
  glUniform4f(m_fogColorLoc, m_fogColor[0], m_fogColor[1], m_fogColor[2], m_fogColor[3]);
  glUniform1f(m_fogStartLoc, m_fogStart);
  glUniform1f(m_fogEndLoc, m_fogEnd);

  return true;
}

void CScreensaverMicrocosm::chooseGizmo(int index/* = -1*/)
{
  if (index >= 0 && index < m_gizmos.size())  // choose specific Gizmo
    m_gizmoIndex = index;
  else
  {
    // choose a new Gizmo at random, making sure to not choose the previous one
    unsigned int oldgizmo = m_gizmoIndex;
    unsigned int range = m_gizmos.size();
    if (!m_tennisAvailable)
      range--;
    while(m_gizmoIndex == oldgizmo)
      m_gizmoIndex = rsRandi(range);
  }

  // collect shapes
  m_shapes.clear();
  m_gizmos[m_gizmoIndex]->getShapes(m_shapes);
  m_numShapes = m_shapes.size();
}

// function for mode 0: single gizmo
float CScreensaverMicrocosm::surfaceFunction0(void* main, float* position)
{
  CScreensaverMicrocosm* base = static_cast<CScreensaverMicrocosm*>(main);

  float value(0.0f);

  for (unsigned int i = 0; i < base->m_numShapes; ++i)
    value += base->m_shapes[i]->value(position);

  return value;
}

// ... and with transition
float CScreensaverMicrocosm::surfaceFunctionTransition0(void* main, float* position)
{
  CScreensaverMicrocosm* base = static_cast<CScreensaverMicrocosm*>(main);

  float value(0.0f);

  for (unsigned int i = 0; i < base->m_numShapes; ++i)
    value += base->m_shapes[i]->value(position);

  // transition
  float trans(((base->m_modeTransition - 0.5f) * 1.5f + position[0]) * 10.0f);
  trans = trans * trans * trans;
  if (trans <= -50.0f)
    value = 0.0f;
  else
    value += (trans < 0.0f) ? trans : 0.0f;

  return value;
}

// function for mode 1: kaleidoscope
float CScreensaverMicrocosm::surfaceFunction1(void* main, float* position)
{
  CScreensaverMicrocosm* base = static_cast<CScreensaverMicrocosm*>(main);

  float value(0.0f);

  for (unsigned int i = 0; i < base->m_numShapes; ++i)
    value += base->m_shapes[i]->value(position);

  // Lower the surface value near the viewpoint
  // to create a bubble around viewpoint.
  const float x(10.0f * (base->m_sfEyeX - position[0]));
  const float y(10.0f * (base->m_sfEyeY - position[1]));
  const float z(10.0f * (base->m_sfEyeZ - position[2]));
  const float hole((1.0f / (x * x + y * y + z * z)) - 1.0f);
  float hole_capped = hole;
  if (hole < 0.0f)
    hole_capped = 0.0f;
  value -= hole_capped * hole_capped;

  return value;
}

// ... and with transition
float CScreensaverMicrocosm::surfaceFunctionTransition1(void* main, float* position)
{
  CScreensaverMicrocosm* base = static_cast<CScreensaverMicrocosm*>(main);

  float value(0.0f);

  for (unsigned int i = 0; i < base->m_numShapes; ++i)
    value += base->m_shapes[i]->value(position);

  // Lower the surface value near the viewpoint
  // to create a bubble around viewpoint.
  const float x(10.0f * (base->m_sfEyeX - position[0]));
  const float y(10.0f * (base->m_sfEyeY - position[1]));
  const float z(10.0f * (base->m_sfEyeZ - position[2]));
  const float hole((1.0f / (x * x + y * y + z * z)) - 1.0f);
  float hole_capped = hole;
  if (hole < 0.0f)
    hole_capped = 0.0f;
  value -= hole_capped * hole_capped;

  // transition
  float trans(((base->m_modeTransition - 0.5f) * 1.5f + position[0]) * 10.0f);
  trans = trans * trans * trans;
  if (trans <= -50.0f)
    value = 0.0f;
  else
    value += (trans < 0.0f) ? trans : 0.0f;

  return value;
}

void CScreensaverMicrocosm::threadFunction0()
{
  // Conditional variables require their associated mutexes to start out locked
  m_t0StartMutex.lock();

  // Wait until main loop is signalable, then tell it that this thread has started
  m_t0EndMutex.lock();
  m_t0EndMutex.unlock();
  m_t0End.notify_all();

  while (m_startOK)
  {
    // Block until signal is received.  Mutex is unlocked while waiting.
    m_t0Start.wait(m_t0StartMutex);

    if (!m_startOK)
      break;

    // Compute surface
    m_volume0->makeSurface(m_crawlpoints);

    // Wait until main loop is signalable, then tell it that this computation is complete.
    m_t0EndMutex.lock();
    m_t0EndMutex.unlock();
    m_t0End.notify_all();
  }

  m_t0StartMutex.unlock();

  return;
}

void CScreensaverMicrocosm::threadFunction1()
{
  // Conditional variables require their associated mutexes to start out locked
  m_t1StartMutex.lock();

  // Wait until main loop is signalable, then tell it that this thread has started
  m_t1EndMutex.lock();
  m_t1EndMutex.unlock();
  m_t1End.notify_all();

  while (m_startOK)
  {
    // Block until signal is received.  Mutex is unlocked while waiting.
    m_t1Start.wait(m_t1StartMutex);

    if (!m_startOK)
      break;

    // Compute surfaces
    m_volume1->makeSurface(m_crawlpoints);
    m_volume2->makeSurface(m_crawlpoints);

    // Wait until main loop is signalable, then tell it that this computation is complete.
    m_t1EndMutex.lock();
    m_t1EndMutex.unlock();
    m_t1End.notify_all();
  }

  m_t1StartMutex.unlock();

  return;
}

ADDONCREATOR(CScreensaverMicrocosm);
