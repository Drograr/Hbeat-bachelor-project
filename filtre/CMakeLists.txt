cmake_minimum_required(VERSION 2.8 FATAL_ERROR) #TODO: check which version is really minimal
set(CMAKE_LEGACY_CYGWIN_WIN32 0)  # TODO Check if this is really needed

project("lsl-obs-plugin")


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



#set (CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -fpermissive")

# Native `std::thread`s still require a platform thread library.
# CMake can find and link to it with the `Threads` package (link your
# executable with `Threads::Threads` afterwards).
find_package(Threads REQUIRED)

# It seems libobs cmake to not use ALIAS, so must include dirs manually
include_directories(${LIBOBS_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS})

add_subdirectory(src)
