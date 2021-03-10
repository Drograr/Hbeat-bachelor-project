
set(lsl-obs-plugin_SOURCES
    obs-convenience.cpp
    text-functionality.cpp
    lsl_plugin.cpp
    obs-convenience.h
    obs-frontend-api.h
    lsl_plugin.h)

add_library(lsl_plugin MODULE
    ${lsl-obs-plugin_SOURCES})
target_link_libraries(lsl_plugin
    # TODO: PRIVATE ?
    LSL::lsl
    Threads::Threads
    Freetype::Freetype
    ${LIBOBS_LIBRARIES}
    ${OBS_FRONTEND_LIBRARIES}
    ${lsl-obs-plugin_PLATFORM_DEPS}
    ${FREETYPE_LIBRARIES})
