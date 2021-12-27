/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2002, 2009 Sören Sonnenburg <sonne@debian.org>
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

#include <chrono>

#include <rsMath/rsMath.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

namespace {

static float lorenz_a = 11.0f;
static float lorenz_b = 15.0f;
static float lorenz_c = 3.0f;
static float lorenz_dt = 0.02f;

static float mean[3] = { 0.0f, 0.0f, 0.0f };
static float simTime = 0.0f;
static float lastn0 = 0.0f;
static float flipn0 = 1.0f;
static float deltaflipn0 = 0.0f;

static inline float distance(float P0x, float P0y, float P0z,
                             float P1x, float P1y, float P1z)
{
  float x = P0x - P1x;
  float y = P0y - P1y;
  float z = P0z - P1z;
  return sqrt(x*x+y*y+z*z);
}

static inline void norm_one(float* x, float* y, float* z)
{
  float n = sqrt(((*x)*(*x))+((*y)*(*y))+((*z)*(*z)));
  (*x)/=n;
  (*y)/=n;
  (*z)/=n;
}

inline void calc_normal(float P0x, float P0y, float P0z,
                        float P1x, float P1y, float P1z,
                        float P2x, float P2y, float P2z,
                        float* Nx, float* Ny, float* Nz)
{
  (*Nx)= (P1y - P0y) * (P2z - P0z) - (P1z - P0z) * (P2y - P0y);
  (*Ny)= (P1z - P0z) * (P2x - P0x) - (P1x - P0x) * (P2z - P0z);
  (*Nz)= (P1x - P0x) * (P2y - P0y) - (P1y - P0y) * (P2x - P0x);
}

inline float angle(float P0x, float P0y, float P0z,
                   float P1x, float P1y, float P1z,
                   float P2x, float P2y, float P2z)
{
  return atan2(distance(P0x,P0y,P0z, P1x, P1y, P1z), distance(P0x,P0y,P0z, P2x, P2y, P2z));
}

void coords_at_time(float* from, float t, float* x, float* y, float* z)
{
  int u=(int) t;

  float s;
  s=(t-floor(t));
  *x=(1-s)*from[3*u]+s*from[3*(u+1)];
  *y=(1-s)*from[3*u+1]+s*from[3*(u+1)+1];
  *z=(1-s)*from[3*u+2]+s*from[3*(u+1)+2];
}

inline void normal_at_time(float* from, float t, float* x, float* y, float* z)
{
  float p1[3];
  float p2[3];
  float p3[3];
  coords_at_time(from, t, &p1[0], &p1[1], &p1[2]);
  coords_at_time(from, t-1, &p2[0], &p2[1], &p2[2]);
  coords_at_time(from, t+1, &p3[0], &p3[1], &p3[2]);
  calc_normal(p1[0], p1[1], p1[2],
      p2[0], p2[1], p2[2],
      p3[0], p3[1], p3[2],
      x,y,z);
}

inline float distance_to_line(float P0x, float P0y, float P0z,
    float P1x, float P1y, float P1z,
    float P2x, float P2y, float P2z)
{
  return distance(P0x,P0y,P0z, P1x,P1y,P1z)*sin(angle(P0x,P0y,P0z, P1x,P1y,P1z, P2x,P2y,P2z));
}

} /* namespace */

////////////////////////////////////////////////////////////////////////////
// Kodi has loaded us into memory, we should set our core values
// here and load any settings we may have from our config file
//
CScreensaverLorenz::CScreensaverLorenz()
{
  m_settings.num_precomputed_points = kodi::addon::GetSettingInt("num-points");
  m_settings.num_satellites = kodi::addon::GetSettingInt("num-satellites");
  m_settings.camera_speed = 0.01f * kodi::addon::GetSettingInt("camera-speed");
  m_settings.camera_angle = float(kodi::addon::GetSettingInt("camera-angle"));
  m_settings.line_width_attractor = kodi::addon::GetSettingInt("line-width");
  m_settings.line_width_satellites = kodi::addon::GetSettingInt("line-width-sat");
  m_settings.linear_cutoff = 0.01f * kodi::addon::GetSettingInt("line-cutoff");
}

