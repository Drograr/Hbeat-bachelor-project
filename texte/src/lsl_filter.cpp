#include <stdio.h>
#include <obs.h>
#include <algorithm>
#include <chrono>
#include <obs-frontend-api.h>
#include <cmath>
#include <lsl_filter.h>

#include FT_FREETYPE_H
#include <ft2build.h>
#include <sys/stat.h>
#include "obs-convenience.h"
#include "util/platform.h"

#define S_RECORD "record"
#define S_STREAM "stream"
#define S_NAME  "name"
#define S_ID "id"
#define S_START "start"
#define S_STOP "stop"

#define T_RECORD "Record"
#define T_STREAM "Stream"
#define T_NAME  "Name"
#define T_ID "Id"
#define T_START "Start"
#define T_STOP "Stop"

FT_Library ft2_lib;

uint32_t texbuf_w = 2048, texbuf_h = 2048;

const char *channels_f[] = {"frame"};

obs_output_t *lsl_fil;
bool filter_record = false;
bool filter_stream = false;
bool filter_pause = false;

bool check_f(bool stream_data, bool record_data) {
    // if stream check or record_check is validate we return true
    bool stream = stream_data && filter_stream;
    bool record = record_data && filter_record;
    return stream || record;
}

bool init_font(struct lsl_filter* srcdata)
{

    FT_Long index = 0;
    /*
    const char* path = get_font_path(srcdata->font_name, srcdata->font_size,
        srcdata->font_style,
        srcdata->font_flags, &index);
    if (!path)
        return false;

    if (srcdata->font_face != NULL) {
        FT_Done_Face(srcdata->font_face);
        srcdata->font_face = NULL;
    }
    */
    std::string path = "C:/Windows/Fonts/Arial.tff";
    return FT_New_Face(ft2_lib, path.c_str(), index, &srcdata->font_face) == 0;
}

static bool lsl_filter_start(obs_properties_t* props, obs_property_t* p, void* data)
{
    obs_property_t* stop = obs_properties_get(props, S_STOP);
    lsl_filter* filter = (lsl_filter*) data;
    obs_source_t* target = obs_filter_get_target(filter->src);
    struct obs_video_info ovi = { 0 };
    uint32_t cx, cy;
    cx = obs_source_get_base_width(target);
    cy = obs_source_get_base_height(target);
    if (cx > 0 && cy > 0)
    {
        filter->cx = cx;
        filter->cy = cy;
        filter->active = true;
    }
    else {
        blog(LOG_WARNING, "lsl_filter target size error");
        filter->active = false;
    }

    double framerate = video_output_get_frame_rate(obs_get_video());
    //double framerate = lsl::IRREGULAR_RATE; //TODO consider using this to prevent LSL from interpolating data.

    if (filter->active) {
        lsl::stream_info info(filter->name, "OBS frame numbers", 4, framerate, lsl::cf_double64, filter->id_name);
        lsl::xml_element chns = info.desc().append_child("channels");
        chns.append_child("channel").append_child_value("label", "frame_time").append_child_value("unit", "seconds").append_child_value("type", "timestamp");
        chns.append_child("channel").append_child_value("label", "frame_num").append_child_value("unit", "integer").append_child_value("type", "frameID");
        chns.append_child("channel").append_child_value("label", "frame_dtime").append_child_value("unit", "seconds").append_child_value("type", "time_diff");
        chns.append_child("channel").append_child_value("label", "frames_rendered").append_child_value("unit", "integer").append_child_value("type", "count");
        //chns.append_child("channel").append_child_value("label", "frames_skipped").append_child_value("unit", "integer").append_child_value("type", "count");
        filter->outlet = new lsl::stream_outlet(info);
        blog(LOG_INFO, "start LSL output");
        obs_property_set_visible(p, false);
        obs_property_set_visible(stop, true);
    }
    return filter->active;
}

static bool lsl_filter_stop(obs_properties_t* props, obs_property_t* p, void* data)
{
    lsl_filter* filter = (lsl_filter*)data;
    obs_property_t* start = obs_properties_get(props, S_START);
    filter->active = false;
    obs_property_set_visible(p, false);
    obs_property_set_visible(start, true);
    filter_record = false;
    filter_stream = false;
    filter_pause = false;
    delete filter->outlet;
    filter->outlet = NULL;
    blog(LOG_INFO, "lsl-filter stop");
    return true;
}

