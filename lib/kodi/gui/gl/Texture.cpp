/*
 *  Copyright (C) 2005-2019 Team Kodi
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

#include "Texture.h"

namespace kodi
{
namespace gui
{
namespace gl
{

GLuint Load(const gli::texture& Texture)
{
  gli::gl GL(gli::gl::PROFILE_GL33);
  gli::gl::format const Format = GL.translate(Texture.format(), Texture.swizzles());
  GLenum Target = GL.translate(Texture.target());

  GLuint TextureName = 0;
  glGenTextures(1, &TextureName);
  glBindTexture(Target, TextureName);
#if defined(HAS_GL) || (defined(HAS_GLES) && HAS_GLES == 3)
  glTexParameteri(Target, GL_TEXTURE_BASE_LEVEL, 0);
  glTexParameteri(Target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(Texture.levels() - 1));
  glTexParameteri(Target, GL_TEXTURE_SWIZZLE_R, Format.Swizzles[0]);
  glTexParameteri(Target, GL_TEXTURE_SWIZZLE_G, Format.Swizzles[1]);
  glTexParameteri(Target, GL_TEXTURE_SWIZZLE_B, Format.Swizzles[2]);
  glTexParameteri(Target, GL_TEXTURE_SWIZZLE_A, Format.Swizzles[3]);
#endif
  glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

  glm::tvec3<GLsizei> const Extent(Texture.extent());
  GLsizei const FaceTotal = static_cast<GLsizei>(Texture.layers() * Texture.faces());

  switch(Texture.target())
  {
  case gli::TARGET_1D:
#if defined(HAS_GL)
    glTexStorage1D(
      Target, static_cast<GLint>(Texture.levels()), Format.Internal, Extent.x);
    break;
#endif
  case gli::TARGET_1D_ARRAY:
  case gli::TARGET_2D:
  case gli::TARGET_CUBE:
  {
#if defined(HAS_GL) || (defined(HAS_GLES) && HAS_GLES == 3)
    glTexStorage2D(
      Target, static_cast<GLint>(Texture.levels()), Format.Internal,
      Extent.x, Texture.target() == gli::TARGET_2D ? Extent.y : FaceTotal);
#else
#ifndef glTexStorage2DEXT
    GLsizei width = Extent.x;
    GLsizei height = Texture.target() == gli::TARGET_2D ? Extent.y : FaceTotal;
    for (int i = 0; i < static_cast<int>(Texture.levels()); i++)
    {
        glTexImage2D(Target, i, Format.Internal, 
          width, height,
          0, Format.External, Format.Type, nullptr);
        width = std::max(1, (width / 2));
        height = std::max(1, (height / 2));
    }
#else
    glTexStorage2DEXT(
      Target, static_cast<GLint>(Texture.levels()), Format.Internal,
      Extent.x, Texture.target() == gli::TARGET_2D ? Extent.y : FaceTotal);
#endif
#endif
    break;
  }
#if defined(HAS_GL) || (defined(HAS_GLES) && HAS_GLES == 3)
  case gli::TARGET_2D_ARRAY:
  case gli::TARGET_3D:
  case gli::TARGET_CUBE_ARRAY:
    glTexStorage3D(
      Target, static_cast<GLint>(Texture.levels()), Format.Internal,
      Extent.x, Extent.y,
      Texture.target() == gli::TARGET_3D ? Extent.z : FaceTotal);
    break;
#endif
  default:
    assert(0);
    break;
  }

  for(std::size_t Layer = 0; Layer < Texture.layers(); ++Layer)
  for(std::size_t Face = 0; Face < Texture.faces(); ++Face)
  for(std::size_t Level = 0; Level < Texture.levels(); ++Level)
  {
    GLsizei const LayerGL = static_cast<GLsizei>(Layer);
    glm::tvec3<GLsizei> Extent(Texture.extent(Level));
    Target = gli::is_target_cube(Texture.target())
      ? static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face)
      : Target;

    switch(Texture.target())
    {
    case gli::TARGET_1D:
#ifdef HAS_GL
      if(gli::is_compressed(Texture.format()))
        glCompressedTexSubImage1D(
          Target, static_cast<GLint>(Level), 0, Extent.x,
          Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
          Texture.data(Layer, Face, Level));
      else
        glTexSubImage1D(
          Target, static_cast<GLint>(Level), 0, Extent.x,
          Format.External, Format.Type,
          Texture.data(Layer, Face, Level));
      break;
#endif
    case gli::TARGET_1D_ARRAY:
    case gli::TARGET_2D:
    case gli::TARGET_CUBE:
      if(gli::is_compressed(Texture.format()))
        glCompressedTexSubImage2D(
          Target, static_cast<GLint>(Level),
          0, 0,
          Extent.x,
          Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
          Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
          Texture.data(Layer, Face, Level));
      else
        glTexSubImage2D(
          Target, static_cast<GLint>(Level),
          0, 0,
          Extent.x,
          Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
          Format.External, Format.Type,
          Texture.data(Layer, Face, Level));
      break;
#if defined(HAS_GL) || (defined(HAS_GLES) && HAS_GLES == 3)
    case gli::TARGET_2D_ARRAY:
    case gli::TARGET_3D:
    case gli::TARGET_CUBE_ARRAY:
      if(gli::is_compressed(Texture.format()))
        glCompressedTexSubImage3D(
          Target, static_cast<GLint>(Level),
          0, 0, 0,
          Extent.x, Extent.y,
          Texture.target() == gli::TARGET_3D ? Extent.z : LayerGL,
          Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
          Texture.data(Layer, Face, Level));
      else
        glTexSubImage3D(
          Target, static_cast<GLint>(Level),
          0, 0, 0,
          Extent.x, Extent.y,
          Texture.target() == gli::TARGET_3D ? Extent.z : LayerGL,
          Format.External, Format.Type,
          Texture.data(Layer, Face, Level));
      break;
#endif
    default: assert(0); break;
    }
  }
  return TextureName;
}

GLuint CreateTextureFromMem(const void* data, std::size_t size)
{
  gli::texture Texture = gli::load((const char*)data, size);
  if(Texture.empty())
    return 0;
  return Load(Texture);
}

/// Filename can be KTX or DDS files
GLuint CreateTexture(const std::string& filename)
{
  gli::texture Texture = gli::load(filename);
  if(Texture.empty())
    return 0;
  return Load(Texture);
}

}
}
}
