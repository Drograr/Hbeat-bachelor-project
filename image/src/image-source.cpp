#include <obs-module.h>
#include <graphics/image-file.h>
#include <util/platform.h>
#include <util/dstr.h>
#include <sys/stat.h>
#include <stdio.h>
#include <obs.h>
#include <lsl_c.h>
#include <stdio.h>
#include <lsl_cpp.h>
#include <vector>
#include <obs.h>
#include <map>
#include <thread>
#include <algorithm>



#define SETTING_LSL_CHAN              "LSL_chan"

#define TEXT_LSL_CHAN                obs_module_text("LSL_chan")


#define blog(log_level, format, ...)                    \
	blog(log_level, "[image_source: '%s'] " format, \
	     obs_source_get_name(context->source), ##__VA_ARGS__)

#define debug(format, ...) blog(LOG_DEBUG, format, ##__VA_ARGS__)
#define info(format, ...) blog(LOG_INFO, format, ##__VA_ARGS__)
#define warn(format, ...) blog(LOG_WARNING, format, ##__VA_ARGS__)

struct image_source {
	obs_source_t *source;

	char *file;
	bool persistent;
	time_t file_timestamp;
	float update_time_elapsed;
	uint64_t last_time;
	bool active;
	const char *lsl_chan_name;

	gs_image_file2_t if2;
//	obs_property_t *p;
};

static time_t get_modified_timestamp(const char *filename)
{
	struct stat stats;
	if (os_stat(filename, &stats) != 0)
		return -1;
	return stats.st_mtime;
}

static const char *image_source_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return "Hbeat_image";//obs_module_text("ImageInput");
}

static void image_source_load(struct image_source *context)
{
	char *file = context->file;

	obs_enter_graphics();
	gs_image_file2_free(&context->if2);
	obs_leave_graphics();

	if (file && *file) {
		debug("loading texture '%s'", file);
		context->file_timestamp = get_modified_timestamp(file);
		gs_image_file2_init(&context->if2, file);
		context->update_time_elapsed = 0;

		obs_enter_graphics();
		gs_image_file2_init_texture(&context->if2);
		obs_leave_graphics();

		if (!context->if2.image.loaded)
			warn("failed to load texture '%s'", file);
	}
}

static void image_source_unload(struct image_source *context)
{
	obs_enter_graphics();
	gs_image_file2_free(&context->if2);
	obs_leave_graphics();
}

static void image_source_update(void *data, obs_data_t *settings)
{
	 image_source* context = (image_source*)data;
	const char *file = obs_data_get_string(settings, "file");
	const bool unload = obs_data_get_bool(settings, "unload");

	if (context->file)
		bfree(context->file);
	context->file = bstrdup(file);
	context->persistent = !unload;

	/* Load the image if the source is persistent or showing */
	if (context->persistent || obs_source_showing(context->source))
		image_source_load(context);
	else
		image_source_unload(context);


//	obs_property_list_add_string(context->p, obs_module_text("Green"), "green");
context->lsl_chan_name =
	obs_data_get_string(settings, SETTING_LSL_CHAN);
}

static void image_source_defaults(obs_data_t *settings)
{
	obs_data_set_default_bool(settings, "unload", false);
}

static void image_source_show(void *data)
{
	image_source* context = (image_source*)data;

	if (!context->persistent)
		image_source_load(context);
}

static void image_source_hide(void *data)
{
	image_source* context = (image_source*)data;

	if (!context->persistent)
		image_source_unload(context);
}

static void *image_source_create(obs_data_t *settings, obs_source_t *source)
{
	struct image_source* context = (image_source*) bzalloc(sizeof(struct image_source));
	context->source = source;

	image_source_update(context, settings);
	return context;
}

static void image_source_destroy(void *data)
{
	image_source* context = (image_source*)data;

	image_source_unload(context);

	if (context->file)
		bfree(context->file);
	bfree(context);
}

static uint32_t image_source_getwidth(void *data)
{
	image_source* context = (image_source*)data;
	return context->if2.image.cx;
}

static uint32_t image_source_getheight(void *data)
{
	image_source* context = (image_source*)data;
	return context->if2.image.cy;
}

static void image_source_render(void *data, gs_effect_t *effect)
{
	image_source* context = (image_source*)data;

	if (!context->if2.image.texture)
		return;

	//const bool linear_srgb = gs_get_linear_srgb();

	//const bool previous = gs_framebuffer_srgb_enabled();
	//gs_enable_framebuffer_srgb(linear_srgb);

	gs_eparam_t *const param = gs_effect_get_param_by_name(effect, "image");
	//if (linear_srgb)
		//gs_effect_set_texture_srgb(param, context->if2.image.texture);
	//else
		gs_effect_set_texture(param, context->if2.image.texture);

	gs_draw_sprite(context->if2.image.texture, 0, context->if2.image.cx,
		       context->if2.image.cy);

	//gs_enable_framebuffer_srgb(previous);
}