bool CScreensaverLorenz::Start()
{
  m_startOK = false;

  // Initialize pseudorandom number generator
  srand((unsigned)time(nullptr));

  std::string fraqShader = kodi::addon::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/frag.glsl");
  std::string vertShader = kodi::addon::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
    return false;

  m_num_points = m_settings.num_precomputed_points;
  simTime = m_num_points / 3.0f;
  precompute_lorenz_array();
  reduce_points(m_num_points_max);
  init_satellites();
  init_line_strip();

  m_width = Width();
  m_height = Height();

  set_camera();
  glViewport(X(), Y(), Width()-X(), Height()-Y());

  glGenBuffers(1, &m_vertexVBO);

  m_lastTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();

  m_startOK = true;
  return true;
}

// Kodi tells us to stop the screensaver
// we should free any memory and release
// any resources we have created.
void CScreensaverLorenz::Stop()
{
  if (!m_startOK)
    return;

  m_startOK = false;

  delete m_lorenz_coords;
  delete m_lorenz_path;

  delete m_satellite_times;
  delete m_satellite_speeds;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &m_vertexVBO);
  m_vertexVBO = 0;
}

void CScreensaverLorenz::Render()
{
  if (!m_startOK)
    return;

  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);

  glVertexAttribPointer(m_hNormal, 3, GL_FLOAT, GL_TRUE, sizeof(sLatticeSegmentEntry), BUFFER_OFFSET(offsetof(sLatticeSegmentEntry, normal)));
  glEnableVertexAttribArray(m_hNormal);

  glVertexAttribPointer(m_hVertex, 3, GL_FLOAT, GL_TRUE, sizeof(sLatticeSegmentEntry), BUFFER_OFFSET(offsetof(sLatticeSegmentEntry, vertex)));
  glEnableVertexAttribArray(m_hVertex);

  glVertexAttribPointer(m_hColor, 4, GL_FLOAT, GL_TRUE, sizeof(sLatticeSegmentEntry), BUFFER_OFFSET(offsetof(sLatticeSegmentEntry, color)));
  glEnableVertexAttribArray(m_hColor);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);

  double currentTime = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
  m_frameTime = static_cast<float>(currentTime - m_lastTime);
  m_lastTime = currentTime;

  set_camera();
  display();

  simTime+=37*m_frameTime*m_settings.camera_speed*m_num_points/m_settings.num_precomputed_points;

  m_settings.camera_angle += 37 * m_frameTime * m_camera_angle_anim_speed;
  if (m_settings.camera_angle < m_camera_angle_anim[0] || m_settings.camera_angle > m_camera_angle_anim[1])
  {
    m_camera_angle_anim_speed *= -1;
    m_settings.camera_angle += m_camera_angle_anim_speed;
  }

  glDisable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ZERO);

  glDisableVertexAttribArray(m_hNormal);
  glDisableVertexAttribArray(m_hVertex);
  glDisableVertexAttribArray(m_hColor);
}

