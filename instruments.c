/*----------------------------------------------------------------------------
  XMT: a library designed to generate XM files.
  Copyright (c) 2014 Paul Batchelor All rights reserved.
    http://www.batchelorsounds.com

  This file is part of XMT.

  XMT is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  U.S.A.
-----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sndfile.h>

#include "base.h"

void init_xm_ins(xm_file *f, xm_ins *i)
{
	//i->size = 0x1d;
	i->size = 0x107;
	memset(i->name, 0, sizeof(char) * 22);
	i->type = 0; 
	i->num_samples = 0;
	memset(i->sample_map, 0, sizeof(uint8_t) * 96);
	int k;
	for(k = 0; k < 12; k++){
		i->volume_points[k].x = 0;
		i->volume_points[k].y = 0;
		i->envelope_points[k].x = 0;
		i->envelope_points[k].y = 0;
	}
	i->sample_header_size = 0x28;
	i->num_volume_points = 2;
	i->num_envelope_points = 2;
	i->vol_sustain = 0x0;
	i->vol_loop_start = 0;
	i->vol_loop_end = 0;
	i->pan_sustain = 0;
	i->pan_loop_start = 0;
	i->pan_loop_end = 0;
	i->vol_type = 0;
	i->pan_type = 0;
	i->vib_type = 0;
	i->vib_sweep = 0;
	i->vib_depth = 0;
	i->vib_rate = 0;
	i->vol_fadeout = 0x7fff;
	memset(i->reserved, 0, sizeof(uint16_t) * 11);
} 

int add_instrument(xm_file *f)
{
    if(f->num_instruments == 0x99) f->num_instruments = 0x01;
    else f->num_instruments++; 
	int n = f->num_instruments;
	init_xm_ins(f, &f->ins[n - 1]);
	return n - 1;
}
