cmake_minimum_required(VERSION 3.0.2)
project(war2edit
   LANGUAGES C
   VERSION 0.1.0)

include(CheckCCompilerFlag)
include(cmake/check_compiler_warnings.cmake)
include(CPack)

add_definitions(-g) # Bad
check_compiler_warnings(
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
add_definitions(${COMPILER_WARNINGS})

set(PROJECT_DESCRIPTION "Warcraft II Map Editor")
set(PROJECT_URL "http://www.jeanguyomarch.com/war2edit.php")

set(CMAKE_COLOR_MAKEFILE   ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_MACOSX_RPATH OFF)

find_package(PkgConfig)

# 1.16.99 is required because a bug in hoversel has been fixed then
set(EFL_VERSION 1.16.99)
set(ELM_VERSION 1.16.99)
pkg_check_modules(EINA eina>=${EFL_VERSION} REQUIRED)
pkg_check_modules(EET eet>=${EFL_VERSION} REQUIRED)
pkg_check_modules(EVAS evas>=${EFL_VERSION} REQUIRED)
pkg_check_modules(ECORE ecore>=${EFL_VERSION} REQUIRED)
pkg_check_modules(ECORE_FILE ecore-file>=${EFL_VERSION} REQUIRED)
pkg_check_modules(ELEMENTARY elementary>=${ELM_VERSION} REQUIRED)
pkg_check_modules(CAIRO cairo REQUIRED)
pkg_check_modules(LIBPUD pud REQUIRED)
pkg_check_modules(LIBWAR2 war2 REQUIRED)

add_subdirectory(bin)
add_subdirectory(data)

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake)
