#include <stdint.h>
#include <stdio.h>
#include <sndfile.h>
#include "../base.h"

int main(int argc, char *argv[])
{
    xm_file file;
    xm_params p;
    xm_note note;

    init_xm_params(&p);
    init_xm_file(&file, &p);
    create_pattern(&file, 0x40);
    update_ptable(&file, 0, 0);
    note = make_note(60, 0, 0x00, 0, 0);
    add_note(&file, 0, 0, 0, note);

    write_xm_file(&file, "out.xm");

    return 0;
}
