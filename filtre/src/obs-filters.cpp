#include <obs-module.h>
#include "obs-filters-config.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("Hbeat_obs-filters", "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
	return "Hbeat core filters";
}


extern struct obs_source_info Hbeat_scroll_filter;



bool obs_module_load(void)
{

	obs_register_source(&Hbeat_scroll_filter);

	return true;
}

#ifdef LIBNVAFX_ENABLED
void obs_module_unload(void)
{
	release_lib();
}
#endif
