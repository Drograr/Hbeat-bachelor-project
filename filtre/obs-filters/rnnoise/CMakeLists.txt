

set(obs-filters_SOURCES
	obs-filters.c
	color-correction-filter.c
	async-delay-filter.c
	gpu-delay.c
	crop-filter.c
	scale-filter.c
	scroll-filter.c
	chroma-key-filter.c
	color-key-filter.c
	color-grade-filter.c
	sharpness-filter.c
	gain-filter.c
	noise-gate-filter.c
	mask-filter.c
	invert-audio-polarity.c
	compressor-filter.c
	limiter-filter.c
	expander-filter.c
	luma-key-filter.c)



add_library(obs-filters MODULE
	${rnnoise_SOURCES}
	${obs-filters_SOURCES}
	${obs-filters_config_HEADERS}
	${obs-filters_NOISEREDUCTION_SOURCES}
	${obs-filters_NOISEREDUCTION_HEADERS})
target_link_libraries(obs-filters
	libobs
	${obs-filters_PLATFORM_DEPS}
	${obs-filters_NOISEREDUCTION_LIBRARIES})
set_target_properties(obs-filters PROPERTIES FOLDER "plugins")
