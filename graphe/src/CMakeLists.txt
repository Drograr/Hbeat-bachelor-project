

set(Hbeat_graphe_SOURCES
	Hbeat_graphe.cpp)

#include_directories("usr/include/x86_64-linux-gnu/qt5/QtCore")

 include_directories(${Qt5Widgets_INCLUDES})
add_definitions(${Qt5Widgets_DEFINITIONS})
set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")


add_library(Hbeat_image MODULE

	${Hbeat_graphe_SOURCES})

target_link_libraries(Hbeat_image
	libobs
	LSL::lsl
	Qwt::Qwt
	#Qt5::Qt5Widgets
	qwt-qt5


	Threads::Threads
	Freetype::Freetype
	${image-source_PLATFORM_DEPS}
	${LIBOBS_LIBRARIES}
	${OBS_FRONTEND_LIBRARIES}
	${lsl-obs-plugin_PLATFORM_DEPS}
	${FREETYPE_LIBRARIES}
	${Qt5Widgets_LIBRARIES})
