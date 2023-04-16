#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "xmt.h"

static void mksine(float *buf, float freq, int sz, int sr)
{
    int n;
    float phs;
    float inc;
    float onedsr;
    phs = 0;
    onedsr = 1.0 / sr;
    inc = onedsr * freq;

    for (n = 0; n < sz; n++) {
        buf[n] = sin(2 * M_PI * phs);
        phs += inc;
        phs = fmod(phs, 1.0);
    }
}

int main(int argc, char *argv[])
{
    xm_file file;
    xm_params p;
    xm_note note;
    int ins;
    float *buf;
    xm_samp_params sparams;

    buf = calloc(1, 44100 * sizeof(float));
    mksine(buf, 440, 44100, 44100);
    xm_params_init(&p);
    xm_file_init(&file, &p);
    xm_create_pattern(&file, 0x40);
    xm_update_ptable(&file, 0, 0);
    ins = xm_add_instrument(&file);
    note = xm_make_note(60, ins, 0x00, 0, 0);

    sparams = xm_new_buf(buf, 44100);
    xm_add_samp(&file, &sparams, ins);

    xm_add_note(&file, 0, 0, 0, note);
    xm_file_write(&file, "out.xm");

    {
        size_t sz;
        sz = xm_calculate_size(&file);
        printf("size is %ld\n", sz);
    }

    free(buf);
    return 0;
}
