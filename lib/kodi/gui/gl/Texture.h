#pragma once
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

#include "GL.h"
#include <gli/gli.hpp>

#include <string>

namespace kodi
{
namespace gui
{
namespace gl
{
  GLuint Load(const gli::texture& Texture);
  GLuint CreateTexture(const std::string& filename);
  GLuint CreateTextureFromMem(const void* data, std::size_t size);
}
}
}
