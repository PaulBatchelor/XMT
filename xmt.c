#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "xmt.h"

static void write_to_file(xm_writer *w, void *ptr, size_t sz)
{
    FILE *fp;
    fp = w->data;
    fwrite(ptr, 1, sz, fp);
}

struct memory_buffer {
    size_t pos;
    char *buf;
};

static void write_to_memory(xm_writer *w, void *ptr, size_t sz)
{
    size_t i;
    size_t pos;
    char *src;
    struct memory_buffer *mem;
    mem = w->data;
    pos = mem->pos;
    src = ptr;
    for (i = 0; i < sz; i++) {
        mem->buf[pos] = src[i];
        pos++;
    }
    mem->pos = pos;
}

static void get_size(xm_writer *w, void *ptr, size_t sz)
{
    size_t *total_size;
    total_size = w->data;
    *total_size += sz;
}

static void writer(xm_writer *w, void *ptr, size_t sz)
{
    w->write(w, ptr, sz);
}

/* base */

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

void xm_params_init(xm_params *p)
{
    memset(p->id_text, 0x20, sizeof(char) * 17);
    strcpy(p->id_text, "Extended Module:");
    p->id_text[16] = ' ';
    memset(p->module_name, 0x20, sizeof(char) * 20);
    sprintf(p->module_name, "Test Module");
    memset(p->tracker_name, 0x20, sizeof(char) * 20);
    sprintf(p->tracker_name, "Milkytracker");
    p->var = 0x1a;
    p->version = 0x0104;
    p->header_size = 0x114;
    p->song_length = 0x01;
    p->restart_position = 0x00;
    p->num_channels = 0x08;
    p->num_patterns = 0x00;
    /* initialzed num_instruments to special value */
    p->num_instruments = 0x99;
    p->freq_table = LINEAR;
    p->speed = 6;
    p->bpm = 125;
}

void xm_file_init(xm_file *f, xm_params *p){
    int i;
    memset(f->id_text, 0x20, sizeof(char) * 17);
    /* only copy 16 characters over. last char is space */
    strncpy(f->id_text, "Extended Module:", 16);
    memset(f->module_name, 0x0, sizeof(char) * 20);
    memcpy(f->module_name, p->module_name, strlen(p->module_name));
    memset(f->tracker_name, ' ', sizeof(char) * 20);
    sprintf(f->tracker_name, "%s", p->tracker_name);
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

    memset(f->ptable, 0x0, sizeof(uint8_t) * 256);

    for (i = 0; i < 256; i++) {
        xm_pat_init(f, i, 0x40);
    }

}

static void write_note(xm_writer *xmw, xm_note *n)
{
    writer(xmw, &n->pscheme, sizeof(uint8_t));
    if (n->pscheme & NOTE)
        writer(xmw, &n->note, sizeof(uint8_t));
    if (n->pscheme & INSTRUMENT)
        writer(xmw, &n->instrument, sizeof(uint8_t));
    if (n->pscheme & VOLUME)
        writer(xmw, &n->volume, sizeof(uint8_t));
    if (n->pscheme & FX)
        writer(xmw, &n->fx, sizeof(uint8_t));
    if (n->pscheme & PARAM)
        writer(xmw, &n->fx_param, sizeof(uint8_t));
}


static void write_pattern_data(xm_writer *xmw)
{
    int i, p;
    xm_file *f;

    f = xmw->xm;

    for(p = 0; p < f->num_patterns; p++)
    {
        writer(xmw, &f->pat[p].header_size, sizeof(uint32_t));
        writer(xmw, &f->pat[p].packing_type, sizeof(uint8_t));
        writer(xmw, &f->pat[p].num_rows, sizeof(uint16_t));
        writer(xmw, &f->pat[p].data_size, sizeof(uint16_t));
        for(i = 0; i < f->pat[p].num_rows * f->num_channels; i++){
            write_note(xmw, &f->pat[p].data[i]);
        }
    }
}

static int8_t scale_8(XMFLT s){
	return (int8_t)(s * 0x7f);
}



/* TODO: don't use malloc */

static int8_t write_delta_data(XMFLT *buffer,
                               xm_writer *xmw,
                               int count,
                               int8_t prev)
{
	int8_t *delta_buffer;
	int8_t tmp;
	int i;

    delta_buffer = calloc(1, sizeof(int8_t) * count);

	for (i = 0; i < count; i++){
		tmp = scale_8(buffer[i]);
		delta_buffer[i] = prev - tmp;
		prev = tmp;
	}

	writer(xmw, delta_buffer, sizeof(int8_t)*count);
    free(delta_buffer);
	return prev;
}

