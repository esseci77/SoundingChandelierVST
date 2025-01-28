// -------------------------------------------------------------------------
//
//  Copyright (C) 2008-2010 Fons Adriaensen <fons@kokkinizita.net>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// -------------------------------------------------------------------------


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "spkarray.h"


//#define TEST


Sgroup::Sgroup (void) :
    _nspkr (0), _x (0), _y (0), _z (0)
{
}


SPK_array::SPK_array ()
{
    int       i, j, g, n, r, s;
    double    radius;
    double    angle;
    double    a, d, h, x, y, z, x0, x1, y0, y1;
    double    dd1 [5];
    double    zz1 [5];
    double    dd [8];
    double    zz [8];
    Sgroup    *G;

    radius = 2.0;     // array radius
    angle  = 39.8;    // half array angle

    angle *= M_PI / 180;
    a = angle / 8;
    dd1 [0] = radius * sin (a);
    zz1 [0] = radius * cos (a);
    a = 2 * angle / 8;
    dd1 [1] = radius * sin (a);
    zz1 [1] = radius * cos (a);
    a = 4 * angle / 8;
    dd1 [2] = radius * sin (a);
    zz1 [2] = radius * cos (a);
    a = 6 * angle / 8;
    dd1 [3] = radius * sin (a);
    zz1 [3] = radius * cos (a);
    a = angle;
    dd1 [4] = radius * sin (a);
    zz1 [4] = radius * cos (a);
    
    dd [0] = 0.5 * dd1 [0];
    zz [0] = zz1 [0];
    dd [1] = 0.5 * (dd1 [0] + dd1 [1]);
    zz [1] = 0.5 * (zz1 [0] + zz1 [1]);
    dd [2] = 0.75 * dd1 [1] + 0.25 * dd1 [2];
    zz [2] = 0.75 * zz1 [1] + 0.25 * zz1 [2];
    dd [3] = 0.25 * dd1 [1] + 0.75 * dd1 [2];
    zz [3] = 0.25 * zz1 [1] + 0.75 * zz1 [2];
    dd [4] = 0.75 * dd1 [2] + 0.25 * dd1 [3];
    zz [4] = 0.75 * zz1 [2] + 0.25 * zz1 [3];
    dd [5] = 0.25 * dd1 [2] + 0.75 * dd1 [3];
    zz [5] = 0.25 * zz1 [2] + 0.75 * zz1 [3];
    dd [6] = 0.75 * dd1 [3] + 0.25 * dd1 [4];
    zz [6] = 0.75 * zz1 [3] + 0.25 * zz1 [4];
    dd [7] = 0.25 * dd1 [3] + 0.75 * dd1 [4];
    zz [7] = 0.25 * zz1 [3] + 0.75 * zz1 [4];

    h = sqrt (0.5);
    x0 = y0 = x1 = y1 = 0;
    for (r = g = s = 0; r <  8; r++)
    {
	d = dd [r];
        z = zz [r];
        if (r == 0)
	{
	    g = 0;
	    x = y = h * d;
            addspkr (s,           g,           r,  x,  y, z); 
            addspkr (s +     NS4, g +     NG4, r, -x,  y, z); 
            addspkr (s + 2 * NS4, g + 2 * NG4, r, -x, -y, z); 
            addspkr (s + 3 * NS4, g + 3 * NG4, r,  x, -y, z); 
            s++;
	}
	else
	{
 	    for (i = j = 0; i < 2 * r; i++, j++)
	    {
		if (i == 0)
		{
		    x0 = d; 
		    y0 = 0;
		    x1 = h * d;
		    y1 = h * d;
		}
		if (i == r)
		{
		    x0 = h * d; 
		    y0 = h * d;
		    x1 = 0;
		    y1 = d;
		    j = 0;
		}

		x = ((r - j - 0.5) * x0 + (j + 0.5) * x1) / r;
		y = ((r - j - 0.5) * y0 + (j + 0.5) * y1) / r;

		switch (r)
		{
		case 1:
		    g = i / 2;
		    break;
		case 2:
		    g = 1 + (3 * i + 1) / 4;
		    break;
		case 3:
		    g = 1 + i / 2;
		    break;
		case 4:
		    g = 4 + (5 * i + 2) / 8;
		    break;
		case 5:
		    g = 4 + i / 2;
		    break;
		case 6:
		    g = 9 + (7 * i + 3) / 12;
		    break;
		case 7:
		    g = 9 + i / 2;
		    break;
		}

		addspkr (s,           g,           r,  x,  y, z); 
		addspkr (s +     NS4, g +     NG4, r, -x,  y, z); 
		addspkr (s + 2 * NS4, g + 2 * NG4, r, -x, -y, z); 
		addspkr (s + 3 * NS4, g + 3 * NG4, r,  x, -y, z); 
		s++;
	    }
	}
    }

    for (g = 0; g < NSGROUP; g++)
    {
        G = _sgroups + g;
	G->_active = true;
	n = G->_nspkr;
	G->_x /= n;
	G->_y /= n;
	G->_z /= n;
	G->_d = sqrt (G->_x * G->_x + G->_y * G->_y + G->_z * G->_z);
    }
}


void SPK_array::set_active (int g, bool act)
{
    if ((g >= 0) && (g < NSGROUP)) _sgroups [g]._active = act;
}


void SPK_array::addspkr (int s, int g, int r, double x, double y, double z)
{
    Speaker *S = _speakers + s;
    Sgroup  *G = _sgroups + g;
    S->_group = g;
    S->_x = x;
    S->_y = y;
    S->_z = z;
    G->_iring = r / 2;
    G->_index [G->_nspkr++] = s;
    G->_x += x;
    G->_y += y;
    G->_z += z;
}


#ifdef TEST


int main (void)
{
    int            i, j;
    double         d;
    SPK_array         A;
    const Speaker  *S;
    const Sgroup   *G;

    for (i = 0; i < SPK_array::NSPEAKER; i++)
    {
	S = A.speaker (i);
//        printf ("%5d %5d  %8.3lf %8.3lf %8.3lf\n", i, S->_group, S->_x, S->_y, S->_z);
    }

    for (i = 0; i < SPK_array::NSGROUP; i++)
    {
	G = A.sgroup (i);
	d = sqrt (G->_x * G->_x + G->_y * G->_y + G->_z * G->_z);
        printf ("%5d %8.3lf %8.3lf %8.3lf %8.3lf   ", i, G->_x, G->_y, G->_z, d);
	for (j = 0; j < G->_nspkr; j++) printf ("%5d", G->_index [j]);
        printf ("\n");
    }

    return 0;
}


#endif
