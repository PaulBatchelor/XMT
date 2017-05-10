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
/*TODO: add error handling to file I/O */

void xm_set_bpm(xm_params *p, uint8_t bpm) 
{
	p->bpm =  bpm;
}
void xm_set_nchan(xm_params *p, uint8_t n)
{
	if(n % 2 == 0)	
	p->num_channels = n;
	else
	p->num_channels = n + 1;
	
}
void xm_set_speed(xm_params *p, uint8_t speed) 
{	
	p->speed = speed;
}

void init_xm_params(xm_params *p)
{
	memset(p->id_text, 0x20, sizeof(char) * 17);
	sprintf(p->id_text, "Extended Module:");
	memset(p->module_name, 0x20, sizeof(char) * 20);
	sprintf(p->module_name, "Test Module");
	memset(p->tracker_name, 0x20, sizeof(char) * 20);
	sprintf(p->tracker_name, "Milkytracker");
	//sprintf(p->tracker_name, "FastTracker v2.00  ");
	p->var = 0x1a;
	p->version = 0x0104;
	p->header_size = 0x114;
	p->song_length = 0x01;
	p->restart_position = 0x00;
	p->num_channels = 0x08;
	p->num_patterns= 0x00;
	p->num_instruments= 0x99;
	p->freq_table = LINEAR;
	p->speed = 6;
	p->bpm = 125;
}

void init_xm_file(xm_file *f, xm_params *p){
	memset(f->id_text, 0x20, sizeof(char) * 17);
	sprintf(f->id_text, "Extended Module: ");
	memset(f->module_name, 0x0, sizeof(char) * 20);
	sprintf(f->module_name, "Test Module");
	memset(f->tracker_name, ' ', sizeof(char) * 20);
	sprintf(f->tracker_name, p->tracker_name);
	f->var = p->var;
	f->version = p->version;
	f->header_size = p->header_size;
	f->song_length = p->song_length;
	f->restart_position = p->restart_position;
	f->num_channels = p->num_channels;
	f->num_patterns= p->num_patterns;
	f->num_instruments= p->num_instruments;
	f->freq_table = p->freq_table;
	f->speed = p->speed;
	f->bpm = p->bpm;
	//initialize pattern table
	memset(f->ptable, 0x0, sizeof(uint8_t) * 256);
    int i;
    for(i = 0; i < 256; i++)
    {
	    init_xm_pat(f, i, 0x40);
    }
    //init_xm_pat(f);

}

void write_pattern_data(xm_file *f)
{
	int i, p;
	
	for(p = 0; p < f->num_patterns; p++)
	{
		fwrite(&f->pat[p].header_size, sizeof(uint32_t), 1, f->file);
		fwrite(&f->pat[p].packing_type, sizeof(uint8_t), 1, f->file);
		fwrite(&f->pat[p].num_rows, sizeof(uint16_t), 1, f->file);
		fwrite(&f->pat[p].data_size, sizeof(uint16_t), 1, f->file);
		for(i = 0; i < f->pat[p].num_rows * f->num_channels; i++){
			write_note(f->file, &f->pat[p].data[i]);
		}
	}
}


