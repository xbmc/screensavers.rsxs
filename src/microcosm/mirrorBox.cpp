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

#include <Implicit/impCubeVolume.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "mirrorBox.h"
#include "rsCamera.h"
#include "main.h"

MirrorBox::MirrorBox(CScreensaverMicrocosm* base) : m_base(base)
{
  // Planes overflow past 0.0 and 1.0 by a thousandth to hide cracks
  // caused by floating point inaccuracy
  plane0[0] = 1.0;  plane0[1] = 0.0;  plane0[2] = 0.0;  plane0[3] = 0.0001;
  plane1[0] = -1.0; plane1[1] = 0.0;  plane1[2] = 0.0;  plane1[3] = 1.0001;
  plane2[0] = 0.0;  plane2[1] = 1.0;  plane2[2] = 0.0;  plane2[3] = 0.0001;
  plane3[0] = 0.0;  plane3[1] = -1.0; plane3[2] = 0.0;  plane3[3] = 1.0001;
  plane4[0] = 0.0;  plane4[1] = 0.0;  plane4[2] = 1.0;  plane4[3] = 0.0001;
  plane5[0] = 0.0;  plane5[1] = 0.0;  plane5[2] = -1.0; plane5[3] = 1.0001;

  for (unsigned int i = 0; i < 6; ++i)
  {
    mCoeffRate[i] = rsRandf(0.1f) + 0.02f;
    mCoeffPhase[i] = rsRandf(RS_PIx2) - RS_PI;
    // test constants
    //mCoeffRate[i] = 0.01f * float(i) + 0.01f;
    //mCoeffPhase[i] = 0.0f;
  }
}

void MirrorBox::update(float frametime)
{
  for (unsigned int i = 0; i < 6; ++i)
  {
    mCoeffPhase[i] += mCoeffRate[i] * frametime;
    if(mCoeffPhase[i] > RS_PI)
      mCoeffPhase[i] -= RS_PIx2;
    mCoeff[i] = cosf(mCoeffPhase[i]);
  }

  mMatrix.makeTranslate(-0.5f, -0.5f, -0.5f);
  mMatrix.rotate(mCoeff[0] * 3.0f, 1.0f, 0.0f, 0.0f);
  mMatrix.rotate(mCoeff[1] * 3.0f, 0.0f, 1.0f, 0.0f);
  mMatrix.rotate(mCoeff[2] * 3.0f, 0.0f, 0.0f, 1.0f);
  mMatrix.translate(0.5f * mCoeff[3] + 0.5f, 0.5f * mCoeff[4] + 0.5f, 0.5f * mCoeff[5] + 0.5f);
  //mMatrix.translate( 0.5f,  0.5f,  0.5f);

  // test code for keeping objects centered
  mMatrix.identity();
}