void CScreensaverLorenz::OnCompiledAndLinked()
{
  // Variables passed directly to the Vertex shader
  m_projMatLoc = glGetUniformLocation(ProgramHandle(), "u_projectionMatrix");
  m_modelViewMatLoc = glGetUniformLocation(ProgramHandle(), "u_modelViewMatrix");
  m_normalMatLoc = glGetUniformLocation(ProgramHandle(), "u_transposeAdjointModelViewMatrix");

  m_lightingLoc = glGetUniformLocation(ProgramHandle(), "u_lightingEnabled");
  m_uniformColorLoc = glGetUniformLocation(ProgramHandle(), "u_globalAmbientColor");
  m_light0_ambientLoc = glGetUniformLocation(ProgramHandle(), "u_light0.ambient");
  m_light0_diffuseLoc = glGetUniformLocation(ProgramHandle(), "u_light0.diffuse");
  m_light0_specularLoc = glGetUniformLocation(ProgramHandle(), "u_light0.specular");
  m_light0_positionLoc = glGetUniformLocation(ProgramHandle(), "u_light0.position");
  m_light0_constantAttenuationLoc = glGetUniformLocation(ProgramHandle(), "u_light0.constantAttenuation");
  m_light0_linearAttenuationLoc = glGetUniformLocation(ProgramHandle(), "u_light0.linearAttenuation");
  m_light0_quadraticAttenuationLoc = glGetUniformLocation(ProgramHandle(), "u_light0.quadraticAttenuation");
  m_light0_spotDirectionLoc = glGetUniformLocation(ProgramHandle(), "u_light0.spotDirection");
  m_light0_spotExponentLoc = glGetUniformLocation(ProgramHandle(), "u_light0.spotExponent");
  m_light0_spotCutoffAngleCosLoc = glGetUniformLocation(ProgramHandle(), "u_light0.spotCutoffAngleCos");
  m_material_ambientLoc = glGetUniformLocation(ProgramHandle(), "u_material.ambient");
  m_material_diffuseLoc = glGetUniformLocation(ProgramHandle(), "u_material.diffuse");
  m_material_specularLoc = glGetUniformLocation(ProgramHandle(), "u_material.specular");
  m_material_emissionLoc = glGetUniformLocation(ProgramHandle(), "u_material.emission");
  m_material_shininessLoc = glGetUniformLocation(ProgramHandle(), "u_material.shininess");

  m_hNormal = glGetAttribLocation(ProgramHandle(), "a_normal");
  m_hVertex = glGetAttribLocation(ProgramHandle(), "a_position");
  m_hColor = glGetAttribLocation(ProgramHandle(), "a_color");
}

bool CScreensaverLorenz::OnEnabled()
{
  // This is called after glUseProgram()
  glUniformMatrix4fv(m_projMatLoc, 1, GL_FALSE, glm::value_ptr(m_projMat));
  glUniformMatrix4fv(m_modelViewMatLoc, 1, GL_FALSE, glm::value_ptr(m_modelMat));
  glUniformMatrix3fv(m_normalMatLoc, 1, GL_FALSE, glm::value_ptr(m_normalMat));

  glUniform1i(m_lightingLoc, true);
  glUniform4f(m_uniformColorLoc, 0.0f, 0.1f, 0.4f, 0.0f);

  glUniform4f(m_light0_ambientLoc, 0.0f, 0.0f, 0.0f, 1.0f); /* default value */
  glUniform4f(m_light0_diffuseLoc, 1.0f, 1.0f, 0.0f, 1.0f); /* NOT default value */
  glUniform4f(m_light0_specularLoc, 0.1f, 0.1f, 0.05f, 0.0f); /* NOT default value */
  glUniform4f(m_light0_positionLoc, m_position[0], m_position[1], m_position[2], m_position[3]); /* default value */
  glUniform1f(m_light0_constantAttenuationLoc, 0.5f); /* NOT default value */
  glUniform1f(m_light0_linearAttenuationLoc, 0.0f); /* default value */
  glUniform1f(m_light0_quadraticAttenuationLoc, 5.01f); /* NOT default value */
  glUniform3f(m_light0_spotDirectionLoc, m_lightDir0[0], m_lightDir0[1], m_lightDir0[2]); /* default value */
  glUniform1f(m_light0_spotExponentLoc, 0.0f); /* default value */
  glUniform1f(m_light0_spotCutoffAngleCosLoc, 100.0f); /* NOT default value */

  glUniform4f(m_material_ambientLoc, 0.2f, 0.2f, 0.2f, 1.0f); /* default value */
  glUniform4f(m_material_diffuseLoc, 0.8f, 0.8f, 0.8f, 1.0f); /* default value */
  glUniform4f(m_material_specularLoc, 0.0f, 0.0f, 0.0f, 1.0f); /* default value */
  glUniform4f(m_material_emissionLoc, 0.0f, 0.0f, 0.0f, 1.0f); /* default value */
  glUniform1f(m_material_shininessLoc, 0.0f); /* default value */

  return true;
}

