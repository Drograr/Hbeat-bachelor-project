
set(Hbeat_text_SOURCES
    obs-convenience.cpp
    text-functionality.cpp
    Hbeat_text.cpp
    obs-convenience.h
    obs-frontend-api.h
    lsl_plugin.h)

add_library(Hbeat_text MODULE
    ${Hbeat_text_SOURCES})
target_link_libraries(Hbeat_text
    # TODO: PRIVATE ?
    LSL::lsl
    Threads::Threads
    Freetype::Freetype
    ${LIBOBS_LIBRARIES}
    ${OBS_FRONTEND_LIBRARIES}
    ${lsl-obs-plugin_PLATFORM_DEPS}
    ${FREETYPE_LIBRARIES})
