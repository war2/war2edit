cmake_minimum_required(VERSION 2.8)
project(war2edit C)

add_definitions(-g)
add_definitions(
    -Wall
    -Wextra
    -Winit-self
    -Wfloat-equal
    -Wunused
    -Wunused-parameter
    #-Wsign-conversion
    -Wchar-subscripts
    -Wdouble-promotion
    -Wformat-security
    -Wformat-nonliteral
    -Wmissing-braces
    -Wmissing-include-dirs
    -Wparentheses
    -Wsequence-point
    -Wtrigraphs
    -Wmaybe-uninitialized
    -Wsuggest-final-types
    -Wsuggest-final-methods
    #-Wconversion
    -Wlogical-op
    -Wlogical-not-parentheses
    -Wmemset-transposed-args
    -Wsizeof-pointer-memaccess
    -Wmissing-field-initializers
    -Wmultichar
    #-Wpacked
    #-Wpacked-bitfield-compat
    #-Wpadded
    #-Wredundant-decls
    -Winline
    -Winvalid-pch
    -Woverlength-strings
    -Wcast-align
)
add_definitions(-std=gnu11)
add_definitions(-D_BSD_SOURCE)
add_definitions(-D_DEFAULT_SOURCE)

set(PROJECT_DESCRIPTION "Warcraft II Map Editor")
set(PROJECT_URL "http://www.jeanguyomarch.com/war2edit.php")

set(CMAKE_COLOR_MAKEFILE   ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_MACOSX_RPATH OFF)

find_package(PkgConfig)

pkg_check_modules(EINA eina REQUIRED)
pkg_check_modules(EET eet REQUIRED)
pkg_check_modules(EVAS evas REQUIRED)
pkg_check_modules(ECORE ecore REQUIRED)
pkg_check_modules(ECORE_FILE ecore-file REQUIRED)
pkg_check_modules(ELEMENTARY elementary REQUIRED)
pkg_check_modules(ELM_BITMAP elm-bitmap REQUIRED)
pkg_check_modules(LIBPUD pud REQUIRED)
pkg_check_modules(LIBWAR2 war2 REQUIRED)

include(cmake/copy_file.cmake)

add_subdirectory(bin)
add_subdirectory(data)

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake)

