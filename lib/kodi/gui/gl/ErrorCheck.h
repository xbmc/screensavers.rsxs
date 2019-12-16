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

#pragma once

#include <kodi/gui/gl/GL.h>
#include <string>

#if defined(HAS_GL) && !defined(TARGET_DARWIN)
namespace kodi
{
namespace gui
{
namespace gl
{

  const GLubyte* ErrorString(GLenum errorCode);
  void PrintError(bool always, const std::string& place);
  void ActivateErrorMessageCallback();
  void DeactivateErrorMessageCallback();
  void PrintEnabledStatus();
}
}
}
#endif
