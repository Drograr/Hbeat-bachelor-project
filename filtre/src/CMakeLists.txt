


if(DISABLE_SPEEXDSP)
	message(STATUS "SpeexDSP support disabled")
	set(LIBSPEEXDSP_FOUND FALSE)
else()
	find_package(Libspeexdsp)
	if(LIBSPEEXDSP_FOUND)
		add_definitions(-DLIBSPEEXDSP_ENABLED)
	endif()
endif()

find_package(Librnnoise QUIET)
add_definitions(-DLIBRNNOISE_ENABLED)
if(NOT LIBRNNOISE_FOUND)
	file(GLOB rnnoise_SOURCES
		"rnnoise/src/*.c"
		"rnnoise/src/*.h"
		"rnnoise/include/*.h")
	add_definitions(-DCOMPILE_OPUS)
	if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
		set_property(SOURCE ${rnnoise_SOURCES} PROPERTY COMPILE_FLAGS "-fvisibility=protected")
	endif()
	include_directories("rnnoise/include")
	source_group("rnnoise" FILES ${rnnoise_SOURCES})
	set(LIBRNNOISE_FOUND TRUE)
endif()

if (WIN32)
	if(DISABLE_NVAFX)
		message(STATUS "NVidia Audio FX support disabled")
		set(LIBNVAFX_FOUND FALSE)
	else()
		message(STATUS "NVidia Audio FX support enabled; requires redist to be installed by end-user")
		add_definitions(-DLIBNVAFX_ENABLED)
		set(LIBNVAFX_FOUND TRUE)
	endif()
endif()

if(LIBSPEEXDSP_FOUND OR LIBRNNOISE_FOUND OR LIBNVAFX_FOUND)
	set(obs-filters_NOISEREDUCTION_SOURCES
		noise-suppress-filter.c)
	if(LIBNVAFX_FOUND)
		set(obs-filters_NOISEREDUCTION_HEADERS
			nvafx-load.h)
	else()
		set(obs-filters_NOISEREDUCTION_HEADERS)
	endif()
	set(obs-filters_NOISEREDUCTION_LIBRARIES
		${LIBSPEEXDSP_LIBRARIES} ${LIBRNNOISE_LIBRARIES})
	set(NOISEREDUCTION_ENABLED TRUE)
else()
	set(NOISEREDUCTION_ENABLED FALSE)
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/obs-filters-config.h.in"
	"${CMAKE_BINARY_DIR}/plugins/obs-filters/config/obs-filters-config.h")

set(obs-filters_config_HEADERS
	"${CMAKE_BINARY_DIR}/plugins/obs-filters/config/obs-filters-config.h")
include_directories(${LIBSPEEXDSP_INCLUDE_DIRS} ${LIBRNNOISE_INCLUDE_DIRS}
	"${CMAKE_BINARY_DIR}/plugins/obs-filters/config")

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
