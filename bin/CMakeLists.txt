# Copyright (c) 2014-2016 Jean Guyomarc'h
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

#add_definitions(-DPACKAGE_BIN_DIR=\"${CMAKE_INSTALL_PREFIX}/bin\")
#add_definitions(-DPACKAGE_DATA_DIR=\"${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME}\")
#add_definitions(-DPACKAGE_LIB_DIR=\"${CMAKE_INSTALL_PREFIX}/lib\")
#add_definitions(-DBUILD_DATA_DIR=\"${CMAKE_BINARY_DIR}/data\")

add_executable(war2edit
   main.c
   editor.c
   editor.h
   log.c
   log.h
   bitmap.c
   bitmap.h
   texture.c
   texture.h
   sprite.c
   sprite.h
   file.c
   file.h
   mainconfig.c
   mainconfig.h
   menu.c
   menu.h
   toolbar.c
   toolbar.h
   cell.c
   cell.h
   minimap.c
   minimap.h
   sel.c
   sel.h
   tile.c
   tile.h
   inwin.c
   inwin.h
   prefs.c
   prefs.h
   ipc.c
   ipc.h
   undo.c
   undo.h
   str.h
   war2edit.h
)

target_include_directories(war2edit
   SYSTEM
   PUBLIC ${EINA_INCLUDE_DIRS}
   PUBLIC ${EET_INCLUDE_DIRS}
   PUBLIC ${EVAS_INCLUDE_DIRS}
   PUBLIC ${ECORE_INCLUDE_DIRS}
   PUBLIC ${ECORE_FILE_INCLUDE_DIRS}
   PUBLIC ${ELEMENTARY_INCLUDE_DIRS}
   PUBLIC ${LIBPUD_INCLUDE_DIRS}
   PUBLIC ${LIBWAR2_INCLUDE_DIRS}
   PUBLIC ${CAIRO_INCLUDE_DIRS}
)

target_link_libraries(war2edit
   ${EINA_LIBRARIES}
   ${EET_LIBRARIES}
   ${EVAS_LIBRARIES}
   ${ECORE_LIBRARIES}
   ${ECORE_FILE_LIBRARIES}
   ${ELEMENTARY_LIBRARIES}
   ${LIBPUD_LIBRARIES}
   ${LIBWAR2_LIBRARIES}
   ${CAIRO_LIBRARIES}
)
add_dependencies(
   war2edit
   prefs
   themes
)

set_property(TARGET war2edit PROPERTY C_STANDARD 99)

target_compile_definitions(war2edit
   PRIVATE PACKAGE_BIN_DIR=\"${CMAKE_INSTALL_PREFIX}/bin\"
   PRIVATE PACKAGE_DATA_DIR=\"${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME}\"
   PRIVATE PACKAGE_LIB_DIR=\"${CMAKE_INSTALL_PREFIX}/lib\"
   PRIVATE BUILD_DATA_DIR=\"${CMAKE_BINARY_DIR}/data\"
   PRIVATE EO_BETA_API=1
   PRIVATE EFL_BETA_API_SUPPORT=1
   PRIVATE EFL_EO_API_SUPPORT=1
   PRIVATE _DEFAULT_SOURCE=1
)


install(
   TARGETS war2edit
   RUNTIME DESTINATION bin
)