static obs_properties_t *lsl_filter_properties(void *data){
    lsl_filter* filter_data = (lsl_filter*)data;

    obs_properties_t *props = obs_properties_create();

    obs_properties_add_bool(props, S_RECORD, T_RECORD);
    obs_properties_add_bool(props, S_STREAM, T_STREAM);
    obs_properties_add_text(props, S_NAME, T_NAME, OBS_TEXT_DEFAULT);
    obs_properties_add_text(props, S_ID, T_ID, OBS_TEXT_DEFAULT);
    obs_property* start, * stop;

    start = obs_properties_add_button(props, S_START, T_START,lsl_filter_start);
    stop = obs_properties_add_button(props, S_STOP, T_STOP,lsl_filter_stop);
    obs_property_set_visible(start, !filter_data->active);
    obs_property_set_visible(stop, filter_data->active);
    return props;
}

static void lsl_filter_destroy(void *data){
    struct lsl_filter* srcdata = (lsl_filter*)data;

    if (srcdata->font_face != NULL) {
        FT_Done_Face(srcdata->font_face);
        srcdata->font_face = NULL;
    }

    for (uint32_t i = 0; i < num_cache_slots; i++) {
        if (srcdata->cacheglyphs[i] != NULL) {
            bfree(srcdata->cacheglyphs[i]);
            srcdata->cacheglyphs[i] = NULL;
        }
    }

    if (srcdata->font_name != NULL)
        bfree(srcdata->font_name);
    if (srcdata->font_style != NULL)
        bfree(srcdata->font_style);
    if (srcdata->text != NULL)
        bfree(srcdata->text);
    if (srcdata->texbuf != NULL)
        bfree(srcdata->texbuf);
    if (srcdata->colorbuf != NULL)
        bfree(srcdata->colorbuf);
    if (srcdata->text_file != NULL)
        bfree(srcdata->text_file);

    obs_enter_graphics();

    if (srcdata->tex != NULL) {
        gs_texture_destroy(srcdata->tex);
        srcdata->tex = NULL;
    }
    if (srcdata->vbuf != NULL) {
        gs_vertexbuffer_destroy(srcdata->vbuf);
        srcdata->vbuf = NULL;
    }
    if (srcdata->draw_effect != NULL) {
        gs_effect_destroy(srcdata->draw_effect);
        srcdata->draw_effect = NULL;
    }

    obs_leave_graphics();

    bfree(srcdata);
}

