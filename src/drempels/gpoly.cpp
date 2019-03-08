/* 
 *  This source file is part of Drempels, a program that falls
 *  under the Gnu Public License (GPL) open-source license.  
 *  Use and distribution of this code is regulated by law under 
 *  this license.  For license details, visit:
 *	http://www.gnu.org/copyleft/gpl.html
 * 
 *  The Drempels open-source project is accessible at 
 *  sourceforge.net; the direct URL is:
 *	http://sourceforge.net/projects/drempels/
 *  
 *  Drempels was originally created by Ryan M. Geiss in 2001
 *  and was open-sourced in February 2005.  The original
 *  Drempels homepage is available here:
 *	http://www.geisswerks.com/drempels/
 *
 */

#include "gpoly.h"

#include <math.h>
#include <stdio.h>
#include <stdint.h>

void iCubicInterp(const float &uL, const float &uR, const float &rL, const float &rR, 
				  int32_t *u, int32_t *du, int32_t *ddu, int32_t *dddu,
				  const float &dx)
{
	{
		float f[6];
		float fddu;
		f[0] = dx*dx*dx;
		f[1] = dx*dx;
		f[2] = uR - uL - rL*dx;
		f[3] = 3*dx*dx;
		f[4] = 2*dx;
		f[5] = rR - rL;

		*u = (int)(uL);
		*du = (int)(rL);
		float denom = (f[4]*f[0] - f[3]*f[1]);
		if (denom != 0)
		{
			fddu = (f[5]*f[0] - f[2]*f[3]) / denom;
			*ddu = (int)(fddu);
			*dddu = (int)(((f[2] - f[1]*fddu) / f[0]));
		}
	}
}

void fCubicInterp(const float &uL, const float &uR, const float &rL, const float &rR, 
				  float *u, float *du, float *ddu, float *dddu,
				  const float &dx)
{
	float f[6];
	float fddu;
	f[0] = dx*dx*dx;
	f[1] = dx*dx;
	f[2] = uR - uL - rL*dx;
	f[3] = 3*dx*dx;
	f[4] = 2*dx;
	f[5] = rR - rL;

	*u = uL;
	*du = rL;
	float denom = (f[4]*f[0] - f[3]*f[1]);
	if (denom != 0)
	{
		fddu = (f[5]*f[0] - f[2]*f[3]) / denom;
		*ddu = fddu;
		*dddu = (f[2] - f[1]*fddu) / f[0];
	}
}

