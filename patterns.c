#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "base.h"

xm_note xm_make_note(int note,
                     int ins,
                     int vol,
                     int fx,
                     int param)
{
    xm_note n;
    n.pscheme = 0x80;

    if (note != -1) {
        n.pscheme = n.pscheme | NOTE;
        n.note = (note + 1) - 12;
    }

    if (ins != -1) {
        n.pscheme = n.pscheme | INSTRUMENT;
        n.instrument = ins + 1;
    }

    if (vol != -1) {
        n.pscheme = n.pscheme | VOLUME;
        n.volume = vol;
    }

    if (fx != -1) {
        n.pscheme = n.pscheme | FX;
        n.fx = fx;
    }

    if (param != -1) {
        n.pscheme = n.pscheme | PARAM;
        n.fx_param = param;
    }

    return n;
}

void write_note(FILE *f, xm_note *n)
{
    fwrite(&n->pscheme, sizeof(uint8_t), 1, f);
    if (n->pscheme & NOTE)
        fwrite(&n->note, sizeof(uint8_t), 1, f);
    if (n->pscheme & INSTRUMENT)
        fwrite(&n->instrument, sizeof(uint8_t), 1, f);
    if (n->pscheme & VOLUME)
        fwrite(&n->volume, sizeof(uint8_t), 1, f);
    if (n->pscheme & FX)
        fwrite(&n->fx, sizeof(uint8_t), 1, f);
    if (n->pscheme & PARAM)
        fwrite(&n->fx_param, sizeof(uint8_t), 1, f);
}

void xm_add_note(xm_file *f,
                 uint8_t patnum,
                 uint8_t chan,
                 uint8_t row,
                 xm_note note)
{
    xm_pat *p = &f->pat[patnum];
    /*remove previous note */
    xm_remove_note(f, patnum, chan, row);
    if(note.pscheme & NOTE) p->data_size++;
    if(note.pscheme & INSTRUMENT) p->data_size++;
    if(note.pscheme & VOLUME) p->data_size++;
    if(note.pscheme & FX) p->data_size++;
    if(note.pscheme & PARAM) p->data_size++;
    p->data[(row * p->num_channels) + chan ] = note;
}

void xm_remove_note(xm_file *f, uint8_t patnum, uint8_t chan, uint8_t row)
{
    xm_pat *p = &f->pat[patnum];
    xm_note *note = &p->data[(row * p->num_channels) + chan ];
    if(note->pscheme & NOTE) p->data_size--;
    if(note->pscheme & INSTRUMENT) p->data_size--;
    if(note->pscheme & VOLUME) p->data_size--;
    if(note->pscheme & FX) p->data_size--;
    if(note->pscheme & PARAM) p->data_size--;
}

void xm_pat_init(xm_file *f, uint8_t patnum, uint16_t size)
{
    int i;
    xm_pat *p = &f->pat[patnum];
    p->header_size = 0x09;
    p->packing_type = 0x00;
    p->num_rows = size;
    p->num_channels = f->num_channels;
    p->data_size = p->num_rows * f->num_channels;
    for(i = 0; i < p->data_size; i++)
    {
        p->data[i].pscheme = 0x80;
    }
}

void xm_update_ptable(xm_file *f, uint8_t pos, uint8_t pnum){
    if((pos + 1) > f->song_length) f->song_length = pos + 1;
    f->ptable[pos] = pnum;
}

int xm_add_pattern(xm_file *f){
    return f->num_patterns++;
}

int xm_create_pattern(xm_file *f, uint16_t size)
{
    int i;
    xm_pat *p;
    f->num_patterns++;
    p = &f->pat[f->num_patterns - 1];
    p->num_rows = size;
    p->data_size = p->num_rows * f->num_channels;
    for(i = 0; i < p->data_size; i++)
    {
        p->data[i].pscheme = 0x80;
    }
    return f->num_patterns;
}
