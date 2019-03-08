/*
 *  Copyright (C) 2005-2018 Team Kodi
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
 *
 */

#include "ErrorCheck.h"

#include <kodi/General.h>

#if defined(HAS_GL) && !defined(TARGET_DARWIN)

namespace kodi
{
namespace gui
{
namespace gl
{

struct token_string
{
   GLuint Token;
   const char *String;
};

static const struct token_string Errors[] = {
   { GL_NO_ERROR, "no error" },
   { GL_INVALID_ENUM, "invalid enumerant" },
   { GL_INVALID_VALUE, "invalid value" },
   { GL_INVALID_OPERATION, "invalid operation" },
   { GL_STACK_OVERFLOW, "stack overflow" },
   { GL_STACK_UNDERFLOW, "stack underflow" },
   { GL_OUT_OF_MEMORY, "out of memory" },
   { GL_TABLE_TOO_LARGE, "table too large" },
   { GL_INVALID_FRAMEBUFFER_OPERATION_EXT, "invalid framebuffer operation" },
   { 0, NULL } /* end of list indicator */
};

const GLubyte* ErrorString(GLenum errorCode)
{
    int i;
    for (i = 0; Errors[i].String; i++) {
        if (Errors[i].Token == errorCode)
            return (const GLubyte *) Errors[i].String;
    }
    return (const GLubyte *) 0;
}

void PrintError(bool always, const std::string& place)
{
  GLenum err = glGetError();
  const GLubyte* errMsg = ErrorString(err);
  if (errMsg)
  {
    kodi::Log(err == GL_NO_ERROR ? ADDON_LOG_INFO : ADDON_LOG_ERROR,
              "OpenGL Call error from place '%s': %s",
              place.c_str(), errMsg);
    fprintf(stderr, "OpenGL Call error from place '%s': %s\n",
              place.c_str(), errMsg);
  }
  else
  {
    kodi::Log(ADDON_LOG_ERROR,
              "OpenGL unknown Call error from place '%s': %i",
              place.c_str(), static_cast<int>(err));
    fprintf(stderr, "OpenGL unknown Call error from place '%s': %i\n",
              place.c_str(), static_cast<int>(err));
  }
}

void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
  fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
}

void ActivateErrorMessageCallback()
{
  // During init, enable debug output
  glEnable              ( GL_DEBUG_OUTPUT );
  glDebugMessageCallback( MessageCallback, 0 );
}

void DeactivateErrorMessageCallback()
{
  glDisable              ( GL_DEBUG_OUTPUT );
  glDebugMessageCallback( nullptr, 0 );
}

void PrintEnabledStatus()
{
  fprintf(stderr, "-GL_ALPHA_TEST- %i\n", glIsEnabled(GL_ALPHA_TEST));
  fprintf(stderr, "-GL_AUTO_NORMAL- %i\n", glIsEnabled(GL_AUTO_NORMAL));
  fprintf(stderr, "-GL_BLEND- %i\n", glIsEnabled(GL_BLEND));
  fprintf(stderr, "-GL_CLIP_PLANE0- %i\n", glIsEnabled(GL_CLIP_PLANE0));
  fprintf(stderr, "-GL_COLOR_ARRAY- %i\n", glIsEnabled(GL_COLOR_ARRAY));
  fprintf(stderr, "-GL_COLOR_LOGIC_OP- %i\n", glIsEnabled(GL_COLOR_LOGIC_OP));
  fprintf(stderr, "-GL_COLOR_MATERIAL- %i\n", glIsEnabled(GL_COLOR_MATERIAL));
  fprintf(stderr, "-GL_COLOR_SUM- %i\n", glIsEnabled(GL_COLOR_SUM));
  fprintf(stderr, "-GL_COLOR_TABLE- %i\n", glIsEnabled(GL_COLOR_TABLE));
  fprintf(stderr, "-GL_CONVOLUTION_1D- %i\n", glIsEnabled(GL_CONVOLUTION_1D));
  fprintf(stderr, "-GL_CONVOLUTION_2D- %i\n", glIsEnabled(GL_CONVOLUTION_2D));
  fprintf(stderr, "-GL_CULL_FACE- %i\n", glIsEnabled(GL_CULL_FACE));
  fprintf(stderr, "-GL_DEPTH_TEST- %i\n", glIsEnabled(GL_DEPTH_TEST));
  fprintf(stderr, "-GL_DITHER- %i\n", glIsEnabled(GL_DITHER));
  fprintf(stderr, "-GL_EDGE_FLAG_ARRAY- %i\n", glIsEnabled(GL_EDGE_FLAG_ARRAY));
  fprintf(stderr, "-GL_FOG- %i\n", glIsEnabled(GL_FOG));
  fprintf(stderr, "-GL_FOG_COORD_ARRAY- %i\n", glIsEnabled(GL_FOG_COORD_ARRAY));
  fprintf(stderr, "-GL_HISTOGRAM- %i\n", glIsEnabled(GL_HISTOGRAM));
  fprintf(stderr, "-GL_INDEX_ARRAY- %i\n", glIsEnabled(GL_INDEX_ARRAY));
  fprintf(stderr, "-GL_INDEX_LOGIC_OP- %i\n", glIsEnabled(GL_INDEX_LOGIC_OP));
  fprintf(stderr, "-GL_LIGHT0- %i\n", glIsEnabled(GL_LIGHT0));
  fprintf(stderr, "-GL_LIGHT1- %i\n", glIsEnabled(GL_LIGHT1));
  fprintf(stderr, "-GL_LIGHTING- %i\n", glIsEnabled(GL_LIGHTING));
  fprintf(stderr, "-GL_LINE_SMOOTH- %i\n", glIsEnabled(GL_LINE_SMOOTH));
  fprintf(stderr, "-GL_LINE_STIPPLE- %i\n", glIsEnabled(GL_LINE_STIPPLE));
  fprintf(stderr, "-GL_MAP1_COLOR_4- %i\n", glIsEnabled(GL_MAP1_COLOR_4));
  fprintf(stderr, "-GL_MAP1_INDEX- %i\n", glIsEnabled(GL_MAP1_INDEX));
  fprintf(stderr, "-GL_MAP1_NORMAL- %i\n", glIsEnabled(GL_MAP1_NORMAL));
  fprintf(stderr, "-GL_MAP1_TEXTURE_COORD_1- %i\n", glIsEnabled(GL_MAP1_TEXTURE_COORD_1));
  fprintf(stderr, "-GL_MAP1_TEXTURE_COORD_2- %i\n", glIsEnabled(GL_MAP1_TEXTURE_COORD_2));
  fprintf(stderr, "-GL_MAP2_COLOR_4- %i\n", glIsEnabled(GL_MAP1_TEXTURE_COORD_3));
  fprintf(stderr, "-GL_MAP1_TEXTURE_COORD_4- %i\n", glIsEnabled(GL_MAP1_TEXTURE_COORD_4));
  fprintf(stderr, "-GL_MAP2_COLOR_4- %i\n", glIsEnabled(GL_MAP2_COLOR_4));
  fprintf(stderr, "-GL_MAP2_INDEX- %i\n", glIsEnabled(GL_MAP2_INDEX));
  fprintf(stderr, "-GL_MAP2_NORMAL- %i\n", glIsEnabled(GL_MAP2_NORMAL));
  fprintf(stderr, "-GL_MAP2_TEXTURE_COORD_1- %i\n", glIsEnabled(GL_MAP2_TEXTURE_COORD_1));
  fprintf(stderr, "-GL_MAP2_TEXTURE_COORD_2- %i\n", glIsEnabled(GL_MAP2_TEXTURE_COORD_2));
  fprintf(stderr, "-GL_MAP2_TEXTURE_COORD_3- %i\n", glIsEnabled(GL_MAP2_TEXTURE_COORD_3));
  fprintf(stderr, "-GL_MAP2_TEXTURE_COORD_4- %i\n", glIsEnabled(GL_MAP2_TEXTURE_COORD_4));
  fprintf(stderr, "-GL_MAP2_VERTEX_3- %i\n", glIsEnabled(GL_MAP2_VERTEX_3));
  fprintf(stderr, "-GL_MAP2_VERTEX_4- %i\n", glIsEnabled(GL_MAP2_VERTEX_4));
  fprintf(stderr, "-GL_MINMAX- %i\n", glIsEnabled(GL_MINMAX));
  fprintf(stderr, "-GL_MULTISAMPLE- %i\n", glIsEnabled(GL_MULTISAMPLE));
  fprintf(stderr, "-GL_NORMAL_ARRAY- %i\n", glIsEnabled(GL_NORMAL_ARRAY));
  fprintf(stderr, "-GL_NORMALIZE- %i\n", glIsEnabled(GL_NORMALIZE));
  fprintf(stderr, "-GL_POINT_SMOOTH- %i\n", glIsEnabled(GL_POINT_SMOOTH));
  fprintf(stderr, "-GL_POINT_SPRITE- %i\n", glIsEnabled(GL_POINT_SPRITE));
  fprintf(stderr, "-GL_POLYGON_SMOOTH- %i\n", glIsEnabled(GL_POLYGON_SMOOTH));
  fprintf(stderr, "-GL_POLYGON_OFFSET_FILL- %i\n", glIsEnabled(GL_POLYGON_OFFSET_FILL));
  fprintf(stderr, "-GL_POLYGON_OFFSET_LINE- %i\n", glIsEnabled(GL_POLYGON_OFFSET_LINE));
  fprintf(stderr, "-GL_POLYGON_OFFSET_POINT- %i\n", glIsEnabled(GL_POLYGON_OFFSET_POINT));
  fprintf(stderr, "-GL_POLYGON_STIPPLE- %i\n", glIsEnabled(GL_POLYGON_STIPPLE));
  fprintf(stderr, "-GL_POST_COLOR_MATRIX_COLOR_TABLE- %i\n", glIsEnabled(GL_POST_COLOR_MATRIX_COLOR_TABLE));
  fprintf(stderr, "-GL_POST_CONVOLUTION_COLOR_TABLE- %i\n", glIsEnabled(GL_POST_CONVOLUTION_COLOR_TABLE));
  fprintf(stderr, "-GL_RESCALE_NORMAL- %i\n", glIsEnabled(GL_RESCALE_NORMAL));
  fprintf(stderr, "-GL_SAMPLE_ALPHA_TO_COVERAGE- %i\n", glIsEnabled(GL_SAMPLE_ALPHA_TO_COVERAGE));
  fprintf(stderr, "-GL_SAMPLE_ALPHA_TO_ONE- %i\n", glIsEnabled(GL_SAMPLE_ALPHA_TO_ONE));
  fprintf(stderr, "-GL_SAMPLE_COVERAGE- %i\n", glIsEnabled(GL_SAMPLE_COVERAGE));
  fprintf(stderr, "-GL_SCISSOR_TEST- %i\n", glIsEnabled(GL_SCISSOR_TEST));
  fprintf(stderr, "-GL_SECONDARY_COLOR_ARRAY- %i\n", glIsEnabled(GL_SECONDARY_COLOR_ARRAY));
  fprintf(stderr, "-GL_SEPARABLE_2D- %i\n", glIsEnabled(GL_SEPARABLE_2D));
  fprintf(stderr, "-GL_STENCIL_TEST- %i\n", glIsEnabled(GL_STENCIL_TEST));
  fprintf(stderr, "-GL_TEXTURE_1D %i\n", glIsEnabled(GL_TEXTURE_1D));
  fprintf(stderr, "-GL_TEXTURE_2D- %i\n", glIsEnabled(GL_TEXTURE_2D));
  fprintf(stderr, "-GL_TEXTURE_3D- %i\n", glIsEnabled(GL_TEXTURE_3D));
  fprintf(stderr, "-GL_TEXTURE_COORD_ARRAY- %i\n", glIsEnabled(GL_TEXTURE_COORD_ARRAY));
  fprintf(stderr, "-GL_TEXTURE_CUBE_MAP- %i\n", glIsEnabled(GL_TEXTURE_CUBE_MAP));
  fprintf(stderr, "-GL_TEXTURE_GEN_Q- %i\n", glIsEnabled(GL_TEXTURE_GEN_Q));
  fprintf(stderr, "-GL_TEXTURE_GEN_R- %i\n", glIsEnabled(GL_TEXTURE_GEN_R));
  fprintf(stderr, "-GL_TEXTURE_GEN_S- %i\n", glIsEnabled(GL_TEXTURE_GEN_S));
  fprintf(stderr, "-GL_TEXTURE_GEN_T- %i\n", glIsEnabled(GL_TEXTURE_GEN_T));
  fprintf(stderr, "-GL_VERTEX_ARRAY- %i\n", glIsEnabled(GL_VERTEX_ARRAY));
  fprintf(stderr, "-GL_VERTEX_PROGRAM_POINT_SIZE- %i\n", glIsEnabled(GL_VERTEX_PROGRAM_POINT_SIZE));
  fprintf(stderr, "-GL_VERTEX_PROGRAM_TWO_SIDE- %i\n", glIsEnabled(GL_VERTEX_PROGRAM_TWO_SIDE));
}

}
}
}
#endif