static void lsl_filter_update(void *data, obs_data_t *settings)
{
	lsl_filter *filter_data = (lsl_filter*) data;

	const char* id_name = obs_data_get_string(settings,S_ID);
    const char* name = obs_data_get_string(settings,S_NAME);
    filter_data->id_name = (char *) id_name;
    filter_data->name = (char *) name;
    filter_data->record = obs_data_get_bool(settings,S_RECORD);
	filter_data->stream = obs_data_get_bool(settings, S_STREAM);

    obs_data_t* font_obj = obs_data_get_obj(settings, "font");
    bool vbuf_needs_update = false;
    bool word_wrap = false;
    uint32_t color[2];
    uint32_t custom_width = 0;

    const char* font_name = obs_data_get_string(font_obj, "face");
    const char* font_style = obs_data_get_string(font_obj, "style");
    uint16_t font_size = (uint16_t)obs_data_get_int(font_obj, "size");
    uint32_t font_flags = (uint32_t)obs_data_get_int(font_obj, "flags");

    if (!font_obj)
        return;

    filter_data->outline_width = 0;

    filter_data->drop_shadow = obs_data_get_bool(settings, "drop_shadow");
    filter_data->outline_text = obs_data_get_bool(settings, "outline");

    if (filter_data->outline_text && filter_data->drop_shadow)
        filter_data->outline_width = 6;
    else if (filter_data->outline_text || filter_data->drop_shadow)
        filter_data->outline_width = 4;

    word_wrap = false;// obs_data_get_bool(settings, "word_wrap");

    color[0] = 0xFFFFFFFF;//(uint32_t)obs_data_get_int(settings, "color1");
    color[1] = 0xFFFFFFFF;//(uint32_t)obs_data_get_int(settings, "color2");

    custom_width = 101;// (uint32_t)obs_data_get_int(settings, "custom_width");
    if (custom_width >= 100) {
        if (custom_width != filter_data->custom_width) {
            filter_data->custom_width = custom_width;
            vbuf_needs_update = true;
        }
    }
    else {
        if (filter_data->custom_width >= 100)
            vbuf_needs_update = true;
        filter_data->custom_width = 0;
    }

    if (word_wrap != filter_data->word_wrap) {
        filter_data->word_wrap = word_wrap;
        vbuf_needs_update = true;
    }

    if (color[0] != filter_data->color[0] || color[1] != filter_data->color[1]) {
        filter_data->color[0] = color[0];
        filter_data->color[1] = color[1];
        vbuf_needs_update = true;
    }

    if (ft2_lib == NULL)
        goto error;

    const size_t texbuf_size = (size_t)texbuf_w * (size_t)texbuf_h;

    if (filter_data->draw_effect == NULL) {
        char* effect_file = NULL;
        char* error_string = NULL;

        effect_file = obs_module_file("text_default.effect");

        if (effect_file) {
            obs_enter_graphics();
            filter_data->draw_effect = gs_effect_create_from_file(
                effect_file, &error_string);
            obs_leave_graphics();

            bfree(effect_file);
            if (error_string != NULL)
                bfree(error_string);
        }
    }

    if (filter_data->font_size != font_size)
        vbuf_needs_update = true;

    const bool new_aa_setting = true;//obs_data_get_bool(settings, "antialiasing");
    const bool aa_changed = filter_data->antialiasing != new_aa_setting;
    if (aa_changed) {
        filter_data->antialiasing = new_aa_setting;
        if (filter_data->texbuf != NULL) {
            memset(filter_data->texbuf, 0, texbuf_size);
        }
        cache_standard_glyphs(filter_data);
    }

    if (filter_data->font_name != NULL) {
        if (strcmp(font_name, filter_data->font_name) == 0 &&
            strcmp(font_style, filter_data->font_style) == 0 &&
            font_flags == filter_data->font_flags &&
            font_size == filter_data->font_size)
            goto skip_font_load;

        bfree(filter_data->font_name);
        bfree(filter_data->font_style);
        filter_data->font_name = NULL;
        filter_data->font_style = NULL;
        filter_data->max_h = 0;
        vbuf_needs_update = true;
    }

    filter_data->font_name = bstrdup(font_name);
    filter_data->font_style = bstrdup(font_style);
    filter_data->font_size = font_size;
    filter_data->font_flags = font_flags;

    if (!init_font(filter_data) || filter_data->font_face == NULL) {
        blog(LOG_WARNING, "FT2-text: Failed to load font %s",
            filter_data->font_name);
        goto error;
    }
    else {
        FT_Set_Pixel_Sizes(filter_data->font_face, 0, filter_data->font_size);
        FT_Select_Charmap(filter_data->font_face, FT_ENCODING_UNICODE);
    }

    if (filter_data->texbuf != NULL) {
        bfree(filter_data->texbuf);
        filter_data->texbuf = NULL;
    }
    filter_data->texbuf = (uint8_t*)bzalloc(texbuf_size);

    if (filter_data->font_face)
        cache_standard_glyphs(filter_data);

skip_font_load:

    const char* tmp = "ID:  Timestamp: ";
    if (!tmp || !*tmp)
        goto error;

    if (filter_data->text != NULL) {
        bfree(filter_data->text);
        filter_data->text = NULL;
    }

    os_utf8_to_wcs_ptr(tmp, strlen(tmp), &filter_data->text);

    if (filter_data->font_face) {
        cache_glyphs(filter_data, filter_data->text);
        set_up_vertex_buffer(filter_data);
    }

error:
    obs_data_release(font_obj);


}

static const char *lsl_filter_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return "LSL Filter";
}

static void lsl_filter_video(void* data, gs_effect_t* effect) {
    lsl_filter* srcdata = (lsl_filter*)data;

    if (srcdata == NULL)
        return;

    if (srcdata->tex == NULL || srcdata->vbuf == NULL)
        return;
    if (srcdata->text == NULL || *srcdata->text == 0)
        return;

    gs_reset_blend_state();
    /*
    if (srcdata->outline_text)
        draw_outlines(srcdata);
    if (srcdata->drop_shadow)
        draw_drop_shadow(srcdata);
    */

    draw_uv_vbuffer(srcdata->vbuf, srcdata->tex, srcdata->draw_effect,
        (uint32_t)wcslen(srcdata->text) * 6);

    UNUSED_PARAMETER(effect);
}

