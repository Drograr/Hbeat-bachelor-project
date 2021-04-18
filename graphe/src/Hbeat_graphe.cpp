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
#include <qwt_plot.h>
#include <qwt_plot_curve.h>

#define blog(log_level, format, ...)                    \
	blog(log_level, "[graphe_source: '%s'] " format, \
	     obs_source_get_name(context->source), ##__VA_ARGS__)

#define debug(format, ...) blog(LOG_DEBUG, format, ##__VA_ARGS__)
#define info(format, ...) blog(LOG_INFO, format, ##__VA_ARGS__)
#define warn(format, ...) blog(LOG_WARNING, format, ##__VA_ARGS__)

struct graphe_source {
	obs_source_t *source;

	char *file;
	bool persistent;
	time_t file_timestamp;
	float update_time_elapsed;
	uint64_t last_time;
	bool active;
	double x[10];
	double y[10];
	QwtPlot *myplot;
	QwtPlotCurve *curve;


	uint32_t cx=100,cy=100;
	// if2;
};

static time_t get_modified_timestamp(const char *filename)
{
	struct stat stats;
	if (os_stat(filename, &stats) != 0)
		return -1;
	return stats.st_mtime;
}

static const char *graphe_source_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return "Hbeat_graphe";//obs_module_text("ImageInput");
}





static void graphe_source_update(void *data, obs_data_t *settings)
{
	 graphe_source* context = (graphe_source*)data;


















//context->myplot->show();






	/* Load the image if the source is persistent or showing */
	//if (context->persistent || obs_source_showing(context->source))
		//graphe_source_load(context);
	//else
		//graphe_source_unload(context);
}

static void graphe_source_defaults(obs_data_t *settings)
{
	//obs_data_set_default_bool(settings, "unload", false);
}



static void *graphe_source_create(obs_data_t *settings, obs_source_t *source)
{
	struct graphe_source* context = (graphe_source*) bzalloc(sizeof(struct graphe_source));
	context->source = source;
	context->persistent = true;
	context->myplot = new QwtPlot;


	context->curve =  new QwtPlotCurve("curve1");

	context->myplot->show();
	context->x[0] = 1;
	context->x[1] = 2;
	context->x[2] = 3;
	context->x[3] = 4;
	context->x[4] = 5;
	context->x[5] = 6;
	context->x[6] = 7;
	context->x[7] = 8;
	context->x[8] = 9;
	context->x[9] = 10;
	context->y[0] = 1;
	context->y[1] = 2;
	context->y[2] = 3;
	context->y[3] = 4;
	context->y[4] = 5;
	context->y[5] = 6;
	context->y[6] = 7;
	context->y[7] = 8;
	context->y[8] = 9;
	context->y[9] = 10;

	context->curve->setSamples(context->x,context->y,10);
	context->curve->attach(context->myplot);
	context->myplot->replot();







	graphe_source_update(context, settings);
	return context;
}

static void graphe_source_destroy(void *data)
{
	graphe_source* context = (graphe_source*)data;


	bfree(context);
}

static uint32_t graphe_source_getwidth(void *data)
{
	graphe_source* context = (graphe_source*)data;
	return context-> cx;
}

static uint32_t graphe_source_getheight(void *data)
{
	graphe_source* context = (graphe_source*)data;
	return context-> cy;
}

static void graphe_source_render(void *data, gs_effect_t *effect)
{
	graphe_source* context = (graphe_source*)data;

	context->myplot->show();
	




}

static void graphe_source_tick(void *data, float seconds)
{
	graphe_source* context = (graphe_source*)data;
	uint64_t frame_time = obs_get_video_frame_time();

	context->update_time_elapsed += seconds;

	if (obs_source_showing(context->source)) {
		if (context->update_time_elapsed >= 1.0f) {
			time_t t = get_modified_timestamp(context->file);
			context->update_time_elapsed = 0.0f;

			if (context->file_timestamp != t) {
				//graphe_source_load(context);
			}
		}
	}



	context->x[0] = 1;
	context->x[1] = 2;
	context->x[2] = 3;
	context->x[3] = 4;
	context->x[4] = 5;
	context->x[5] = 6;
	context->x[6] = 7;
	context->x[7] = 8;
	context->x[8] = 9;
	context->x[9] = 10;
	context->y[0] = context->y[1];
	context->y[1] = context->y[2];
	context->y[2] = context->y[3];
	context->y[3] = context->y[4];
	context->y[4] = context->y[5];
	context->y[5] = context->y[6];
	context->y[6] = context->y[7];
	context->y[7] = context->y[8];
	context->y[8] = context->y[9];
	context->y[9] = context->y[0];

	context->curve->setSamples(context->x,context->y,10);
	context->curve->attach(context->myplot);
	context->myplot->replot();









	context->last_time = frame_time;
}



static obs_properties_t *graphe_source_properties(void *data)
{
	 graphe_source* s = (graphe_source*)data;


	obs_properties_t *props = obs_properties_create();




	return props;
}








 struct obs_source_info create_plugin_info() {
	 struct obs_source_info plugin_info = {};
	plugin_info.id = "Hbeat_graphe";
	plugin_info.type = OBS_SOURCE_TYPE_INPUT;
	plugin_info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW;
	plugin_info.get_name = graphe_source_get_name;
	plugin_info.create = graphe_source_create;
	plugin_info.destroy = graphe_source_destroy;
	plugin_info.update = graphe_source_update;
	plugin_info.get_defaults = graphe_source_defaults;
	plugin_info.get_width = graphe_source_getwidth;
	plugin_info.get_height = graphe_source_getheight;
	plugin_info.video_render = graphe_source_render;
	plugin_info.video_tick = graphe_source_tick;

	plugin_info.get_properties = graphe_source_properties;
	//plugin_info.icon_type = OBS_ICON_TYPE_IMAGE;
	return plugin_info;
}

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("graphe-source", "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
	return "graphe_lsl";
}



bool obs_module_load(void)
{
	obs_source_info Hbeat_image_info = create_plugin_info();
	obs_register_source(&Hbeat_image_info);

	return true;
}
