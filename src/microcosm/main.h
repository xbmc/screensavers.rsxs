/*
 *  Copyright (C) 2005-2020 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2010 Terence M. Welsh
 *  Ported to Kodi by Alwin Esch <alwinus@kodi.tv>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

/*
 * Code is based on:
 *   https://github.com/reallyslickscreensavers/reallyslickscreensavers
 * and reworked to GL 4.0.
 */

#pragma once

#include <kodi/addon-instance/Screensaver.h>
#include <kodi/gui/gl/GL.h>
#include <kodi/gui/gl/Shader.h>
#include <glm/gtc/type_ptr.hpp>
#include <Implicit/impCubeVolume.h>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "light.h"
#include "gizmo.h"
#include "mirrorBox.h"
#include "rsCamera.h"

typedef enum eMicrocosmType
{
  TYPE_REGULAR = 0,
  TYPE_SIMPLE = 1,
  TYPE_HYPERSPACE = 2,
  TYPE_ADVANCED = -1
} eMicrocosmType;

// Parameters edited in the dialog box
struct sMicrocosmSettings
{
  sMicrocosmSettings()
  {
    SetDefaults(TYPE_REGULAR);
  }

  void SetDefaults(int which)
  {
    switch(which)
    {
    case TYPE_ADVANCED:
      kodi::CheckSettingInt("advanced.kaleidoscopetime", dKaleidoscopeTime);
      kodi::CheckSettingInt("advanced.singletime", dSingleTime);
      kodi::CheckSettingInt("advanced.background", dBackground);
      kodi::CheckSettingInt("advanced.resolution", dResolution);
      kodi::CheckSettingInt("advanced.depth", dDepth);
      kodi::CheckSettingInt("advanced.fov", dFov);
      kodi::CheckSettingInt("advanced.gizmospeed", dGizmoSpeed);
      kodi::CheckSettingInt("advanced.colorspeed", dColorSpeed);
      kodi::CheckSettingInt("advanced.cameraspeed", dCameraSpeed);
      kodi::CheckSettingBoolean("advanced.fog", dFog);
      break;
    case TYPE_SIMPLE:  // Simple
      dSingleTime = 60;
      dKaleidoscopeTime = 0;
      dBackground = 0;
      dResolution = 50;
      dDepth = 4;
      dFov = 60;
      dGizmoSpeed = 10;
      dColorSpeed = 10;
      dCameraSpeed = 10;
      dFog = true;
      break;
    case TYPE_HYPERSPACE:  // Hyperspace
      dSingleTime = 0;
      dKaleidoscopeTime = 120;
      dBackground = 25;
      dResolution = 40;
      dDepth = 5;
      dFov = 100;
      dGizmoSpeed = 20;
      dColorSpeed = 20;
      dCameraSpeed = 50;
      dFog = true;
      break;
    case TYPE_REGULAR:  // Regular
    default:
      dSingleTime = 60;
      dKaleidoscopeTime = 60;
      dBackground = 25;
      dResolution = 50;
      dDepth = 4;
      dFov = 60;
      dGizmoSpeed = 10;
      dColorSpeed = 10;
      dCameraSpeed = 10;
      dFog = true;
      // This is the sort of thing somebody sets once and never changes.  It's more of
      // a technical choice than a visual choice like the rest of the defaults.
      //dFrameRateLimit = 0;
      break;
    }
  }

  void Load()
  {
    int type = TYPE_REGULAR;
    kodi::CheckSettingInt("general.type", type);
    SetDefaults(type);

    if (type != TYPE_ADVANCED &&
        type != kodi::GetSettingInt("general.lastType"))
    {
      kodi::SetSettingInt("general.lastType", type);

      kodi::SetSettingInt("advanced.kaleidoscopetime", dKaleidoscopeTime);
      kodi::SetSettingInt("advanced.singletime", dSingleTime);
      kodi::SetSettingInt("advanced.background", dBackground);
      kodi::SetSettingInt("advanced.resolution", dResolution);
      kodi::SetSettingInt("advanced.depth", dDepth);
      kodi::SetSettingInt("advanced.fov", dFov);
      kodi::SetSettingInt("advanced.gizmospeed", dGizmoSpeed);
      kodi::SetSettingInt("advanced.colorspeed", dColorSpeed);
      kodi::SetSettingInt("advanced.cameraspeed", dCameraSpeed);
      kodi::SetSettingBoolean("advanced.fog", dFog);
    }
  }

