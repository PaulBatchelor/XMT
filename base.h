#define AMIGA 0x0
#define LINEAR 0x1
#define NOTE 0b1
#define INSTRUMENT 0b10
#define VOLUME 0b100
#define FX 0b1000
#define PARAM 0b10000
#define BSIZE 1024

#define NO_LOOP 0b000
#define FORWARD_LOOP 0b001
#define PING_PONG 0b010
#define BIT_16 0b100

#define NOTEOFF 96

typedef struct 
{
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
}
xm_params;

typedef struct
{

}
xm_ins_params;

typedef struct
{
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
}
xm_samp_params;


typedef struct
{
	uint8_t pscheme;
	uint8_t note;
	uint8_t instrument;
	uint8_t volume;
	uint8_t fx;
	uint8_t fx_param;
}
xm_note;

typedef struct
{
    uint32_t header_size;
    uint8_t packing_type;
    uint16_t num_rows; 
    uint16_t num_channels;
    uint16_t data_size;
    //uint8_t *data;
    xm_note *data;
}
xm_pat;

typedef struct
{
    uint16_t x, y;
}point;

typedef struct
{
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
    SNDFILE *sfile;
    int nchnls;
    double *sampbuf;
    int samptype;
}
xm_sample;

typedef struct 
{
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
}
xm_ins;

typedef struct
{
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
}
xm_file;

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