static void image_source_tick(void *data, float seconds)
{
	image_source* context = (image_source*)data;
	uint64_t frame_time = obs_get_video_frame_time();

	context->update_time_elapsed += seconds;

	if (obs_source_showing(context->source)) {
		if (context->update_time_elapsed >= 1.0f) {
			time_t t = get_modified_timestamp(context->file);
			context->update_time_elapsed = 0.0f;

			if (context->file_timestamp != t) {
				image_source_load(context);
			}
		}
	}

	if (obs_source_active(context->source)) {
		if (!context->active) {
			if (context->if2.image.is_animated_gif)
				context->last_time = frame_time;
			context->active = true;
		}

	} else {
		if (context->active) {
			if (context->if2.image.is_animated_gif) {
				context->if2.image.cur_frame = 0;
				context->if2.image.cur_loop = 0;
				context->if2.image.cur_time = 0;

				obs_enter_graphics();
				gs_image_file2_update_texture(&context->if2);
				obs_leave_graphics();
			}

			context->active = false;
		}

		return;
	}

	if (context->last_time && context->if2.image.is_animated_gif) {
		uint64_t elapsed = frame_time - context->last_time;
		bool updated = gs_image_file2_tick(&context->if2, elapsed);

		if (updated) {
			obs_enter_graphics();
			gs_image_file2_update_texture(&context->if2);
			obs_leave_graphics();
		}
	}




	std::vector<lsl::stream_info> results = lsl::resolve_streams();

	std::map<std::string, lsl::stream_info> found_streams;
	// display them
	int i;
	i = 0;
	for (auto &stream : results) {
		found_streams.emplace(std::make_pair(stream.uid(), stream));
		if (strcmp(context->lsl_chan_name, stream.name().c_str()) == 0)
		{
			int a;
			using namespace lsl;

				stream_inlet inlet(results[i]);




					 //receive data
				std::vector<int> simple;

				inlet.pull_sample(simple);
				a =  simple.front();
				context->if2.image.cy = a;
				context->if2.image.cx = a;
		}
		i = i +1;
	}




	context->last_time = frame_time;
}

static const char *image_filter =
	"All formats (*.bmp *.tga *.png *.jpeg *.jpg *.gif *.psd *.webp);;"
	"BMP Files (*.bmp);;"
	"Targa Files (*.tga);;"
	"PNG Files (*.png);;"
	"JPEG Files (*.jpeg *.jpg);;"
	"GIF Files (*.gif);;"
	"PSD Files (*.psd);;"
	"WebP Files (*.webp);;"
	"All Files (*.*)";

static obs_properties_t *image_source_properties(void *data)
{
	 image_source* s = (image_source*)data;
	struct dstr path = {0};

	obs_properties_t *props = obs_properties_create();

	//string lsl_name;

	if (s && s->file && *s->file) {
		const char *slash;

		dstr_copy(&path, s->file);
		dstr_replace(&path, "\\", "/");
		slash = strrchr(path.array, '/');
		if (slash)
			dstr_resize(&path, slash - path.array + 1);
	}

	obs_properties_add_path(props, "file", obs_module_text("File"),
				OBS_PATH_FILE, image_filter, path.array);
	obs_properties_add_bool(props, "unload",
				obs_module_text("UnloadWhenNotShowing"));
	dstr_free(&path);

	obs_property_t *p = obs_properties_add_list(props, SETTING_LSL_CHAN,
								TEXT_LSL_CHAN,
								OBS_COMBO_TYPE_LIST,
								OBS_COMBO_FORMAT_STRING);

	std::vector<lsl::stream_info> results = lsl::resolve_streams();

	std::map<std::string, lsl::stream_info> found_streams;
								// display them
	for (auto &stream : results) {
				found_streams.emplace(std::make_pair(stream.uid(), stream));
					 //lsl_name = stream.name();

	obs_property_list_add_string(p, stream.name().c_str(), stream.name().c_str());
  }
	return props;
}

uint64_t image_source_get_memory_usage(void *data)
{
image_source* s = (image_source*)data;
	return s->if2.mem_usage;
}

static void missing_file_callback(void *src, const char *new_path, void *data)
{
	image_source* s = (image_source*)src;

	obs_source_t *source = s->source;
	obs_data_t *settings = obs_source_get_settings(source);
	obs_data_set_string(settings, "file", new_path);
	obs_source_update(source, settings);
	obs_data_release(settings);

	UNUSED_PARAMETER(data);
}




 struct obs_source_info create_plugin_info() {
	 struct obs_source_info plugin_info = {};
	plugin_info.id = "Hbeat_image";
	plugin_info.type = OBS_SOURCE_TYPE_INPUT;
	plugin_info.output_flags = OBS_SOURCE_VIDEO;
	plugin_info.get_name = image_source_get_name;
	plugin_info.create = image_source_create;
	plugin_info.destroy = image_source_destroy;
	plugin_info.update = image_source_update;
	plugin_info.get_defaults = image_source_defaults;
	plugin_info.show = image_source_show;
	plugin_info.hide = image_source_hide;
	plugin_info.get_width = image_source_getwidth;
	plugin_info.get_height = image_source_getheight;
	plugin_info.video_render = image_source_render;
	plugin_info.video_tick = image_source_tick;

	plugin_info.get_properties = image_source_properties;
	plugin_info.icon_type = OBS_ICON_TYPE_IMAGE;
	return plugin_info;
}

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("image-source", "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
	return "Image/color/slideshow sources";
}

extern struct obs_source_info slideshow_info;
extern struct obs_source_info color_source_info_v1;
extern struct obs_source_info color_source_info_v2;
extern struct obs_source_info color_source_info_v3;

bool obs_module_load(void)
{
	obs_source_info Hbeat_image_info = create_plugin_info();
	obs_register_source(&Hbeat_image_info);
	obs_register_source(&color_source_info_v1);
	obs_register_source(&color_source_info_v2);
	obs_register_source(&color_source_info_v3);
	obs_register_source(&slideshow_info);
	return true;
}
