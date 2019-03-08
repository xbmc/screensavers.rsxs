/*
 *  This source file is part of Drempels, a program that falls
 *  under the Gnu Public License (GPL) open-source license.
 *  Use and distribution of this code is regulated by law under
 *  this license.  For license details, visit:
 *    http://www.gnu.org/copyleft/gpl.html
 *
 *  The Drempels open-source project is accessible at
 *  sourceforge.net; the direct URL is:
 *    http://sourceforge.net/projects/drempels/
 *
 *  Drempels was originally created by Ryan M. Geiss in 2001
 *  and was open-sourced in February 2005.  The original
 *  Drempels homepage is available here:
 *    http://www.geisswerks.com/drempels/
 *
 */

#ifndef GEISS_POLYCORE_H
#define GEISS_POLYCORE_H 1

class td_cellcornerinfo
{
public:
	float u, dudy;
	float r, drdy;	// r = dudx
	float v, dvdy;
	float s, dsdy;	// s = dvdx
	// 1. get u's
	// 2. get dudy's
	// 3. get dudx's
	// 4. get dudxdy's from (3)
};

// In the translation to C, an overflow has crept in, resulting in bars when
// zoomed out. Maybe the loss of 64-bit result in evaluating the cubics. Anyway
// I only use 8-bits of the fractional bit, so 10 additional bits looks like it
// still works.
//#define INTFACTOR (65536*256) / 8     // -> for max of 2048x2048 (vs. 256x256)
//#define SHIFTFACTOR (16+8      -3)
#define INTFACTOR (1 << 18)
#define SHIFTFACTOR 18

void Warp(const td_cellcornerinfo &cell0,
	  const td_cellcornerinfo &cell1,
	  const td_cellcornerinfo &cell2,
	  const td_cellcornerinfo &cell3,
	  const unsigned int &_dx, const unsigned int &_dy,
	  unsigned short *buf, const unsigned int &stride);

#endif
