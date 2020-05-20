/*
 *  Copyright (C) 2005-2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <kodi/gui/gl/GL.h>

GLint Build2DMipmaps(GLenum target, GLint internalFormat,
                      GLsizei width, GLsizei height,
                      GLenum format, GLenum type,
                      const void *data);
