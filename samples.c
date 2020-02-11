#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "base.h"

xm_samp_params new_samp(const char *filename)
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

xm_samp_params new_buf(XMFLT *buf, int size)
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
    /* s.buf = (XMFLT *) malloc(size * sizeof(XMFLT)); */
    /* for (i = 0; i < size; i++){ */
    /*     s.buf[i] = buf[i]; */
    /* } */
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

int8_t scale_8(XMFLT s){
	return (int8_t)(s * 0x7f);
}


/* TODO: don't use malloc */

int8_t write_delta_data(XMFLT *buffer, FILE *out, int count, int8_t prev)
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

	fwrite(delta_buffer, sizeof(int8_t), count, out);
    free(delta_buffer);
	return prev;
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

int add_samp(xm_file *f, xm_samp_params *s, uint8_t ins)
{
	xm_ins *i;
	if(ins > f->num_instruments)
	{
		printf("invalid instrument number..\n");
		ins = ins % f->num_instruments;
	}
	i = &f->ins[ins];
	i->num_samples++;
	init_xm_sample(&i->sample[i->num_samples - 1], s);
	return i->num_samples - 1;
}

void write_sample_data(xm_file *f, int insnum)
{
    int sampnum = f->ins[insnum].num_samples;
    int i;

    for (i = 0; i < sampnum; i++) {
        xm_sample *s = &f->ins[insnum].sample[i];
        fwrite(&s->length, sizeof(uint32_t), 1, f->file);
        fwrite(&s->loop_start, sizeof(uint32_t), 1, f->file);
        fwrite(&s->loop_length, sizeof(uint32_t), 1, f->file);
        fwrite(&s->volume, sizeof(uint8_t), 1, f->file);
        fwrite(&s->finetune, sizeof(int8_t), 1, f->file);
        fwrite(&s->type, sizeof(uint8_t), 1, f->file);
        fwrite(&s->panning, sizeof(uint8_t), 1, f->file);
        fwrite(&s->nn, sizeof(int8_t), 1, f->file);
        fwrite(&s->reserved, sizeof(int8_t), 1, f->file);
        fwrite(&s->sample_name, sizeof(char), 22, f->file);
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
                             f->file,
                             s->length, prev);
        }
    }
}
