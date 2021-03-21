


#include <obs-module.h>
#include "obs-frontend-api.h"
#include <obs.h>

#include <stdio.h>
#include <obs.h>
#include <algorithm>
#include <chrono>
#include <obs-frontend-api.h>
#include <cmath>
#include <lsl_plugin.h>

#include FT_FREETYPE_H
#include <ft2build.h>
#include <sys/stat.h>
#include "obs-convenience.h"
#include "util/platform.h"



#define S_NAME  "name"
#define S_ID "id"
#define S_START "start"
#define S_STOP "stop"
#define S_LINK "link"
#define S_HASTEXT "hastext"
#define S_AUTOCOLOR "color autochange"
#define S_FONTSIZE "fontsize"
#define S_COLOR "color"
#define S_OUTLINE "outline"
#define S_SHADOW "shadow"



#define T_NAME  "Name"
#define T_ID "Id"
#define T_START "Start"
#define T_STOP "Stop"
#define T_LINK "Link"
#define T_HASTEXT "Enable text"
#define T_AUTOCOLOR "color autochange"
#define T_FONTSIZE "Font size"
#define T_COLOR "Text color"
#define T_OUTLINE "Text outline"
#define T_SHADOW "Text shadow"



FT_Library ft2_lib;
OBS_DECLARE_MODULE()

//OBS_MODULE_USE_DEFAULT_LOCALE("lsl_plugin", "en-US")

MODULE_EXPORT const char* obs_module_description(void)
{
    return "LSL plugin source";
}

uint32_t texbuf_w = 2048, texbuf_h = 2048;

bool plugin_initialized = false;



bool plugin_pause = false;



bool init_font(struct lsl_plugin* srcdata)
{
    FT_Long index = 0;

    std::string path = "Roboto-Black.ttf";
    return FT_New_Face(ft2_lib, path.c_str(), index, &(srcdata->font_face)) == 0;
}

static bool lsl_plugin_start(void* data)
{
    lsl_plugin* plugin = (lsl_plugin*)data;
    if (plugin->linked_lsl)
        return false;

    double framerate = video_output_get_frame_rate(obs_get_video());


    //tentative de separer la creation de l'inlet
	//using namespace lsl;
    //	std::vector<stream_info> results = resolve_stream("name",   "SimpleStream");
  	//plugin->inlet = lsl_create_inlet(results[0],300, LSL_NO_PREFERENCE, 1);


    plugin->linked_lsl = true;
    return true;
}

static bool lsl_plugin_stop( void* data)
{
    lsl_plugin* plugin = (lsl_plugin*)data;
    if (!plugin->linked_lsl)
        blog(LOG_INFO, "lsl-plugin already stopped");
        return true;
    if (plugin->outlet == NULL)
        blog(LOG_INFO, "lsl-plugin has no outlet to stop");
        return false;
    plugin->linked_lsl = false;

    plugin_pause = false;
    delete plugin->outlet;
    plugin->outlet = NULL;
    blog(LOG_INFO, "lsl-plugin stop");
    return true;
}

static obs_properties_t* lsl_plugin_properties(void* data) {
    //lsl_plugin* plugin_data = (lsl_plugin*)data;
    UNUSED_PARAMETER(data);

    obs_properties_t* props = obs_properties_create();



    obs_properties_add_text(props, S_NAME, T_NAME, OBS_TEXT_DEFAULT);
    obs_properties_add_text(props, S_ID, T_ID, OBS_TEXT_DEFAULT);

    obs_properties_add_bool(props, S_HASTEXT, T_HASTEXT);
    obs_properties_add_bool(props, S_AUTOCOLOR, T_AUTOCOLOR);
    obs_properties_add_int(props, S_FONTSIZE, T_FONTSIZE, 1, 100, 1);
    obs_properties_add_color(props, S_COLOR, T_COLOR);
    obs_properties_add_bool(props, S_OUTLINE, T_OUTLINE);
    obs_properties_add_bool(props, S_SHADOW, T_SHADOW);

    obs_properties_add_bool(props, S_LINK, T_LINK);

    return props;
}