void MirrorBox::draw(const float& x, const float& y, const float& z, const float& eyex, const float& eyey, const float& eyez)
{
  glm::mat4 modelMatOld;
  glm::mat4& modelMat = m_base->ModelMatrix();
  glm::mat4 modelMatBase = glm::translate(modelMat, glm::vec3(x, y, z));

  if(m_base->Camera().inViewVolume(rsVec(x+0.5f, y+0.5f, z+0.5f), 0.866025f))
  {
    modelMatOld = modelMat;
    glFrontFace(GL_CCW);
    setClipPlanes();
    modelMat = modelMatBase * glm::mat4(mMatrix[0],  mMatrix[1],  mMatrix[2],  mMatrix[3],
                                        mMatrix[4],  mMatrix[5],  mMatrix[6],  mMatrix[7],
                                        mMatrix[8],  mMatrix[9],  mMatrix[10], mMatrix[11],
                                        mMatrix[12], mMatrix[13], mMatrix[14], mMatrix[15]);
    modelMat = glm::translate(modelMat, glm::vec3(0.5f, 0.5f, 0.5f));
    drawSubBox(eyex+0.5f, eyey+0.5f, eyez+0.5f);
    modelMat = modelMatOld;
  }

  if(m_base->Camera().inViewVolume(rsVec(x+0.5f, y+0.5f, z-0.5f), 0.866025f))
  {
    modelMatOld = modelMat;
    glFrontFace(GL_CW);
    modelMat = glm::scale(modelMatBase, glm::vec3(1.0f, 1.0f, -1.0f));
    setClipPlanes();
    modelMat = modelMat * glm::mat4(mMatrix[0],  mMatrix[1],  mMatrix[2],  mMatrix[3],
                                    mMatrix[4],  mMatrix[5],  mMatrix[6],  mMatrix[7],
                                    mMatrix[8],  mMatrix[9],  mMatrix[10], mMatrix[11],
                                    mMatrix[12], mMatrix[13], mMatrix[14], mMatrix[15]);
    modelMat = glm::translate(modelMat, glm::vec3(0.5f, 0.5f, 0.5f));
    drawSubBox(eyex+0.5f, eyey+0.5f, eyez-0.5f);
    modelMat = modelMatOld;
  }

  if(m_base->Camera().inViewVolume(rsVec(x+0.5f, y-0.5f, z+0.5f), 0.866025f))
  {
    modelMatOld = modelMat;
    glFrontFace(GL_CW);
    modelMat = glm::scale(modelMatBase, glm::vec3(1.0f, -1.0f, 1.0f));
    setClipPlanes();
    modelMat = modelMat * glm::mat4(mMatrix[0],  mMatrix[1],  mMatrix[2],  mMatrix[3],
                                    mMatrix[4],  mMatrix[5],  mMatrix[6],  mMatrix[7],
                                    mMatrix[8],  mMatrix[9],  mMatrix[10], mMatrix[11],
                                    mMatrix[12], mMatrix[13], mMatrix[14], mMatrix[15]);
    modelMat = glm::translate(modelMat, glm::vec3(0.5f, 0.5f, 0.5f));
    drawSubBox(eyex+0.5f, eyey-0.5f, eyez+0.5f);
    modelMat = modelMatOld;
  }

  if(m_base->Camera().inViewVolume(rsVec(x+0.5f, y-0.5f, z-0.5f), 0.866025f))
  {
    modelMatOld = modelMat;
    glFrontFace(GL_CCW);
    modelMat = glm::scale(modelMatBase, glm::vec3(1.0f, -1.0f, -1.0f));
    setClipPlanes();
    modelMat = modelMat * glm::mat4(mMatrix[0],  mMatrix[1],  mMatrix[2],  mMatrix[3],
                                    mMatrix[4],  mMatrix[5],  mMatrix[6],  mMatrix[7],
                                    mMatrix[8],  mMatrix[9],  mMatrix[10], mMatrix[11],
                                    mMatrix[12], mMatrix[13], mMatrix[14], mMatrix[15]);
    modelMat = glm::translate(modelMat, glm::vec3(0.5f, 0.5f, 0.5f));
    drawSubBox(eyex+0.5f, eyey-0.5f, eyez-0.5f);
    modelMat = modelMatOld;
  }

  if(m_base->Camera().inViewVolume(rsVec(x-0.5f, y+0.5f, z+0.5f), 0.866025f)){
    modelMatOld = modelMat;
    glFrontFace(GL_CW);
    modelMat = glm::scale(modelMatBase, glm::vec3(-1.0f, 1.0f, 1.0f));
    setClipPlanes();
    modelMat = modelMat * glm::mat4(mMatrix[0],  mMatrix[1],  mMatrix[2],  mMatrix[3],
                                    mMatrix[4],  mMatrix[5],  mMatrix[6],  mMatrix[7],
                                    mMatrix[8],  mMatrix[9],  mMatrix[10], mMatrix[11],
                                    mMatrix[12], mMatrix[13], mMatrix[14], mMatrix[15]);
    modelMat = glm::translate(modelMat, glm::vec3(0.5f, 0.5f, 0.5f));
    drawSubBox(eyex-0.5f, eyey+0.5f, eyez+0.5f);
    modelMat = modelMatOld;
  }

  if(m_base->Camera().inViewVolume(rsVec(x-0.5f, y+0.5f, z-0.5f), 0.866025f))
  {
    modelMatOld = modelMat;
    glFrontFace(GL_CCW);
    modelMat = glm::scale(modelMatBase, glm::vec3(-1.0f, 1.0f, -1.0f));
    setClipPlanes();
    modelMat = modelMat * glm::mat4(mMatrix[0],  mMatrix[1],  mMatrix[2],  mMatrix[3],
                                    mMatrix[4],  mMatrix[5],  mMatrix[6],  mMatrix[7],
                                    mMatrix[8],  mMatrix[9],  mMatrix[10], mMatrix[11],
                                    mMatrix[12], mMatrix[13], mMatrix[14], mMatrix[15]);
    modelMat = glm::translate(modelMat, glm::vec3(0.5f, 0.5f, 0.5f));
    drawSubBox(eyex-0.5f, eyey+0.5f, eyez-0.5f);
    modelMat = modelMatOld;
  }

  if(m_base->Camera().inViewVolume(rsVec(x-0.5f, y-0.5f, z+0.5f), 0.866025f))
  {
    modelMatOld = modelMat;
    glFrontFace(GL_CCW);
    modelMat = glm::scale(modelMatBase, glm::vec3(-1.0f, -1.0f, 1.0f));
    setClipPlanes();
    modelMat = modelMat * glm::mat4(mMatrix[0],  mMatrix[1],  mMatrix[2],  mMatrix[3],
                                    mMatrix[4],  mMatrix[5],  mMatrix[6],  mMatrix[7],
                                    mMatrix[8],  mMatrix[9],  mMatrix[10], mMatrix[11],
                                    mMatrix[12], mMatrix[13], mMatrix[14], mMatrix[15]);
    modelMat = glm::translate(modelMat, glm::vec3(0.5f, 0.5f, 0.5f));
    drawSubBox(eyex-0.5f, eyey-0.5f, eyez+0.5f);
    modelMat = modelMatOld;
  }

  if(m_base->Camera().inViewVolume(rsVec(x-0.5f, y-0.5f, z-0.5f), 0.866025f))
  {
    modelMatOld = modelMat;
    glFrontFace(GL_CW);
    modelMat = glm::scale(modelMatBase, glm::vec3(-1.0f, -1.0f, -1.0f));
    setClipPlanes();
    modelMat = modelMat * glm::mat4(mMatrix[0],  mMatrix[1],  mMatrix[2],  mMatrix[3],
                                    mMatrix[4],  mMatrix[5],  mMatrix[6],  mMatrix[7],
                                    mMatrix[8],  mMatrix[9],  mMatrix[10], mMatrix[11],
                                    mMatrix[12], mMatrix[13], mMatrix[14], mMatrix[15]);
    modelMat = glm::translate(modelMat, glm::vec3(0.5f, 0.5f, 0.5f));
    drawSubBox(eyex-0.5f, eyey-0.5f, eyez-0.5f);
    modelMat = modelMatOld;
  }

  glFrontFace(GL_CCW);
}

