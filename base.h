#ifndef XMT_H
#define XMT_H
#define AMIGA 0
#define LINEAR 1
#define NOTE 1
#define INSTRUMENT 2
#define VOLUME (1 << 2)
#define FX (1 << 3)
#define PARAM (1 << 4)
#define BSIZE 1024

#define NO_LOOP 1
#define FORWARD_LOOP 1
#define PING_PONG 2
#define BIT_16 4

#define NOTEOFF 96

typedef struct {
	char id_text[17];
	char tracker_name[20];
	char module_name[20];
	char var;
	uint16_t version;
	uint32_t header_size;
	uint16_t song_length;
	uint16_t restart_position;
	uint16_t num_channels;
	uint16_t num_patterns;
	uint16_t num_instruments;
	uint16_t freq_table;
	uint16_t speed;
	uint16_t bpm;
	uint8_t ptable[256];
} xm_params;


/* TODO: is this needed? */
typedef struct {
    int dummy;
} xm_ins_params;

typedef struct {
	uint32_t length;
	uint32_t loop_start;
	uint32_t loop_length;
	uint8_t volume;
	int8_t finetune;
	uint8_t type;
	uint8_t panning;
	int8_t nn;
	int8_t reserved;
	char sample_name[22];
	const char *filename;
    int samptype;
    int samplen;
    double *buf;
} xm_samp_params;


typedef struct {
	uint8_t pscheme;
	uint8_t note;
	uint8_t instrument;
	uint8_t volume;
	uint8_t fx;
	uint8_t fx_param;
} xm_note;

typedef struct {
    uint32_t header_size;
    uint8_t packing_type;
    uint16_t num_rows;
    uint16_t num_channels;
    uint16_t data_size;
    #define MAX_PATSIZE 0x100
    xm_note data[MAX_PATSIZE];
} xm_pat;

typedef struct {
    uint16_t x, y;
} point;

typedef struct {
    uint32_t length;
    uint32_t loop_start;
    uint32_t loop_length;
    uint8_t volume;
    int8_t finetune;
    uint8_t type;
    uint8_t panning;
    int8_t nn;
    int8_t reserved;
    char sample_name[22];
    const char *filename;
    /* SNDFILE *sfile; */
    int nchnls;
    double *sampbuf;
    int samptype;
} xm_sample;

typedef struct {
    uint32_t size;
    char name[22];
    uint8_t type;
    uint16_t num_samples;

    /*if num_samples > 0, these become important */
    uint32_t sample_header_size;
    uint8_t sample_map[96];
    point volume_points[12];
    point envelope_points[12];

    uint8_t num_volume_points;
    uint8_t num_envelope_points;
    uint8_t vol_sustain;
    uint8_t vol_loop_start;
    uint8_t vol_loop_end;
    uint8_t pan_sustain;
    uint8_t pan_loop_start;
    uint8_t pan_loop_end;
    uint8_t vol_type;
    uint8_t pan_type;
    uint8_t vib_type;
    uint8_t vib_sweep;
    uint8_t vib_depth;
    uint8_t vib_rate;
    uint16_t vol_fadeout;
    /*reserved 11-byte thing here(?)*/
    uint16_t reserved[11];

    xm_sample sample[16];
} xm_ins;

typedef struct {
    FILE *file;
    char id_text[17];
    char tracker_name[20];
    char module_name[20];
    char var;
    uint16_t version;
    uint32_t header_size;
    uint16_t song_length;
    uint16_t restart_position;
    uint16_t num_channels;
    uint16_t num_patterns;
    uint16_t num_instruments;
    uint16_t freq_table;
    uint16_t speed;
    uint16_t bpm;
    uint8_t ptable[256];

    xm_pat pat[256];
    xm_ins ins[256];
} xm_file;

void init_xm_params(xm_params *p);
void set_nchan(xm_params *p, uint8_t n);
void init_xm_file(xm_file *f, xm_params *p);
int add_samp(xm_file *f, xm_samp_params *s, uint8_t ins);
xm_samp_params new_samp(const char *filename);
xm_samp_params new_buf(double *buf, int size);
int add_instrument(xm_file *f);
void write_xm_file(xm_file *f, const char *filename);
xm_note make_note(
		int note,
		int ins,
		int vol,
		int fx,
		int param);
void add_note(xm_file *f, uint8_t patnum, uint8_t chan,
        uint8_t row, xm_note note);
void remove_note(xm_file *f, uint8_t patnum, uint8_t chan, uint8_t row);

void xm_transpose_sample(xm_file *f, uint8_t ins, uint8_t sample, int8_t nn, uint8_t fine);
void update_ptable(xm_file *f, uint8_t pos, uint8_t pnum);
void xm_set_loop_mode(xm_file *f, uint8_t ins, uint8_t sample, uint8_t mode);
void xm_set_nchan(xm_params *p, uint8_t n);
void xm_set_bpm(xm_params *p, uint8_t bpm);
void xm_set_speed(xm_params *p, uint8_t speed);
int create_pattern(xm_file *f, uint16_t size);
#endif