static void lsl_plugin_destroy(void* data) {
    struct lsl_plugin* srcdata = (lsl_plugin*)data;

    lsl_plugin_stop(srcdata);

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

    if (srcdata->text != NULL)
        bfree(srcdata->text);
    if (srcdata->texbuf != NULL)
        bfree(srcdata->texbuf);
    if (srcdata->colorbuf != NULL)
        bfree(srcdata->colorbuf);

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

static void lsl_plugin_update(void* data, obs_data_t* settings)
{

    lsl_plugin* plugin_data = (lsl_plugin*)data;

    const char* id_name = obs_data_get_string(settings, S_ID);
    const char* name = obs_data_get_string(settings, S_NAME);




    //if (std::string(id_name)==std::string(plugin_data->id_name) || std::string(name) == std::string(plugin_data->name)) {// (strcmp(id_name,plugin_data->id_name)!=0 || strcmp(name, plugin_data->name) != 0) {
        if (!plugin_data->linked_lsl) {
            plugin_data->id_name = (char*)id_name;
            plugin_data->name = (char*)name;
        }
        else {

            //lsl_plugin_stop(plugin_data);
            //plugin_data->id_name = (char*)id_name;
            //plugin_data->name = (char*)name;
            //obs_data_set_bool(settings, S_LINK, false);
        }
    //}





    bool link_lsl = obs_data_get_bool(settings, S_LINK);

    if (link_lsl && !plugin_data->linked_lsl) {
        lsl_plugin_start(plugin_data);
    }
    else if (!link_lsl && plugin_data->linked_lsl) {
        lsl_plugin_stop(plugin_data);
    }

    if (obs_data_get_bool(settings, S_HASTEXT)) {

        int font_size = obs_data_get_int(settings, S_FONTSIZE);
        bool vbuf_needs_update = false;

        uint32_t color[2];


        plugin_data->outline_width = 0;

        plugin_data->drop_shadow = obs_data_get_bool(settings, S_SHADOW);
        plugin_data->outline_text =  obs_data_get_bool(settings, S_OUTLINE);

        if (plugin_data->outline_text && plugin_data->drop_shadow)
            plugin_data->outline_width = 6;
        else if (plugin_data->outline_text || plugin_data->drop_shadow)
            plugin_data->outline_width = 4;


      if(plugin_data->beat >100){
        plugin_data->color[0] = 0xFFFFFFFF;
        plugin_data->color[1] = 0xFFFFFFFF;
      }else{

        color[0] = (uint32_t)obs_data_get_int(settings, S_COLOR);
        color[1] = (uint32_t)obs_data_get_int(settings, S_COLOR);
      }


        if (color[0] != plugin_data->color[0] || color[1] != plugin_data->color[1]) {
            plugin_data->color[0] = color[0];
            plugin_data->color[1] = color[1];
            vbuf_needs_update = true;
        }

        if (ft2_lib == NULL) {
            obs_data_set_bool(settings, S_HASTEXT, false);
            plugin_data->text_enable = false;
            return;
        }


        const size_t texbuf_size = (size_t)texbuf_w * (size_t)texbuf_h;

        if (plugin_data->draw_effect == NULL) {
            blog(LOG_WARNING, " LSL plugin draw effect is null");
            char* effect_file = "text_default.effect";
            char* error_string = NULL;

            obs_enter_graphics();
            plugin_data->draw_effect = gs_effect_create_from_file(
                effect_file, &error_string);
            obs_leave_graphics();

            if (error_string != NULL)
                bfree(error_string);

        }


        if (plugin_data->font_size != font_size)
            vbuf_needs_update = true;

        const bool new_aa_setting = true;
        const bool aa_changed = plugin_data->antialiasing != new_aa_setting;
        if (aa_changed) {
            plugin_data->antialiasing = new_aa_setting;
            if (plugin_data->texbuf != NULL) {
                memset(plugin_data->texbuf, 0, texbuf_size);
            }
            cache_standard_glyphs(plugin_data);
        }

        if (plugin_data->font_face != NULL) {
            //if (font_size == plugin_data->font_size)
                //goto skip_font_load;

            plugin_data->max_h = 0;
            vbuf_needs_update = true;
        }

        plugin_data->font_size = font_size;

        if (!init_font(plugin_data) || plugin_data->font_face == NULL) {
            blog(LOG_WARNING, "FT2-text: Failed to load font ");
            obs_data_set_bool(settings, S_HASTEXT, false);
            plugin_data->text_enable = false;
            return;
        }
        else {
            if(FT_Set_Pixel_Sizes(plugin_data->font_face, 0, plugin_data->font_size)!=0)
                blog(LOG_WARNING,"problem setting text size");
            if (FT_Select_Charmap(plugin_data->font_face, FT_ENCODING_UNICODE)!=0)
                blog(LOG_WARNING, "problem setting char map");
        }
        if (plugin_data->texbuf != NULL) {
            bfree(plugin_data->texbuf);
            plugin_data->texbuf = NULL;
        }
        plugin_data->texbuf = (uint8_t*)bzalloc(texbuf_size);

        if (plugin_data->font_face)
            cache_standard_glyphs(plugin_data);

    skip_font_load:;

        plugin_data->text_enable = true;


    }
    else if (!obs_data_get_bool(settings, S_HASTEXT) && plugin_data->text_enable) {

        plugin_data->text_enable = false;

        /*
        for (uint32_t i = 0; i < num_cache_slots; i++) {
            if (plugin_data->cacheglyphs[i] != NULL) {
                bfree(plugin_data->cacheglyphs[i]);
                plugin_data->cacheglyphs[i] = NULL;
            }
        }


        if (plugin_data->texbuf != NULL)
            bfree(plugin_data->texbuf);
        if (plugin_data->colorbuf != NULL)
            bfree(plugin_data->colorbuf);
        */
        obs_enter_graphics();

        if (plugin_data->tex != NULL) {
            gs_texture_destroy(plugin_data->tex);
            plugin_data->tex = NULL;
        }
        if (plugin_data->vbuf != NULL) {
            gs_vertexbuffer_destroy(plugin_data->vbuf);
            plugin_data->vbuf = NULL;
        }

        if (plugin_data->draw_effect != NULL) {
            gs_effect_destroy(plugin_data->draw_effect);
            plugin_data->draw_effect = NULL;
        }

        obs_leave_graphics();


    }
    if (obs_data_get_bool(settings, S_AUTOCOLOR)) {
      plugin_data->autocolor = true;
    }else {
      plugin_data->autocolor = false;

    }


}

static const char* lsl_plugin_name(void* unused)
{
    UNUSED_PARAMETER(unused);
    return "Hbeat_text";// obs_module_text("LSL plugin");
}

static void lsl_plugin_video(void* data, gs_effect_t* effect) {
    UNUSED_PARAMETER(effect);
    lsl_plugin* srcdata = (lsl_plugin*)data;

    if (srcdata == NULL)
        return;

    if (srcdata->text_enable) {
        if (srcdata->tex == NULL || srcdata->vbuf == NULL)
            return;
        if (srcdata->text == NULL || *srcdata->text == 0)
            return;

        gs_reset_blend_state();

        if (srcdata->outline_text)
            draw_outlines(srcdata);

        if (srcdata->drop_shadow)
            draw_drop_shadow(srcdata);


        draw_uv_vbuffer(srcdata->vbuf, srcdata->tex, srcdata->draw_effect,
            (uint32_t)wcslen(srcdata->text) * 6);
    }

}

static void lsl_plugin_tick(void* data, float seconds) {
    lsl_plugin* f = (lsl_plugin*)data;

    if (f == NULL)
        return;



    f->frame_time = (double)((double)(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()) / 1000);

    if (f->outlet != NULL && f->linked_lsl) {

        obs_output_t* output = obs_frontend_get_recording_output();


    }


    if (f->text_enable) {

        //std::string ts_str = std::to_string(f->frame_time);
        std::string id_str = std::to_string(f->frame_number);

        int a;

      	using namespace lsl;
      		std::vector<stream_info> results = resolve_stream("name",   "SimpleStream");
      		stream_inlet inlet(results[0]);




      			 //receive data
      			std::vector<int> simple;

      			inlet.pull_sample(simple);
      	    f->beat = simple.front();
            std::string ts_str = std::to_string(f->beat);

          if (f->autocolor){
            if(f->beat >100){
              f->color[0] = 0xFFFFFFFF;
              f->color[1] = 0xFFFFFFFF;
            }
          }



        std::string txt = " Hearth Pulse: " + ts_str;
        std::wstring wtxt = std::wstring(txt.begin(), txt.end());
        f->text = (wchar_t*)wtxt.c_str();

        cache_glyphs(f, f->text);

        set_up_vertex_buffer(f);
    }
}

static void* lsl_plugin_create(obs_data_t* settings, obs_source_t* context) {

    struct lsl_plugin* data = (struct lsl_plugin*) bzalloc(sizeof(struct lsl_plugin));
    data->src = context;

    lsl_plugin_init();

    obs_data_set_default_string(settings, S_NAME, "OBS");
    obs_data_set_default_string(settings, S_ID, "ID1");
    obs_data_set_default_bool(settings, S_HASTEXT, false);
    obs_data_set_default_bool(settings, S_AUTOCOLOR, false);
    obs_data_set_default_bool(settings, S_LINK, false);
    obs_data_set_default_int(settings, S_FONTSIZE, 30);
    obs_data_set_default_int(settings, S_COLOR, 0xFFFFFFFF);
    obs_data_set_default_bool(settings, S_OUTLINE, false);
    obs_data_set_default_bool(settings, S_SHADOW, false);

    lsl_plugin_update(data, settings);
    return data;

}



struct obs_source_info  create_plugin_info() {
    struct obs_source_info  plugin_info = {};
    plugin_info.id = "Hbeat_text";
    plugin_info.type = OBS_SOURCE_TYPE_INPUT;
    plugin_info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW;
    plugin_info.get_name = lsl_plugin_name;
    plugin_info.create = lsl_plugin_create;
    plugin_info.destroy = lsl_plugin_destroy;
    plugin_info.update = lsl_plugin_update;
    plugin_info.get_properties = lsl_plugin_properties;
    plugin_info.video_render = lsl_plugin_video;
    plugin_info.video_tick = lsl_plugin_tick;
    plugin_info.get_width = lsl_plugin_get_width;
    plugin_info.get_height = lsl_plugin_get_height;

    return plugin_info;
}

void lsl_plugin_init() {
    if (plugin_initialized)
        return;


    FT_Init_FreeType(&ft2_lib);

    if (ft2_lib == NULL) {
        blog(LOG_WARNING, "FT2-text: Failed to initialize FT2.");
        return;
    }
    plugin_initialized = true;
}






uint32_t lsl_plugin_get_width(void* data)
{
    struct lsl_plugin* srcdata = (lsl_plugin*)data;

    return srcdata->cx + srcdata->outline_width;
}

uint32_t lsl_plugin_get_height(void* data)
{
    struct lsl_plugin* srcdata = (lsl_plugin*)data;

    return srcdata->cy + srcdata->outline_width;
}


void obs_module_unload(void){
    if (plugin_initialized) {
        FT_Done_FreeType(ft2_lib);
    }
}



bool obs_module_load(void)
{


    // link fonction to the output
    obs_source_info lsl_plugin = create_plugin_info();

    // register this source
    obs_register_source(&lsl_plugin);
    return true;
}