void CScreensaverLorenz::reduce_points(int cutoff)
{
  int j=0;
  int start=0;
  int end=0;
  float dist=1;
  int current_offs=0;

  m_num_points=m_settings.num_precomputed_points;

  while (current_offs<m_num_points-1 && start<m_num_points-1 && end<m_num_points-1)
  {
    dist=0;
    for (end=start; end<m_num_points && dist<m_settings.linear_cutoff; end++)
    {
      for (j=start; j<=end && dist<m_settings.linear_cutoff; j++)
      {
        dist=distance_to_line(m_lorenz_coords[3*start],
            m_lorenz_coords[3*start+1],m_lorenz_coords[3*start+2],
            m_lorenz_coords[3*j],m_lorenz_coords[3*j+1],m_lorenz_coords[3*j+2],
            m_lorenz_coords[3*end],m_lorenz_coords[3*end+1],m_lorenz_coords[3*end+2]);
      }
    }

    end--;
    current_offs++;
    m_lorenz_path[3*current_offs]=m_lorenz_coords[3*end];
    m_lorenz_path[3*current_offs+1]=m_lorenz_coords[3*end+1];
    m_lorenz_path[3*current_offs+2]=m_lorenz_coords[3*end+2];
    start=end;
  }

  m_num_points=current_offs;

  if (m_num_points_max>0 && m_num_points>m_num_points_max)
    m_num_points=m_num_points_max;
}

void CScreensaverLorenz::init_line_strip(void)
{
  float n[3];

  glClearColor(0.0, 0.0, 0.0, 0.0);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);

  for (int i = 100; i < m_num_points-20; i++)
  {
    calc_normal(m_lorenz_path[3*i], m_lorenz_path[3*i+1], m_lorenz_path[3*i+2],
        m_lorenz_path[3*(i+3)], m_lorenz_path[3*(i+3)+1], m_lorenz_path[3*(i+3)+2],
        m_lorenz_path[3*(i+5)], m_lorenz_path[3*(i+5)+2], m_lorenz_path[3*(i+5)+2],
        &n[0],&n[1],&n[2]);

    norm_one(&n[0], &n[1], &n[2]);

    sLatticeSegmentEntry entry;
    entry.normal.x = n[0];
    entry.normal.y = n[1];
    entry.normal.z = n[2];
    entry.vertex.x = m_lorenz_path[i*3+0];
    entry.vertex.y = m_lorenz_path[i*3+1];
    entry.vertex.z = m_lorenz_path[i*3+2];
    entry.color.r = 1.0f;
    entry.color.g = 1.0f;
    entry.color.b = 1.0f;
    entry.color.a = 1.0f;
    m_stripEntries.push_back(std::move(entry));
  }
}

