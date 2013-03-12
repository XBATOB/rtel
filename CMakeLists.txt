# $Id: CMakeLists.txt 2212 2010-06-24 13:18:03Z xbatob $
cmake_minimum_required(VERSION 2.6)
project(rtel C)
set(catalogname rtel)

option(WITH_LOGGING "enable logging of device data" ON)
option(WITH_FILTER "enable data filters" ON)
option(ENABLE_NLS "enable translations" ON)

include(CheckLibraryExists)
check_library_exists(readline readline "" HAVE_LIBREADLINE)

add_definitions(-D_GNU_SOURCE -DHAVE_CONFIG_H)
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
)

configure_file(
    ${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_BINARY_DIR}/config.h
)

add_subdirectory(src)
if(ENABLE_NLS)
    add_subdirectory(po)
endif(ENABLE_NLS)
