# Copyright (c) 2014-2017 Jean Guyomarc'h
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

cmake_minimum_required(VERSION 3.0.2)
project(war2edit
   LANGUAGES C
   VERSION 0.0.99)

include(CheckCCompilerFlag)
include(cmake/check_compiler_warnings.cmake)

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
    -Wredundant-decls
    -Winline
    -Winvalid-pch
    -Wcast-align
)

set(PROJECT_DESCRIPTION "Warcraft II Map Editor")
set(PROJECT_URL "https://war2.github.io/war2edit/")

find_package(PkgConfig)

set(EFL_VERSION 1.18.0)
pkg_check_modules(EINA eina>=${EFL_VERSION} REQUIRED)
pkg_check_modules(EET eet>=${EFL_VERSION} REQUIRED)
pkg_check_modules(EVAS evas>=${EFL_VERSION} REQUIRED)
pkg_check_modules(ECORE ecore>=${EFL_VERSION} REQUIRED)
pkg_check_modules(ECORE_FILE ecore-file>=${EFL_VERSION} REQUIRED)
pkg_check_modules(ELEMENTARY elementary>=${EFL_VERSION} REQUIRED)
pkg_check_modules(CAIRO cairo REQUIRED)
pkg_check_modules(LIBPUD pud REQUIRED)
pkg_check_modules(LIBWAR2 war2 REQUIRED)
pkg_check_modules(LZMA liblzma REQUIRED)

add_subdirectory(bin)
add_subdirectory(modules)
add_subdirectory(data)
add_subdirectory(macos)

add_custom_target(uninstall
   COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake
)

install(
   FILES ${CMAKE_CURRENT_SOURCE_DIR}/AUTHORS
   DESTINATION share/${CMAKE_PROJECT_NAME}
)