void CScreensaverLorenz::display(void)
{
  int satellites = 0;
  float n[3];
  float p1[3];
  float p2[3];
  float p3[3];
  float l=0;

  //set cam
  glm::mat4 modelview = glm::mat4(1.0f);

  coords_at_time(m_lorenz_coords, simTime,   &p1[0], &p1[1], &p1[2]);
  coords_at_time(m_lorenz_coords, simTime+10,&p2[0], &p2[1], &p2[2]);
  coords_at_time(m_lorenz_coords, simTime+15,&p3[0], &p3[1], &p3[2]);

  calc_normal(p1[0], p1[1], p1[2],
              p2[0], p2[1], p2[2],
              p3[0], p3[1], p3[2],
              &n[0], &n[1], &n[2]);

  norm_one(&n[0], &n[1], &n[2]);

  // n[0]'s sign changes when transitioning between the lobes...
  if ((lastn0 >= 0) != (n[0] >= 0))
  {
    // if we've completed a transition, one in four chance whether to transition again.
    if ((deltaflipn0 >= 1.0) && (rsRandi(4) == 0))
    {
      flipn0 = flipn0 * -1;
      deltaflipn0 = 0;
    }
  }

  lastn0 = n[0];

  deltaflipn0 += m_frameTime;

  // during transition, slerp between flipped states
  if (deltaflipn0 < 1.0)
  {
    float la[3];

    la[0] = p2[0] - p1[0];
    la[1] = p2[1] - p1[1];
    la[2] = p2[2] - p1[2];

    norm_one(&la[0], &la[1], &la[2]);

    rsQuat normal, flipped;
    normal.make(0.0f, la[0], la[1], la[2]);
    flipped.make(static_cast<float>(M_PI), la[0], la[1], la[2]);
    rsQuat f;

    if (flipn0 == -1)
      f.slerp(normal, flipped, deltaflipn0);
    else
      f.slerp(flipped, normal, deltaflipn0);

    rsVec rn = f.apply(rsVec(n[0], n[1], n[2]));
    n[0] = rn[0];
    n[1] = rn[1];
    n[2] = rn[2];
  }
  else
  {
    n[0] = n[0] * flipn0;
    n[1] = n[1] * flipn0;
    n[2] = n[2] * flipn0;
  }

  modelview *= glm::lookAt(glm::vec3(p1[0]+0.9*n[0], p1[1]+0.9*n[1], p1[2]+0.9*n[2]),
                           glm::vec3(p2[0], p2[1], p2[2]),
                           glm::vec3(n[0], n[1], n[2]));

  coords_at_time(m_lorenz_coords, simTime+10,&p2[0], &p2[1], &p2[2]);
  coords_at_time(m_lorenz_coords, simTime+2, &p3[0], &p3[1], &p3[2]);

  //set light
  m_position[0]  = p2[0]+n[0]; m_position[1]  = p2[1]+n[1]; m_position[2]  = p2[2]+n[2]; m_position[3]  = 1.0;
  m_lightDir0[0] = n[0];       m_lightDir0[1] = n[1];       m_lightDir0[2] = n[2];       m_lightDir0[3] = 1.0;

  EnableShader();
  glClear(GL_COLOR_BUFFER_BIT);

  glLineWidth(static_cast<float>(m_settings.line_width_attractor));
  glBufferData(GL_ARRAY_BUFFER, sizeof(sLatticeSegmentEntry)*m_stripEntries.size(), &m_stripEntries[0], GL_STATIC_DRAW);
  glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(m_stripEntries.size()));

  glUniform1i(m_lightingLoc, false);
  for (satellites = 0; satellites < m_settings.num_satellites; satellites++)
  {
    m_modelMat = modelview;
    float x,y,z;
    coords_at_time(m_lorenz_coords, m_satellite_times[satellites], &x, &y, &z);

    glLineWidth(static_cast<float>(m_settings.line_width_satellites));
    float s=37*m_frameTime*m_satellite_speeds[satellites]*m_num_points/m_settings.num_precomputed_points;

    float maxl = ( 10*(fabs(s)) < 3 ) ? 3 : 10 * (fabs(s));
    float stepl = (fabs(s)/maxl);
    std::vector<sLatticeSegmentEntry> entries;
    for (l = 0; l < maxl; l += stepl)
    {
      coords_at_time(m_lorenz_coords, m_satellite_times[satellites]+l, &x,&y,&z);

      sLatticeSegmentEntry entry;
      entry.vertex.x = x;
      entry.vertex.y = y;
      entry.vertex.z = z;
      entry.normal.x = m_lightDir0[0];
      entry.normal.y = m_lightDir0[1];
      entry.normal.z = m_lightDir0[2];
      entry.color.r = 0.4f;
      entry.color.g = 0.3f;
      entry.color.b = s<0 ? 1.0f/(l+1) : 1.0f/(maxl-l+1);
      entry.color.a = s<0 ? 0.9f/(l+1) : 0.9f/(maxl-l+1);
      entries.push_back(std::move(entry));
    }
    m_normalMat = glm::transpose(glm::inverse(glm::mat3(m_modelMat)));
    glBufferData(GL_ARRAY_BUFFER, sizeof(sLatticeSegmentEntry)*entries.size(), &entries[0], GL_STATIC_DRAW);
    glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(entries.size()));

    m_satellite_times[satellites]+=s;
    if (m_satellite_times[satellites]>m_num_points-20)
      m_satellite_times[satellites]-=m_num_points-20;
    if (m_satellite_times[satellites]<0)
      m_satellite_times[satellites]+=m_num_points-20;
  }
  DisableShader();

  glEnable(GL_BLEND);
  glFlush();
}