static void write_sample_data(xm_writer *xmw, int insnum)
{
    int sampnum = xmw->xm->ins[insnum].num_samples;
    int i;
    xm_file *f;

    f = xmw->xm;

    for (i = 0; i < sampnum; i++) {
        xm_sample *s = &f->ins[insnum].sample[i];
        writer(xmw, &s->length, sizeof(uint32_t));
        writer(xmw, &s->loop_start, sizeof(uint32_t));
        writer(xmw, &s->loop_length, sizeof(uint32_t));
        writer(xmw, &s->volume, sizeof(uint8_t));
        writer(xmw, &s->finetune, sizeof(int8_t));
        writer(xmw, &s->type, sizeof(uint8_t));
        writer(xmw, &s->panning, sizeof(uint8_t));
        writer(xmw, &s->nn, sizeof(int8_t));
        writer(xmw, &s->reserved, sizeof(int8_t));
        writer(xmw, &s->sample_name, sizeof(char)*22);
    }

    for (i = 0; i < sampnum; i++) {
        xm_sample *s = &f->ins[insnum].sample[i];
        /* XMFLT buffer[BSIZE]; */
        /* int count = -1; */
        int8_t prev = 0;

        if (s->samptype == 0 ) {
            /* Disabled for now */
            /* while(count != 0) { */
            /*     count = sf_read_XMFLT(s->sfile, buffer, BSIZE); */
            /*     prev = write_delta_data(buffer,f->file, count, prev); */
            /* } */
            /* sf_close(s->sfile); */
        } else if (s->samptype == 1) {
            write_delta_data(s->sampbuf,
                             xmw,
                             s->length, prev);
        }
    }
}

static void write_instrument_data(xm_writer *xmw)
{
    int i;
    xm_file *f;

    f = xmw->xm;
    for(i = 0; i < f->num_instruments; i++) {
        writer(xmw, &f->ins[i].size, sizeof(uint32_t));
        writer(xmw, f->ins[i].name, sizeof(char)*22);
        writer(xmw, &f->ins[i].type, sizeof(uint8_t));
        writer(xmw, &f->ins[i].num_samples, sizeof(uint16_t));
        if(f->ins[i].num_samples!= 0){
            writer(xmw, &f->ins[i].sample_header_size, sizeof(uint32_t));
            writer(xmw, &f->ins[i].sample_map, sizeof(uint8_t)*96);
            writer(xmw, &f->ins[i].volume_points, sizeof(xm_point)*12);
            writer(xmw, &f->ins[i].envelope_points, sizeof(xm_point)*12);
            writer(xmw, &f->ins[i].num_volume_points, sizeof(uint8_t));
            writer(xmw, &f->ins[i].num_envelope_points, sizeof(uint8_t));
            writer(xmw, &f->ins[i].vol_sustain, sizeof(uint8_t));
            writer(xmw, &f->ins[i].vol_loop_start, sizeof(uint8_t));
            writer(xmw, &f->ins[i].vol_loop_end, sizeof(uint8_t));
            writer(xmw, &f->ins[i].pan_sustain, sizeof(uint8_t));
            writer(xmw, &f->ins[i].pan_loop_start, sizeof(uint8_t));
            writer(xmw, &f->ins[i].pan_loop_end, sizeof(uint8_t));
            writer(xmw, &f->ins[i].vol_type, sizeof(uint8_t));
            writer(xmw, &f->ins[i].pan_type, sizeof(uint8_t));
            writer(xmw, &f->ins[i].vib_type, sizeof(uint8_t));
            writer(xmw, &f->ins[i].vib_sweep, sizeof(uint8_t));
            writer(xmw, &f->ins[i].vib_depth, sizeof(uint8_t));
            writer(xmw, &f->ins[i].vib_rate, sizeof(uint8_t));
            writer(xmw, &f->ins[i].vol_fadeout, sizeof(uint16_t));
            writer(xmw, &f->ins[i].reserved, sizeof(uint16_t)*11);

            write_sample_data(xmw, i);
        }
    }
}

static void write_header_data(xm_writer *xmw) {
    xm_file *f;
    f = xmw->xm;
    writer(xmw, f->id_text, sizeof(char)*sizeof(f->id_text));
    writer(xmw, f->module_name, sizeof(char)*sizeof(f->module_name));
    writer(xmw, &f->var, sizeof(char));
    writer(xmw, f->tracker_name, sizeof(char)*sizeof(f->tracker_name));
    writer(xmw, &f->version, sizeof(uint16_t));
    writer(xmw, &f->header_size, sizeof(uint32_t));
    writer(xmw, &f->song_length, sizeof(uint16_t));
    writer(xmw, &f->restart_position, sizeof(uint16_t));
    writer(xmw, &f->num_channels, sizeof(uint16_t));
    writer(xmw, &f->num_patterns, sizeof(uint16_t));
    writer(xmw, &f->num_instruments, sizeof(uint16_t));
    writer(xmw, &f->freq_table, sizeof(uint16_t));
    writer(xmw, &f->speed, sizeof(uint16_t));
    writer(xmw, &f->bpm, sizeof(uint16_t));
    writer(xmw, f->ptable, sizeof(uint8_t)*256);
}