void Warp(const td_cellcornerinfo &cell0, 
	  const td_cellcornerinfo &cell1, 
	  const td_cellcornerinfo &cell2, 
	  const td_cellcornerinfo &cell3, 
	  const unsigned int &_dx, const unsigned int &_dy,
	  unsigned short *buf, const unsigned int &stride)
{
	//  (x0y0)
	//   u0v0 ------ u1v1
	//	|			|
	//	|			|
	//   u2v2 ------->uv3 
	//				(x1y1)

	float ufix = 0, vfix = 0;
	{
		float min;

		min = (cell0.u < cell1.u) ? cell0.u : cell1.u;
		min = (min < cell2.u) ? min : cell2.u;
		min = (min < cell3.u) ? min : cell3.u;
		if (min < 0) 
		{
			ufix = (2.0f - (int)(min/(INTFACTOR*256))) * INTFACTOR*256.0f;
		}

		min = (cell0.v < cell1.v) ? cell0.v : cell1.v;
		min = (min < cell2.v) ? min : cell2.v;
		min = (min < cell3.v) ? min : cell3.v;
		if (min < 0) 
		{
			vfix = (2.0f - (int)(min/(INTFACTOR*256))) * INTFACTOR*256.0f;
		}
	}

	float uL   = 0;
	float duL  = 0;
	float dduL = 0;
	float ddduL= 0;
	float rL   = 0;
	float drL  = 0;
	float ddrL = 0;
	float dddrL= 0;
	float vL   = 0;
	float dvL  = 0;
	float ddvL = 0;
	float dddvL= 0;
	float sL   = 0;
	float dsL  = 0;
	float ddsL = 0;
	float dddsL= 0;
	float uR   = 0;
	float duR  = 0;
	float dduR = 0;
	float ddduR= 0;
	float rR   = 0;
	float drR  = 0;
	float ddrR = 0;
	float dddrR= 0;
	float vR   = 0;
	float dvR  = 0;
	float ddvR = 0;
	float dddvR= 0;
	float sR   = 0;
	float dsR  = 0;
	float ddsR = 0;
	float dddsR= 0;

	// next 24 vars are with respect to y:
	fCubicInterp(cell0.u + ufix, cell2.u + ufix, cell0.dudy, cell2.dudy, &uL, &duL, &dduL, &ddduL, _dy);
	fCubicInterp(cell0.r,        cell2.r,        cell0.drdy, cell2.drdy, &rL, &drL, &ddrL, &dddrL, _dy);
	fCubicInterp(cell0.v + vfix, cell2.v + vfix, cell0.dvdy, cell2.dvdy, &vL, &dvL, &ddvL, &dddvL, _dy);
	fCubicInterp(cell0.s,        cell2.s,        cell0.dsdy, cell2.dsdy, &sL, &dsL, &ddsL, &dddsL, _dy);
	fCubicInterp(cell1.u + ufix, cell3.u + ufix, cell1.dudy, cell3.dudy, &uR, &duR, &dduR, &ddduR, _dy);
	fCubicInterp(cell1.r,        cell3.r,        cell1.drdy, cell3.drdy, &rR, &drR, &ddrR, &dddrR, _dy);
	fCubicInterp(cell1.v + vfix, cell3.v + vfix, cell1.dvdy, cell3.dvdy, &vR, &dvR, &ddvR, &dddvR, _dy);
	fCubicInterp(cell1.s,        cell3.s,        cell1.dsdy, cell3.dsdy, &sR, &dsR, &ddsR, &dddsR, _dy);

	//int u, du, ddu, dddu;
	//int v, dv, ddv, dddv;
	int32_t u0[4] = { 0, 0, 0, 0 };
	int32_t v0[4] = { 0, 0, 0, 0 };

	for (unsigned int ii=0; ii<_dy; ii++)
	{
		const float uL_now = uL + ii*(duL + ii*(dduL + ii*(ddduL)));
		const float rL_now = rL + ii*(drL + ii*(ddrL + ii*(dddrL)));
		const float vL_now = vL + ii*(dvL + ii*(ddvL + ii*(dddvL)));
		const float sL_now = sL + ii*(dsL + ii*(ddsL + ii*(dddsL)));
		const float uR_now = uR + ii*(duR + ii*(dduR + ii*(ddduR)));
		const float rR_now = rR + ii*(drR + ii*(ddrR + ii*(dddrR)));
		const float vR_now = vR + ii*(dvR + ii*(ddvR + ii*(dddvR)));
		const float sR_now = sR + ii*(dsR + ii*(ddsR + ii*(dddsR)));

		iCubicInterp(uL_now, uR_now, rL_now, rR_now, &u0[0], &u0[1], &u0[2], &u0[3], _dx);
		iCubicInterp(vL_now, vR_now, sL_now, sR_now, &v0[0], &v0[1], &v0[2], &v0[3], _dx);
	
		for (unsigned int jj = 0; jj < _dx; ++jj) {
			const int xx = u0[0] + jj * (u0[1] + jj * (u0[2] + jj * u0[3]));
			const int yy = v0[0] + jj * (v0[1] + jj * (v0[2] + jj * v0[3]));

			buf[ii * stride + jj * 2 + 0] = xx >> (SHIFTFACTOR - 8);
			buf[ii * stride + jj * 2 + 1] = yy >> (SHIFTFACTOR - 8);
		}

	}

}
