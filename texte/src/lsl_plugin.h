#pragma once

#include <obs-module.h>
#include <lsl_cpp.h>
#include <ft2build.h>
#include FT_FREETYPE_H



#define num_cache_slots 65535
#define src_glyph srcdata->cacheglyphs[glyph_index]

struct glyph_info {
	float u, v, u2, v2;
	int32_t w, h, xoff, yoff;
	int32_t xadv;
};

struct lsl_plugin {
	bool text_enable;
	bool autocolor;
	uint16_t font_size;

	const char *lsl_chan_name;


	bool antialiasing;
	wchar_t* text;

	uint32_t cx, cy, max_h;
	uint32_t outline_width;
	uint32_t texbuf_x, texbuf_y;
	uint32_t color[2];
	uint32_t* colorbuf;

	gs_texture_t* tex;

	struct glyph_info* cacheglyphs[num_cache_slots];

	FT_Face font_face;

	uint8_t* texbuf;
	gs_vertbuffer_t* vbuf;

	gs_effect_t* draw_effect;
	bool outline_text, drop_shadow;
	bool  word_wrap;

	int frame_number = 0;
	double frame_time = 0;
	lsl::stream_outlet* outlet = NULL;
	//lsl_inlet*  inlet;
  int beat;
	char* id_name;
	char* name;
	bool linked_lsl = false;

	gs_texrender_t* render;

	obs_source_t* src;
};

extern FT_Library ft2_lib;

void lsl_plugin_init();
void start_send_record_f();
void stop_send_record_f();
void start_send_stream_f();
void stop_send_stream_f();
void paused_stream_f();
void unpaused_stream_f();

uint32_t get_ft2_text_width(wchar_t* text, struct lsl_plugin* srcdata);
uint32_t lsl_plugin_get_width(void* data);
uint32_t lsl_plugin_get_height(void* data);
const char* lsl_plugin_name(void* unused);
static void* lsl_plugin_create(obs_data_t* settings, obs_source_t* source);
static void lsl_plugin_destroy(void* data);
static void lsl_plugin_update(void* data, obs_data_t* settings);
obs_properties_t *lsl_plugin_properties(void* unused);
static void lsl_plugin_video(void* data, gs_effect_t* effect);
static void lsl_plugin_tick(void* data, float seconds);
void cache_standard_glyphs(struct lsl_plugin* srcdata);
void cache_glyphs(struct lsl_plugin* srcdata, wchar_t* cache_glyphs);

void set_up_vertex_buffer(struct lsl_plugin* srcdata);
void fill_vertex_buffer(struct lsl_plugin* srcdata);
void draw_outlines(struct lsl_plugin* srcdata);
void draw_drop_shadow(struct lsl_plugin* srcdata);