static void write_data(xm_writer *xmw)
{
    write_header_data(xmw);
    write_pattern_data(xmw);
    write_instrument_data(xmw);
}

void xm_file_write(xm_file *f, const char *filename)
{
    xm_writer xmw;
    FILE *fp;
    fp = fopen(filename, "wb");
    xmw.xm = f;
    xmw.data = fp;
    xmw.write = write_to_file;
    write_data(&xmw);
    fclose(fp);
}

/* instruments */

void xm_ins_init(xm_file *f, xm_ins *i)
{
    int k;

    i->size = 0x107;
    memset(i->name, 0, sizeof(char) * 22);
    i->type = 0;
    i->num_samples = 0;
    memset(i->sample_map, 0, sizeof(uint8_t) * 96);

    for (k = 0; k < 12; k++) {
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

int xm_add_instrument(xm_file *f)
{
    int n;
    if (f->num_instruments == 0x99) f->num_instruments = 0x01;
    else f->num_instruments++;
    n = f->num_instruments;
    xm_ins_init(f, &f->ins[n - 1]);
    return n - 1;
}

/* patterns */

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
    for(i = 0; i < p->data_size; i++) {
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

/* samples */

xm_samp_params xm_new_samp(const char *filename)
{
	xm_samp_params s;
	s.volume = 0x40;
	s.finetune = 0;
	s.type = NO_LOOP;
	s.panning = 0x80;
	s.nn = 0;
	s.filename = filename;
    s.samptype = 0;
	return s;
}

xm_samp_params xm_new_buf(XMFLT *buf, int size)
{
	xm_samp_params s;
    /* int i; */
	s.volume = 0x40;
	s.finetune = 0;
	s.type = NO_LOOP;
	s.panning = 0x80;
	s.nn = 12 * 2 + 5; /* F6 seems to be normal playback */
    s.samptype = 1;
    s.samplen = size;
    s.buf = buf;
	return s;
}

void xm_transpose_sample(xm_file *f, uint8_t ins, uint8_t sample,
        int8_t nn, uint8_t fine)
{
    f->ins[ins].sample[sample].nn = nn;
    f->ins[ins].sample[sample].finetune = fine;
}

void xm_set_loop_mode(xm_file *f, uint8_t ins, uint8_t sample, uint8_t mode)
{
    f->ins[ins].sample[sample].type = mode;
}

void init_xm_sample(xm_sample *s, xm_samp_params *param)
{
	/* SF_INFO info; */
    s->samptype = param->samptype;
    if(s->samptype == 1) {
        /* int i; */
        /* s->length = param->samplen; */
        /* s->sampbuf = (XMFLT *)malloc(sizeof(XMFLT) * s->length); */
        /* memset(s->sampbuf, 0, sizeof(XMFLT) * s->length); */
        /* for(i = 0; i < s->length; i++) */
        /*     s->sampbuf[i] = param->buf[i]; */
        /* int i; */
        s->length = param->samplen;
        s->sampbuf = param->buf;
    }else if(s->samptype == 0){
	    /* s->sfile = sf_open(param->filename, SFM_READ, &info); */
        /* s->length = info.frames; */
    }
	s->loop_start = 0;
	s->loop_length= s->length;
	s->volume = param->volume;
	s->finetune= param->finetune;
	s->type = param->type;
	s->panning = param->panning;
	s->nn = param->nn;
	s->reserved = 0;
    /* s->nchnls = info.channels; */
    s->nchnls = 1;
	memset(s->sample_name, 0, sizeof(char) * 22);
    /* if(s->samptype == 1) free(param->buf); */
}

int xm_add_samp(xm_file *f, xm_samp_params *s, uint8_t ins)
{
	xm_ins *i;
	if(ins > f->num_instruments) {
		printf("invalid instrument number..\n");
		ins = ins % f->num_instruments;
	}
	i = &f->ins[ins];
	i->num_samples++;
	init_xm_sample(&i->sample[i->num_samples - 1], s);
	return i->num_samples - 1;
}

size_t xm_calculate_size(xm_file *f)
{
    size_t sz;
    xm_writer xmw;

    sz = 0;
    xmw.xm = f;
    xmw.data = &sz;
    xmw.write = get_size;
    write_data(&xmw);

    return sz;
}

void xm_write_to_memory(xm_file *f, char *buf)
{
    struct memory_buffer mem;
    xm_writer xmw;
    mem.buf = buf;
    mem.pos = 0;
    xmw.xm = f;
    xmw.data = &mem;
    xmw.write = write_to_memory;
    write_data(&xmw);
}
