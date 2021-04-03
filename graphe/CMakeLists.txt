cmake_minimum_required(VERSION 2.8 FATAL_ERROR) #TODO: check which version is really minimal
set(CMAKE_LEGACY_CYGWIN_WIN32 0)  # TODO Check if this is really needed

project("lsl-obs-plugin")

find_package(Qt5Widgets REQUIRED)

find_package(Freetype REQUIRED)
# Find OBS depedencies
find_package(LibObs REQUIRED)
if(WIN32)
    # TODO: this is not ideal has the first version of the lib found will be used
    # We should try to make it so that the correct version of the lib is used when needed (Debug, Release, etc.)
    find_library(OBS_FRONTEND_LIBRARIES
        NAMES "obs-frontend-api"
        PATHS "${LibObs_DIR}/../UI/obs-frontend-api/Debug"
        "${LibObs_DIR}/../UI/obs-frontend-api/Release"
        "${LibObs_DIR}/../UI/obs-frontend-api/RelWithDebInfo"
        "${LibObs_DIR}/../UI/obs-frontend-api/MinSizeRel"
        NO_DEFAULT_PATH)
    if(${OBS_FRONTEND_LIBRARIES} STREQUAL "OBS_FRONTEND_LIBRARIES-NOTFOUND")
        message(FATAL_ERROR "Obs frontend library not found, confirm that it is compiled")
    else()
        message(STATUS "Obs frontend library found at: ${OBS_FRONTEND_LIBRARIES} ")
    endif()
endif()

# Find LSL depedencies
find_package(LSL REQUIRED
    HINTS ${LSL_INSTALL_ROOT}
    "${CMAKE_CURRENT_LIST_DIR}/../../LSL/liblsl/build/install"
    "${CMAKE_CURRENT_LIST_DIR}/../../LSL/liblsl/out/install/x64-Release"
    PATH_SUFFIXES share/LSL)
get_filename_component(LSL_PATH ${LSL_CONFIG} DIRECTORY)
message(STATUS "Found LSL lib in ${LSL_PATH}")
LSLAPP_Setup_Boilerplate()

# Find Freetype2 dependencies
if(WIN32)
    set(OBSDEPS_DIR "OBSDEPS-NOTFOUND" CACHE PATH "path to the obs dependencies")
    if(${OBSDEPS_DIR} STREQUAL "OBSDEPS-NOTFOUND")
        message(FATAL_ERROR "OBSDEPS_DIR not found, please specify OBSDEPS_DIR path")
    else()
        find_library(FREETYPE_LIBRARIES
            NAMES "freetype"
            PATHS "${OBSDEPS_DIR}/bin"
            NO_DEFAULT_PATH)
        if(${FREETYPE_LIBRARIES} STREQUAL "FREETYPE_LIBRARIES-NOTFOUND")
            message(FATAL_ERROR "FREETYPE_LIBRARIES not found in the specified OBSDEPS_DIR path")
        else()
            message(STATUS "FREETYPE_LIBRARIES found at: ${FREETYPE_LIBRARIES} ")
            set(FREETYPE_INCLUDE_DIRS "${OBSDEPS_DIR}/include")
        endif()
    endif()
endif()

#set (CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -fpermissive")

# Native `std::thread`s still require a platform thread library.
# CMake can find and link to it with the `Threads` package (link your
# executable with `Threads::Threads` afterwards).
find_package(Threads REQUIRED)

# It seems libobs cmake to not use ALIAS, so must include dirs manually
include_directories(${LIBOBS_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS})

####################################
if (Qt5Gui_FOUND)
  get_target_property(QT_INCLUDE_DIR Qt5::Gui INTERFACE_INCLUDE_DIRECTORIES)
endif ()

find_path ( QWT_INCLUDE_DIR
  NAMES qwt_plot.h
  HINTS ${QT_INCLUDE_DIR}
  PATH_SUFFIXES qwt qwt-qt5 qwt6
)

set ( QWT_INCLUDE_DIRS ${QWT_INCLUDE_DIR} )

# version
set ( _VERSION_FILE ${QWT_INCLUDE_DIR}/qwt_global.h )
if ( EXISTS ${_VERSION_FILE} )
  file ( STRINGS ${_VERSION_FILE} _VERSION_LINE REGEX "define[ ]+QWT_VERSION_STR" )
  if ( _VERSION_LINE )
    string ( REGEX REPLACE ".*define[ ]+QWT_VERSION_STR[ ]+\"([^\"]*)\".*" "\\1" QWT_VERSION_STRING "${_VERSION_LINE}" )
  endif ()
endif ()
unset ( _VERSION_FILE )

find_library ( QWT_LIBRARY
  NAMES qwt qwt-qt5
  HINTS ${QT_LIBRARY_DIR}
)

set ( QWT_LIBRARIES ${QWT_LIBRARY} )

include ( FindPackageHandleStandardArgs )
find_package_handle_standard_args( Qwt REQUIRED_VARS QWT_LIBRARY QWT_INCLUDE_DIR VERSION_VAR QWT_VERSION_STRING )

if (Qwt_FOUND AND NOT TARGET Qwt::Qwt)
  add_library(Qwt::Qwt UNKNOWN IMPORTED)
  set_target_properties(Qwt::Qwt PROPERTIES
                        INTERFACE_INCLUDE_DIRECTORIES "${QWT_INCLUDE_DIRS}"
                        IMPORTED_LOCATION "${QWT_LIBRARIES}")
endif ()

mark_as_advanced (
  QWT_LIBRARY
  QWT_INCLUDE_DIR
)


####################################

add_subdirectory(src)