void MirrorBox::setClipPlanes()
{
#if USE_CLIP_PLANES
  glClipPlane(GL_CLIP_PLANE0, plane0);
  glClipPlane(GL_CLIP_PLANE1, plane1);
  glClipPlane(GL_CLIP_PLANE2, plane2);
  glClipPlane(GL_CLIP_PLANE3, plane3);
  glClipPlane(GL_CLIP_PLANE4, plane4);
  glClipPlane(GL_CLIP_PLANE5, plane5);
#endif
}

void MirrorBox::drawSubBox(const float& eyex, const float& eyey, const float& eyez)
{
  const float dist_sq(eyex * eyex + eyey * eyey + eyez * eyez);
  if(dist_sq < 16.0f)
  {
    m_base->DrawSurface0()->draw([&](bool compile, const float* vertices, unsigned int vertex_offset,
                                     const unsigned int* indices, unsigned int index_offset)
    {
      m_base->Draw(vertices, vertex_offset, indices, index_offset);
    });
  }
  else if(dist_sq < 36.0f)
  {
    m_base->DrawSurface1()->draw([&](bool compile, const float* vertices, unsigned int vertex_offset,
                                     const unsigned int* indices, unsigned int index_offset)
    {
      m_base->Draw(vertices, vertex_offset, indices, index_offset);
    });
  }
  else
  {
    m_base->DrawSurface2()->draw([&](bool compile, const float* vertices, unsigned int vertex_offset,
                                     const unsigned int* indices, unsigned int index_offset)
    {
      m_base->Draw(vertices, vertex_offset, indices, index_offset);
    });
  }

/*  srand(0);
  float x, y, z;
  glBegin(GL_TRIANGLES);
    glColor3f(rsRandf(1.0f), rsRandf(1.0f), rsRandf(1.0f));
    x = rsRandf(1.0f);  y = rsRandf(1.0f);  z = rsRandf(1.0f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glColor3f(rsRandf(1.0f), rsRandf(1.0f), rsRandf(1.0f));
    x = rsRandf(1.0f);  y = rsRandf(1.0f);  z = rsRandf(1.0f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glColor3f(rsRandf(1.0f), rsRandf(1.0f), rsRandf(1.0f));
    x = rsRandf(1.0f);  y = rsRandf(1.0f);  z = rsRandf(1.0f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glColor3f(rsRandf(1.0f), rsRandf(1.0f), rsRandf(1.0f));
    x = rsRandf(1.0f);  y = rsRandf(1.0f);  z = rsRandf(1.0f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glColor3f(rsRandf(1.0f), rsRandf(1.0f), rsRandf(1.0f));
    x = rsRandf(1.0f);  y = rsRandf(1.0f);  z = rsRandf(1.0f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glColor3f(rsRandf(1.0f), rsRandf(1.0f), rsRandf(1.0f));
    x = rsRandf(1.0f);  y = rsRandf(1.0f);  z = rsRandf(1.0f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glColor3f(rsRandf(1.0f), rsRandf(1.0f), rsRandf(1.0f));
    x = rsRandf(1.0f);  y = rsRandf(1.0f);  z = rsRandf(1.0f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glColor3f(rsRandf(1.0f), rsRandf(1.0f), rsRandf(1.0f));
    x = rsRandf(1.0f);  y = rsRandf(1.0f);  z = rsRandf(1.0f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glColor3f(rsRandf(1.0f), rsRandf(1.0f), rsRandf(1.0f));
    x = rsRandf(1.0f);  y = rsRandf(1.0f);  z = rsRandf(1.0f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glColor3f(rsRandf(1.0f), rsRandf(1.0f), rsRandf(1.0f));
    x = rsRandf(1.0f);  y = rsRandf(1.0f);  z = rsRandf(1.0f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
    glVertex3f(x + rsRandf(0.6f) - 0.3f, y + rsRandf(0.4f) - 0.3f, z + rsRandf(0.6f) - 0.3f);
  glEnd();*/
}
