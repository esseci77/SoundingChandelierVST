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


#ifndef __SPK_ARRAY_H
#define __SPK_ARRAY_H


class Speaker
{
public:
    
    int     _group;
    float   _x;
    float   _y;
    float   _z;
};


class Sgroup
{
public:

    Sgroup (void);

    bool    _active;
    int     _iring;
    int     _nspkr;
    int     _index [4];
    float   _x;
    float   _y;
    float   _z;
    float   _d;
};


class SPK_array
{
public:

    enum { NSPEAKER = 228, NSGROUP = 64, NS4 = NSPEAKER / 4, NG4 = NSGROUP / 4 };

    SPK_array (void);

    void set_active (int g, bool act);
    const Speaker *speaker (int k) { return _speakers + k; }
    const Sgroup  *sgroup  (int k) { return _sgroups  + k; }

private:

    void addspkr (int s, int g, int r, double x, double y, double z);

    Speaker    _speakers [NSPEAKER];
    Sgroup     _sgroups  [NSGROUP];
};



#endif