void write_instrument_data(xm_file *f)
{
	int i;
	for(i = 0; i < f->num_instruments; i++) 
	{
		fwrite(&f->ins[i].size, sizeof(uint32_t), 1, f->file);
		fwrite(f->ins[i].name, sizeof(char), 22, f->file);
		fwrite(&f->ins[i].type, sizeof(uint8_t), 1, f->file);
		fwrite(&f->ins[i].num_samples, sizeof(uint16_t), 1, f->file);
		if(f->ins[i].num_samples!= 0){
			fwrite(&f->ins[i].sample_header_size, sizeof(uint32_t), 1, f->file);
			fwrite(&f->ins[i].sample_map, sizeof(uint8_t), 96, f->file);
			fwrite(&f->ins[i].volume_points, sizeof(point), 12, f->file);
			fwrite(&f->ins[i].envelope_points, sizeof(point), 12, f->file);
			fwrite(&f->ins[i].num_volume_points, sizeof(uint8_t), 1, f->file);
			fwrite(&f->ins[i].num_envelope_points, sizeof(uint8_t), 1, f->file);
			fwrite(&f->ins[i].vol_sustain, sizeof(uint8_t), 1, f->file);
			fwrite(&f->ins[i].vol_loop_start, sizeof(uint8_t), 1, f->file);
			fwrite(&f->ins[i].vol_loop_end, sizeof(uint8_t), 1, f->file);
			fwrite(&f->ins[i].pan_sustain, sizeof(uint8_t), 1, f->file);
			fwrite(&f->ins[i].pan_loop_start, sizeof(uint8_t), 1, f->file);
			fwrite(&f->ins[i].pan_loop_end, sizeof(uint8_t), 1, f->file);
			fwrite(&f->ins[i].vol_type, sizeof(uint8_t), 1, f->file);
			fwrite(&f->ins[i].pan_type, sizeof(uint8_t), 1, f->file);
			fwrite(&f->ins[i].vib_type, sizeof(uint8_t), 1, f->file);
			fwrite(&f->ins[i].vib_sweep, sizeof(uint8_t), 1, f->file);
			fwrite(&f->ins[i].vib_depth, sizeof(uint8_t), 1, f->file);
			fwrite(&f->ins[i].vib_rate, sizeof(uint8_t), 1, f->file);
			fwrite(&f->ins[i].vol_fadeout, sizeof(uint16_t), 1, f->file);
			fwrite(&f->ins[i].reserved, sizeof(uint16_t), 11, f->file);

			write_sample_data(f, i);
		}
	}
}

void write_header_data(xm_file *f){
	fwrite(f->id_text, sizeof(char), sizeof(f->id_text), f->file);
	fwrite(f->module_name, sizeof(char), sizeof(f->module_name), f->file);
	fwrite(&f->var, sizeof(char), 1, f->file);
	fwrite(f->tracker_name, sizeof(char), sizeof(f->tracker_name), f->file);
	fwrite(&f->version, sizeof(uint16_t), 1, f->file);
	fwrite(&f->header_size, sizeof(uint32_t), 1, f->file);
	fwrite(&f->song_length, sizeof(uint16_t), 1, f->file);
	fwrite(&f->restart_position, sizeof(uint16_t), 1, f->file);
	fwrite(&f->num_channels, sizeof(uint16_t), 1, f->file);
	fwrite(&f->num_patterns, sizeof(uint16_t), 1, f->file);
	fwrite(&f->num_instruments, sizeof(uint16_t), 1, f->file);
	fwrite(&f->freq_table, sizeof(uint16_t), 1, f->file);
	fwrite(&f->speed, sizeof(uint16_t), 1, f->file);
	fwrite(&f->bpm, sizeof(uint16_t), 1, f->file);
	fwrite(f->ptable, sizeof(uint8_t), 256, f->file);
}

void write_xm_file(xm_file *f, const char *filename)
{
    if(f->num_instruments == 0x99){
        add_instrument(f);
    }
    if(f->num_patterns == 0) 
    {
        create_pattern(f, 0x40);
    }
	f->file = fopen(filename, "wb");
	write_header_data(f);
	write_pattern_data(f);
	write_instrument_data(f);
	fclose(f->file);
}

/*
int main()
{
	xm_params p; 
	xm_file file;
	init_xm_params(&p);
	set_nchan(&p, 8);
	init_xm_file(&file, &p);

	int sine = add_instrument(&file);
	xm_samp_params sparams = new_samp("brendan.wav");
	add_samp(&file, &sparams, sine);
	int note[] = {72, 74, 79, 83};
	int i;

	for(i = 0; i < 4; i++) 
	{
		add_note(&file, 0, i, i * 4, make_note(note[i], 1, 0, 0, 0));
		add_note(&file, 0, i, 32, make_note(NOTEOFF, 0, 0, 0, 0));
	}

	write_xm_file(&file);

	return 0;
}
*/