  int dSingleTime;
  int dKaleidoscopeTime;
  int dBackground;
  int dResolution;
  int dDepth;
  int dFov;
  int dGizmoSpeed;
  int dColorSpeed;
  int dCameraSpeed;
  bool dFog;
};

class Gizmo;
class Texture1D;
class impCubeVolume;
class impSurface;

class ATTRIBUTE_HIDDEN CScreensaverMicrocosm
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver,
    public kodi::gui::gl::CShaderProgram
{
public:
  CScreensaverMicrocosm();
  ~CScreensaverMicrocosm() override;

  // kodi::addon::CInstanceScreensaver
  bool Start() override;
  void Stop() override;
  void Render() override;

  // kodi::gui::gl::CShaderProgram
  void OnCompiledAndLinked() override;
  bool OnEnabled() override;

  ATTRIBUTE_FORCEINLINE sMicrocosmSettings& Settings() { return m_settings; }
  ATTRIBUTE_FORCEINLINE int Mode() { return m_mode; }
  ATTRIBUTE_FORCEINLINE float FrameTime() { return m_frameTime; }
  ATTRIBUTE_FORCEINLINE rsCamera& Camera() { return m_camera; }
  ATTRIBUTE_FORCEINLINE impSurface* DrawSurface0() { return m_drawSurface0; }
  ATTRIBUTE_FORCEINLINE impSurface* DrawSurface1() { return m_drawSurface1; }
  ATTRIBUTE_FORCEINLINE impSurface* DrawSurface2() { return m_drawSurface2; }

  ATTRIBUTE_FORCEINLINE glm::mat4& ProjMatrix() { return m_projMat; }
  ATTRIBUTE_FORCEINLINE glm::mat4& ModelMatrix() { return m_modelMat; }

  void Draw(const float* vertices, unsigned int vertex_offset, const unsigned int* indices, unsigned int index_offset);

private:
  void chooseGizmo(int index = -1);

  static float surfaceFunction0(void* main, float* position); // function for mode 0: single gizmo
  static float surfaceFunctionTransition0(void* main, float* position); // ... and with transition
  static float surfaceFunction1(void* main, float* position); // function for mode 1: kaleidoscope
  static float surfaceFunctionTransition1(void* main, float* position); // ... and with transition

  void threadFunction0();
  void threadFunction1();

  glm::mat4 m_projMat;
  glm::mat4 m_modelMat;
  glm::mat3 m_normalMat;

  GLint m_projMatLoc = -1;
  GLint m_modelViewMatLoc = -1;
  GLint m_normalMatLoc = -1;
  GLint m_uTexCoordMix = -1;
  GLint m_light0_ambientLoc = -1;
  GLint m_light0_diffuseLoc = -1;
  GLint m_light0_specularLoc = -1;
  GLint m_light0_positionLoc = -1;
  GLint m_light1_ambientLoc = -1;
  GLint m_light1_diffuseLoc = -1;
  GLint m_light1_specularLoc = -1;
  GLint m_light1_positionLoc = -1;
  GLint m_fogEnabledLoc = -1;
  GLint m_fogColorLoc = -1;
  GLint m_fogStartLoc = -1;
  GLint m_fogEndLoc = -1;
  GLint m_hNormal = -1;
  GLint m_hVertex = -1;

  GLuint m_vertexVBO = 0;
  GLuint m_indexVBO = 0;

  rsCamera m_camera;
  MirrorBox m_mirrorbox;
  std::vector<Gizmo*> m_gizmos;
  Texture1D* m_tex1d;

  sMicrocosmSettings m_settings;
  impCrawlPointVector m_crawlpoints;
  impCubeVolume* m_volume0;
  impCubeVolume* m_volume1;
  impCubeVolume* m_volume2;
  // Double buffers so that each of 3 volumes can store into one surface
  // while the other is being used for drawing.
  impSurface* m_volSurface0[2];
  impSurface* m_volSurface1[2];
  impSurface* m_volSurface2[2];
  // Pointers to surfaces that can be used for drawing
  impSurface* m_drawSurface0;
  impSurface* m_drawSurface1;
  impSurface* m_drawSurface2;

  bool m_dimLightUsed = false;
  bool m_fogEnabled = false;
  float m_fogColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
  float m_fogStart = 100.0f;
  float m_fogEnd = 1000.0f;
  float m_shininess = 30.0f;
  float m_ambient[4] = {0.2f, 0.2f, 0.2f, 0.0f};
  float m_dimAmbient[4];
  float m_noAmbient[4] = {0.0f, 0.0f, 0.0f, 0.0f};
  float m_diffuse0[4] = {1.0f, 1.0f, 1.0f, 0.0f};
  float m_specular0[4] = {1.0f, 1.0f, 1.0f, 0.0f};
  float m_diffuse1[4] = {0.2f, 0.2f, 0.4f, 0.0f};
  float m_specular1[4] = {0.3f, 0.3f, 0.6f, 0.0f};
  float m_dimDiffuse0[4];
  float m_dimSpecular0[4];
  float m_dimDiffuse1[4];
  float m_dimSpecular1[4];
  float m_light0Position[4] = {1.0f, 1.0f, 1.0f, 0.0f};
  float m_light1Position[4] = {-1.0f, -1.0f, 0.0f, 0.0f};
  float m_tcmix[4];

  bool m_doingPreview = false; // Unused
  float m_aspectRatio;
  int m_specificGizmo = -1;
  int m_mode = 0; // 0 = single centered Gizmo; 1 = kaleidoscope mode
  float m_numberInputTimer = 0.0f;
  float m_hFov;
  float m_vFov;
  float m_modeTransition = 0.0f;
  float m_modeTransitionDir = 1.0f;
  float m_frameTime = 0.0f;
  double m_lastTime;
  float m_sfEyeX, m_sfEyeY, m_sfEyeZ; // eye coordinates for surface function
  unsigned int m_gizmoIndex = 0;
  ShapeVector m_shapes;
  unsigned int m_numShapes = 0;
  bool m_startOK = false;

  // easter egg
  // Tennis gizmo is not immediately available
  bool m_tennisAvailable = false;
  
  std::vector<sLight> m_surface;

  // multi-threading
  // Two worker threads are used to compute frame (n+1)'s surfaces while frame n
  // is being drawn.  This gives a bit of a performance advantage on multi-core
  // processors.
  // When the main thread is ready to draw on the screen, it signals the worker
  // threads to start computing surfaces.  Then when the main thread is done
  // drawing, it waits until the worker threads signal it that they are done.
  // That way the main thread does not restart the draw() function and change
  // the surface parameters until the worker threads are done using those parameters.
  bool m_useThreads = true;
  std::thread* m_thread0 = nullptr;
  std::thread* m_thread1 = nullptr;
//   pthread_t m_thread0;
//   pthread_t m_thread1;
// 
  // conditional variables
  std::condition_variable_any m_t0Start;
  std::condition_variable_any m_t0End;
  std::condition_variable_any m_t1Start;
  std::condition_variable_any m_t1End;
//   pthread_cond_t m_t0Start = PTHREAD_COND_INITIALIZER;  // for signaling thread 0 to start
//   pthread_cond_t m_t0End = PTHREAD_COND_INITIALIZER;  // for thread 0 to signal main thread that it is done
//   pthread_cond_t m_t1Start = PTHREAD_COND_INITIALIZER;
//   pthread_cond_t m_t1End = PTHREAD_COND_INITIALIZER;

  // each conditional variable requires a mutex
//   pthread_mutex_t m_t0StartMutex = PTHREAD_MUTEX_INITIALIZER;
//   pthread_mutex_t m_t0EndMutex = PTHREAD_MUTEX_INITIALIZER;
//   pthread_mutex_t m_t1StartMutex = PTHREAD_MUTEX_INITIALIZER;
//   pthread_mutex_t m_t1EndMutex = PTHREAD_MUTEX_INITIALIZER;
  std::mutex m_t0StartMutex;
  std::mutex m_t0EndMutex;
  std::mutex m_t1StartMutex;
  std::mutex m_t1EndMutex;
};
