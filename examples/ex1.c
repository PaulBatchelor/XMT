#include <stdint.h>
#include <stdio.h>
#include <sndfile.h>
#include <stdlib.h>
#include <math.h>
#include "../base.h"

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
    init_xm_params(&p);
    init_xm_file(&file, &p);
    create_pattern(&file, 0x40);
    update_ptable(&file, 0, 0);
    ins = add_instrument(&file);
    note = make_note(60, ins, 0x00, 0, 0);

    sparams = new_buf(buf, 44100);
    add_samp(&file, &sparams, ins);

    add_note(&file, 0, 0, 0, note);

    write_xm_file(&file, "out.xm");

    free(buf);
    return 0;
}