void CScreensaverLorenz::set_camera()
{
  float fovy = m_settings.camera_angle;
  float aspect = float(m_width)/m_height;
  float zNear = 0.0f;
  float zFar = 1000.0f;
  double sine, cotangent, deltaZ;
  double radians = fovy / 2 * M_PI / 180;

  deltaZ = zFar - zNear;
  sine = sin(radians);
  if ((deltaZ == 0) || (sine == 0) || (aspect == 0)) {
    return;
  }
  cotangent = cos(radians) / sine;

  m_projMat = glm::mat4(cotangent / aspect, 0.0f,  0.0f,  0.0f,
                        0.0f, cotangent, 0.0f, 0.0f,
                        0.0f, 0.0f, -(zFar + zNear) / deltaZ, -1,
                        0.0f, 0.0f, -2 * zNear * zFar / deltaZ, 0.0f);
}

void CScreensaverLorenz::init_satellites()
{
  int i;

  if (m_settings.num_satellites>0)
  {
    m_satellite_times = new float[sizeof(float)*m_settings.num_satellites];
    m_satellite_speeds = new float[sizeof(float)*m_settings.num_satellites];

    for (i = 0; i < m_settings.num_satellites; i++)
    {
      m_satellite_times[i] = rsRandf(static_cast<float>(m_num_points));
      m_satellite_speeds[i] = 10.0f * m_settings.camera_speed*(rsRandf(1.0f) - 0.5f);
    }
  }
}

void CScreensaverLorenz::precompute_lorenz_array()
{
  int i;
  float max[3] = { 1,1,1 };

  m_lorenz_coords = new float[3*m_settings.num_precomputed_points*sizeof(float)];
  m_lorenz_path = new float[3*m_settings.num_precomputed_points*sizeof(float)];

  m_lorenz_coords[0] = 20;
  m_lorenz_coords[1] = 5;
  m_lorenz_coords[2] = -5;

  for (i = 0; i < m_settings.num_precomputed_points-1; i++)
  {
    m_lorenz_coords[(i+1)*3] = m_lorenz_coords[i*3] + lorenz_a *
      ( m_lorenz_coords[i*3+1] - m_lorenz_coords[i*3] ) * lorenz_dt;

    m_lorenz_coords[(i+1)*3+1] = m_lorenz_coords[i*3+1] +
      ( lorenz_b * m_lorenz_coords[i*3] - m_lorenz_coords[i*3+1] -
        m_lorenz_coords[i*3] * m_lorenz_coords[i*3+2] ) * lorenz_dt;

    m_lorenz_coords[(i+1)*3+2] = m_lorenz_coords[i*3+2] +
      ( m_lorenz_coords[i*3] * m_lorenz_coords[i*3+1] -
        lorenz_c * m_lorenz_coords[i*3+2] ) * lorenz_dt;
  }

  for (i = 0; i < m_settings.num_precomputed_points; i++)
  {
    mean[0] += m_lorenz_coords[i*3];
    mean[1] += m_lorenz_coords[i*3+1];
    mean[2] += m_lorenz_coords[i*3+2];
  }

  mean[0] /= m_settings.num_precomputed_points;
  mean[1] /= m_settings.num_precomputed_points;
  mean[2] /= m_settings.num_precomputed_points;

  for (i = 0; i < m_settings.num_precomputed_points; i++)
  {
    m_lorenz_coords[i*3] -= mean[0];
    m_lorenz_coords[i*3+1] -= mean[1];
    m_lorenz_coords[i*3+2] -= mean[2];
  }

  for (i = 0; i < m_settings.num_precomputed_points; i++)
  {
    if (m_lorenz_coords[i*3] > max[0])
      max[0] = m_lorenz_coords[i*3];
    if (m_lorenz_coords[i*3+1] > max[1])
      max[1] = m_lorenz_coords[i*3+1];
    if (m_lorenz_coords[i*3+2] > max[2])
      max[2] = m_lorenz_coords[i*3+2];
  }

  float m = max[0];
  if (m < max[1])
    m = max[1];
  if (m < max[2])
    m = max[2];

  m = 2;
  for (i = 0; i < m_settings.num_precomputed_points; i++)
  {
    m_lorenz_coords[i*3] /= m;
    m_lorenz_coords[i*3+1] /= m;
    m_lorenz_coords[i*3+2] /= m;
  }
}

ADDONCREATOR(CScreensaverLorenz)
