// noise1234
// Copyright © 2003-2005, Stefan Gustavson
//
// Contact: stegu@itn.liu.se
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

/** \file
		\brief Declares the "noise1" through "noise4" functions for Perlin noise.
		\author Stefan Gustavson (stegu@itn.liu.se)
*/

/*
 * This is a backport to C of my improved noise class in C++.
 * It is highly reusable without source code modifications.
 *
 * Note:
 * Replacing the "float" type with "double" can actually make this run faster
 * on some platforms. A templatized version of Noise1234 could be useful.
 */

#ifdef __cplusplus
extern "C" {
#endif

/** 1D, 2D, 3D and 4D float Perlin noise, SL "noise()"
 */
extern float noise1( float x );
extern float noise2( float x, float y );
extern float noise3( float x, float y, float z );
extern float noise4( float x, float y, float z, float w );

/** 1D, 2D, 3D and 4D float Perlin periodic noise, SL "pnoise()"
 */
extern float pnoise1( float x, int px );
extern float pnoise2( float x, float y, int px, int py );
extern float pnoise3( float x, float y, float z, int px, int py, int pz );
extern float pnoise4( float x, float y, float z, float w,
                              int px, int py, int pz, int pw );

#ifdef __cplusplus
}
#endif
