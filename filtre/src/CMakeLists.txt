








if(MSVC)
	set(obs-filters_PLATFORM_DEPS
		w32-pthreads)
endif()

set(obs-filters_SOURCES
	scroll-filter.cpp
	)



add_library(obs-filters_Hbeat MODULE
	${rnnoise_SOURCES}
	${obs-filters_SOURCES}
	)
target_link_libraries(obs-filters_Hbeat
	libobs
	${obs-filters_PLATFORM_DEPS}
	${LIBOBS_LIBRARIES}
	${OBS_FRONTEND_LIBRARIES})
set_target_properties(obs-filters_Hbeat PROPERTIES FOLDER "plugins")
