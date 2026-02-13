cmake_minimum_required (VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# LTO
include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)
if (IPO_SUPPORTED)
    set(GUI_ENABLE_LTO ON)
else()
    set(GUI_ENABLE_LTO OFF)
endif()

# GUI LIB OPTION
set(GRAPHICS_LIBRARY "SDL" CACHE STRING "Graphics backend: SDL or SFML")
set_property(CACHE GRAPHICS_LIBRARY PROPERTY STRINGS SDL SFML)


project(Chess LANGUAGES CXX)
include(cmake/dependencies.cmake)
add_subdirectory ("Chess")
