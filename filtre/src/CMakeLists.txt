



set(obs-filters_SOURCES
	scroll-filter.cpp
	)



add_library(obs-filters_Hbeat MODULE
	${rnnoise_SOURCES}
	${obs-filters_SOURCES}
	${obs-filters_config_HEADERS})
target_link_libraries(obs-filters_Hbeat
	libobs
	${obs-filters_PLATFORM_DEPS})
set_target_properties(obs-filters_Hbeat PROPERTIES FOLDER "plugins")
