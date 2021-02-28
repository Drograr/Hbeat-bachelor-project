#include <obs-internal.h>
#include <obs-module.h>
#include "win-serial-reader.h"
#include <lsl_cpp.h>
#include <vector>
#include <iostream>

#define blog(log_level, format, ...) \
	blog(log_level, "[pulse_sensor: '%s'] " format, \
			obs_source_get_name(sensor->source), ##__VA_ARGS__)

struct pulse_sensor {
	obs_source_t *source;
	uint32_t     cx;
	uint32_t     cy;
	obs_source_t *textSource;
	float        update_time_elapsed;
	struct pulse_data *sensor_data;
	pthread_t thread;
	bool showTopHeartRate;
//	const char *comPort;
	volatile bool updateThread;
};

// This is the sensor data which is fetched through a separate thread.
struct pulse_data {
	int bpm;
	//int signal;
	//int IBI;
	int bpmTopRecord;
};

void* read_bpm_thread(struct pulse_sensor *sensor) {
	//HANDLE port = open_com_port(sensor->comPort);

//	if (port == NULL) {
	//	sensor->updateThread = false;
		//blog(LOG_WARNING, "Failed connect.");
		//return NULL;
	//}

	std::vector<stream_info> results = resolve_stream("name", argc > 1 ? argv[1] : "SimpleStream");
	stream_inlet inlet(results[0]);

	while (sensor->updateThread) {
		using namespace lsl;

		// resolve the stream of interest & make an inlet to get data from the first result
		std::vector<stream_info> results = resolve_stream("name", argc > 1 ? argv[1] : "SimpleStream");
		stream_inlet inlet(results[0]);

		// receive data & time stamps forever (not displaying them here)
		std::vector<int> sample;
		while (true) {
		inlet.pull_sample(sample);
		bpm = sample.front();
	}

	if (!close_com_port(port)) {
		blog(LOG_WARNING, "Failed to close COM port.");
	}

	return NULL;
}

static void start_thread(struct pulse_sensor *sensor) {
	sensor->updateThread = true;
	if (pthread_create(&sensor->thread, NULL, read_bpm_thread, sensor)) {
		blog(LOG_WARNING, "Failed to create thread for reading  data.");
	}
}

static void stop_serial_thread(struct pulse_sensor *sensor) {
	sensor->updateThread = false;
	if (pthread_join(sensor->thread, NULL)) {
		blog(LOG_WARNING, "Failed to join  thread.");
	}
}

static void pulse_sensor_defaults(obs_data_t *settings) {
	obs_data_set_default_bool(settings, "unload", true);
	obs_data_set_default_bool(settings, "topheartrate", false);
//	obs_data_set_default_string(settings, "comport", "COM4");
}

static void *pulse_sensor_create(obs_data_t *settings, obs_source_t *source) {
	struct pulse_sensor *sensor = bzalloc(sizeof(struct pulse_sensor));
	sensor->sensor_data = bzalloc(sizeof(struct pulse_data));
	sensor->source = source;
	const char *text_source_id = "text_ft2_source\0";
	//sensor->comPort = obs_data_get_string(settings, "comport");

	sensor->textSource = obs_source_create(text_source_id, text_source_id, settings, NULL);
	obs_source_add_active_child(sensor->source, sensor->textSource);

	return sensor;
}

void enum_active_sources(void *data, obs_source_enum_proc_t enum_callback, void *param) {
	struct pulse_sensor *context = data;

	enum_callback(context->source, context->textSource, param);
}

static uint32_t pulse_sensor_getwidth(void *data) {
	struct pulse_sensor *sensor = data;
	return obs_source_get_width(sensor->textSource);
}

static uint32_t pulse_sensor_getheight(void *data) {
	struct pulse_sensor *sensor = data;
	return obs_source_get_height(sensor->textSource);
}

static void pulse_sensor_tick(void *data, float seconds) {
	struct pulse_sensor *sensor = data;

	if (!obs_source_showing(sensor->source))
		return;

	sensor->update_time_elapsed += seconds;

	if (sensor->update_time_elapsed >= 0.1f) {
		sensor->update_time_elapsed = 0.0f;

		char bpmBuffer[1024];
		char bpmTopRecordBuffer[1024];

		itoa(sensor->sensor_data->bpm, bpmBuffer, 10);
		itoa(sensor->sensor_data->bpmTopRecord, bpmTopRecordBuffer, 10);


		if (sensor->showTopHeartRate) {
			snprintf(bpmBuffer, sizeof(bpmBuffer), "%s\r\n%s", bpmBuffer, bpmTopRecordBuffer);
		}

		if (sensor->sensor_data->bpm > sensor->sensor_data->bpmTopRecord) {
			sensor->sensor_data->bpmTopRecord = sensor->sensor_data->bpm;
		}

	//	bool noSensorData = sensor->sensor_data->bpm == 0 &&
		//	sensor->sensor_data->signal == 0 &&
		//	sensor->sensor_data->IBI == 0;


		bool abovePulseLimit = sensor->sensor_data->bpm > MAX_BPM;

		if ( abovePulseLimit || !sensor->updateThread) {
			obs_data_set_string(sensor->textSource->context.settings, "text", "N/A");
			sensor->sensor_data->bpmTopRecord = 0;
		}
		else {
			obs_data_set_string(sensor->textSource->context.settings, "text", bpmBuffer);
		}

		obs_source_update(sensor->textSource, sensor->textSource->context.settings);
	}
}

static void pulse_sensor_render(void *data, gs_effect_t *effect) {
	struct pulse_sensor *sensor = data;

	obs_source_video_render(sensor->textSource);
}

static const char *pulse_sensor_amped_get_name(void *unused) {
	UNUSED_PARAMETER(unused);
	return obs_module_text("Hbeat");
}

static void pulse_sensor_show(void *data) {

	struct pulse_sensor *sensor = data;
	start_thread(sensor);
}

static void pulse_sensor_update(void *data, obs_data_t *settings) {
	struct pulse_sensor *sensor = data;

	stop_serial_thread(sensor);

	obs_data_set_string(sensor->textSource->context.settings, "text", "N/A");

	sensor->showTopHeartRate = obs_data_get_bool(settings, "topheartrate");
//	sensor->comPort = obs_data_get_string(settings, "comport");

	start_thread(sensor);
}

static void pulse_sensor_destroy(void *data) {
	struct pulse_sensor *sensor = data;

	stop_serial_thread(sensor);

	bfree(sensor->sensor_data);
	sensor->sensor_data = NULL;

	obs_source_remove(sensor->textSource);
	obs_source_release(sensor->textSource);
	sensor->textSource = NULL;

	bfree(sensor);
}

static obs_properties_t *pulse_sensor_properties(void *unused) {
	struct pulse_sensor *sensor = unused;

	obs_properties_t *props = obs_source_properties(sensor->textSource);

	//obs_properties_add_text(props, "comport", "COM port", OBS_TEXT_DEFAULT);

	obs_properties_add_bool(props, "topheartrate", "Show top heart rate record");

	return props;
}

static struct obs_source_info pulse_sensor_info = {
	.id             = "pulse-sensor",
	.type           = OBS_SOURCE_TYPE_INPUT,
	.output_flags   = OBS_SOURCE_VIDEO,
	.get_name       = pulse_sensor_amped_get_name,
	.create         = pulse_sensor_create,
	.destroy        = pulse_sensor_destroy,
	.update         = pulse_sensor_update,
	.get_defaults   = pulse_sensor_defaults,
	.show           = pulse_sensor_show,
	.get_width      = pulse_sensor_getwidth,
	.get_height     = pulse_sensor_getheight,
	.video_render   = pulse_sensor_render,
	.video_tick     = pulse_sensor_tick,
	.get_properties = pulse_sensor_properties,
	.enum_active_sources = enum_active_sources
};


OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("win-pulse-sensor-amped", "en-US")

bool obs_module_load(void)
{
	obs_register_source(&pulse_sensor_info);
	return true;
}
