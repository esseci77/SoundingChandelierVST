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


#ifndef __GLOBAL_H
#define __GLOBAL_H

#define  EV_X11         16
#define  EV_UDPTX       0
#define  EV_UDPRX       1
#define  EV_EXIT        31

#define  NSRCE          16
#define  NSPKR          64


class OSC_state
{
public:

    float       *_dline;
    int32_t      _flags;
    int32_t      _count;
    float        _glin;
    float        _x;
    float        _y;
    float        _z;
    float        _g;
    float        _dx;
    float        _dy;
    float        _dz;
    float        _dg;
};


class OSC_param
{
public:

    int32_t      _index;
    int32_t      _flags;
    float        _x;
    float        _y;
    float        _z;
    float        _g;
    float        _t;
};


class OSC_queue
{
public:

    OSC_queue (unsigned int size) :
    _size (size),
    _mask (size - 1)	
    {
	assert (! (_size & _mask));
       _data = new OSC_param [_size];
	reset ();
    }

    ~OSC_queue (void)
    {
	delete[] _data;
    }

    void reset (void)
    {
        _nwr = _nrd = 0;
    }  

    int         wr_avail (void) const { return _size - _nwr + _nrd; } 
    void        wr_commit (void) { _nwr++; }
    OSC_param  *wr_ptr (void) { return _data + (_nwr & _mask); }

    int         rd_avail (void) const { return _nwr - _nrd; } 
    void        rd_commit (void) { _nrd++; }
    OSC_param  *rd_ptr (void) { return _data + (_nrd & _mask); }

private:

    OSC_param  *_data;
    int         _size;
    int         _mask;
    int         _nwr;
    int         _nrd;
};


#endif