static void lsl_filter_tick(void* data, float seconds) {
    lsl_filter* f = (lsl_filter*)data;

    if (f->outlet !=NULL) {
        if (f->active && check_f(f->stream, f->record) && !filter_pause) {
            // we add one to the frame number
            f->frame_number++;
        }
        else if (f->active && !check_f(f->stream, f->record)) {
            f->frame_number = 0;
        }

        f->frame_time = (double)((double)(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()) / 1000);

        obs_output_t* output = obs_frontend_get_recording_output();

        /*
        uint16_t nanosecs = video_output_get_frame_time(obs_output_video(output));
        auto secs = std::lldiv(nanosecs, 1000000000ULL);
        auto milisecs = std::lldiv(secs.rem, 1000000ULL);
        */
        std::string ts_str = std::to_string(f->frame_time);
        std::string id_str = std::to_string(f->frame_number);

        /*
        std::string ft_str = std::to_string(nanosecs);

        #pragma warning(suppress : 4996)
        f->txtfile = fopen("C:\\Users\\mfano\\Desktop\\frame_inf.txt", "w");
        if (f->txtfile != NULL) {
            fputs(("ID: " + id_str+ " Timestamp: "+ts_str+" frametime: "+ft_str).c_str(), f->txtfile);
            fclose(f->txtfile);
        }
        */
        std::string txt = "I: " + id_str + " Timestamp: " + ts_str;
        std::wstring wtxt = std::wstring(txt.begin(), txt.end());
        f->text = (wchar_t*) wtxt.c_str();

        double sample[4];

        sample[0] = f->frame_time;
        sample[1] = (double)f->frame_number;
        sample[2] = seconds;// double(secs.quot) + double(milisecs.quot) / double(1000);
        sample[3] = obs_output_get_total_frames(output);//video_output_get_total_frames(obs_get_video());


        //sample[4] = obs_output_get_frames_dropped(obs_frontend_get_recording_output());//video_output_get_skipped_frames(obs_get_video());

        // we send the sample to lsl
        f->outlet->push_sample(sample);

        cache_glyphs(f, f->text);
        set_up_vertex_buffer(f);
    }
}

static void *lsl_filter_create(obs_data_t *settings,obs_source_t *context){
    struct lsl_filter *data = (struct lsl_filter *) bzalloc(sizeof(struct lsl_filter));
    data->src = context;

    lsl_filter_init();

    data->render = gs_texrender_create(GS_BGRA, GS_ZS_NONE);

    obs_data_t* font_obj = obs_data_create();
    const uint16_t font_size = 26;

    data->font_size = font_size;

    lsl_filter_update(context, settings);
    UNUSED_PARAMETER(settings);
    return data;

}



struct obs_source_info  create_filter_info() {
    struct obs_source_info  filter_info = {};
    filter_info.id = "lsl_filter";
    filter_info.type = OBS_SOURCE_TYPE_FILTER;
    filter_info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW;
    filter_info.get_name = lsl_filter_name;
    filter_info.create = lsl_filter_create;
    filter_info.destroy = lsl_filter_destroy;
    filter_info.update = lsl_filter_update;
    filter_info.get_properties = lsl_filter_properties;
    filter_info.video_render = lsl_filter_video;
    filter_info.video_tick = lsl_filter_tick;
    filter_info.get_width = lsl_filter_get_width;
    filter_info.get_height = lsl_filter_get_height;

    return filter_info;
}

void lsl_filter_init(){
    // link fonction to the output
    obs_source_info lsl_filter = create_filter_info();
    // register this source
    obs_register_source(&lsl_filter);

   // obs_output_t *output = obs_frontend_get_recording_output();
    //obs_encoder_t* encoder = obs_output_get_video_encoder(output);
    FT_Init_FreeType(&ft2_lib);

    if (ft2_lib == NULL) {
        blog(LOG_WARNING, "FT2-text: Failed to initialize FT2.");
        return;
    }

}

void start_send_record_f(){
	filter_record = true;
    filter_pause = false;
}
void stop_send_record_f(){
    filter_record = false;
    filter_pause = false;
}
void start_send_stream_f(){
    filter_stream = true;
    filter_pause = false;
}
void stop_send_stream_f(){
    filter_stream = false;
    filter_pause = false;
}
void paused_stream_f() {
    filter_pause = true;
}
void unpaused_stream_f() {
    filter_pause = false;
}


uint32_t lsl_filter_get_width(void* data)
{
    struct lsl_filter* srcdata = (lsl_filter*)data;

    return srcdata->cx + srcdata->outline_width;
}

uint32_t lsl_filter_get_height(void* data)
{
    struct lsl_filter* srcdata = (lsl_filter*)data;

    return srcdata->cy + srcdata->outline_width;
}
