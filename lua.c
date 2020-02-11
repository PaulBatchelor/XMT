#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdint.h>
#include <sndfile.h>

#include "base.h"

typedef struct {
    const char *name;
    int (*func)(lua_State *L);
}LEntry;

void make_note_wrapper(
xm_note *n,
int note,
int ins,
int vol,
int fx,
int param)
{
    xm_note tmp = make_note(
    note,
    ins,
    vol,
    fx,
    param);
    *n = tmp;
}

static int L_hello(lua_State *L)
{
	//double cf = lua_tonumber(L, 1);
	//double r = lua_tonumber(L, 2);
	//size_t n = sizeof(sp_fx);
	//sp_fx *fx = (sp_fx *)lua_newuserdata(L, n);
	//moogladder_init(&D.data, fx, cf, r);
    printf("hello world!\n");
	return 1;
}
static int L_init_xm_params(lua_State *L)
{
    xm_params *p = (xm_params *)lua_newuserdata(L, sizeof(xm_params));
    init_xm_params(p);
    return 1;
}
static int L_new(lua_State *L)
{
    xm_file *file = (xm_file *)lua_newuserdata(L, sizeof(xm_file));
    xm_params *p = (xm_params *)lua_touserdata(L, 1);
    //xm_params p;
    //init_xm_params(&p);
    init_xm_file(file, p);
    return 1;
}

static int L_write(lua_State *L)
{
    xm_file *f = (xm_file *)lua_touserdata(L, 1);
    const char *filename = lua_tostring(L, 2);
	if(lua_isuserdata(L, 1))
	write_xm_file(f, filename);
	else
	printf("Could not close file.");
	return 1;
}

static int L_makenote(lua_State *L)
{
    int note = lua_tointeger(L, 1);
    int ins = lua_tointeger(L, 2);
    int vol = lua_tointeger(L, 3);
    int fx = lua_tointeger(L, 4);
    int param = lua_tointeger(L, 5);
    xm_note *n = (xm_note *)lua_newuserdata(L, sizeof(xm_note));
    make_note_wrapper(n, note, ins, vol, fx, param);
    return 1;
}

static int L_addnote(lua_State *L)
{
    xm_file *f = (xm_file *)lua_touserdata(L, 1);
    int patnum = lua_tonumber(L, 2);
    int chan = lua_tonumber(L, 3);
    int row = lua_tonumber(L, 4);
    xm_note *note = (xm_note *)lua_touserdata(L, 5);
    add_note(f, (uint8_t)patnum, (uint8_t)chan, (uint8_t)row, *note);
    return 1;
}

static int L_addinstrument(lua_State *L)
{
    xm_file *f = (xm_file *)lua_touserdata(L, 1);
    int ins = add_instrument(f);
    lua_pushnumber(L, ins);
    return 1;
}

static int L_addsample(lua_State *L)
{
    xm_file *f = (xm_file *)lua_touserdata(L, 1);
    int ins = lua_tonumber(L, 2);
    const char *fname = lua_tostring(L, 3);
    xm_samp_params sparams = new_samp(fname);
    int s = add_samp(f, &sparams, ins);
    lua_pushinteger(L, s);
    return 1;
}

static int L_addbuffer(lua_State *L)
{
    xm_file *f = (xm_file *)lua_touserdata(L, 1);
    int ins = lua_tonumber(L, 2);
    int size = lua_tointeger(L, 3);
    double buffer[size];
    int c;
    luaL_checktype(L, 4, LUA_TTABLE);
    for(c = 1; c <= size; c++)
    {
        lua_rawgeti(L, 4, c);
        buffer[c - 1] = lua_tonumber(L, -1);
        lua_pop(L, 1);
    }
    xm_samp_params sparams = new_buf(buffer,size);
    int s = add_samp(f, &sparams, ins);
    lua_pushinteger(L, s);
    return 1;
}

static int L_transpose(lua_State *L)
{
    xm_file *f = (xm_file *)lua_touserdata(L, 1);
    int ins = lua_tointeger(L, 2);
    int samp = lua_tointeger(L, 3);
    int nn = lua_tointeger(L, 4);
    int fine = lua_tointeger(L, 5);
    xm_transpose_sample(f, ins, samp, nn, fine);
    return 1;
}

static int L_update_ptable(lua_State *L)
{
    xm_file *f = (xm_file *)lua_touserdata(L, 1);
    int pos = lua_tointeger(L, 2);
    int pnum = lua_tointeger(L, 3);
    update_ptable(f, pos, pnum);
    return 1;
}
static int L_set_loop_mode(lua_State *L)
{
    xm_file *f = (xm_file *)lua_touserdata(L, 1);
    int ins = lua_tointeger(L, 2);
    int samp = lua_tointeger(L, 3);
    int mode = lua_tointeger(L, 4);
    xm_set_loop_mode(f, ins, samp, mode);
    return 1;
}

static int L_set_nchan(lua_State *L)
{
    xm_params *p = (xm_params *)lua_touserdata(L, 1);
    int nchan = lua_tointeger(L, 2);
    xm_set_nchan(p, nchan);
    return 1 ;
}

static int L_set_speed(lua_State *L)
{
    xm_params *p = (xm_params *)lua_touserdata(L, 1);
    int speed = lua_tointeger(L, 2);
    xm_set_speed(p, speed);
    return 1;
}
static int L_set_bpm(lua_State *L)
{
    xm_params *p = (xm_params *)lua_touserdata(L, 1);
    int bpm = lua_tointeger(L, 2);
    xm_set_bpm(p, bpm);
    return 1;
}

static int L_create_pattern(lua_State *L)
{
    xm_file *f = (xm_file *)lua_touserdata(L, 1);
    int size = lua_tointeger(L, 2);
    lua_pushnumber(L, create_pattern(f, size));
    return 1;
}

static LEntry entry[] = {
    {"hello", L_hello},
    {"xm_write", L_write},
	{"xm_init_xm_params", L_init_xm_params},
	{"xm_new", L_new},
	{"xm_makenote", L_makenote},
	{"xm_addnote", L_addnote},
	{"xm_addinstrument", L_addinstrument},
	{"xm_addsample", L_addsample},
	{"xm_addbuffer", L_addbuffer},
	{"xm_transpose", L_transpose},
	{"xm_update_ptable", L_update_ptable},
	{"xm_set_loop_mode", L_set_loop_mode},
	{"xm_set_nchan", L_set_nchan},
	{"xm_set_speed", L_set_speed},
	{"xm_set_bpm", L_set_bpm},
	{"xm_create_pattern", L_create_pattern},
    {NULL, NULL}
};


int luaopen_luaxmt(lua_State *L){
    int i = 0;
    while(entry[i].name!= NULL)
    {
	    lua_register(L, entry[i].name, entry[i].func);
        i++;
    }
	return 0;
}
