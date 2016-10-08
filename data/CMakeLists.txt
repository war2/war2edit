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

install(
   FILES
   ${CMAKE_CURRENT_SOURCE_DIR}/tiles/forest.png
   ${CMAKE_CURRENT_SOURCE_DIR}/tiles/winter.png
   ${CMAKE_CURRENT_SOURCE_DIR}/tiles/wasteland.png
   ${CMAKE_CURRENT_SOURCE_DIR}/tiles/swamp.png
   DESTINATION share/${CMAKE_PROJECT_NAME}/tiles
)

install(
   FILES
   ${CMAKE_CURRENT_SOURCE_DIR}/sprites/buildings/forest.eet
   ${CMAKE_CURRENT_SOURCE_DIR}/sprites/buildings/winter.eet
   ${CMAKE_CURRENT_SOURCE_DIR}/sprites/buildings/wasteland.eet
   ${CMAKE_CURRENT_SOURCE_DIR}/sprites/buildings/swamp.eet
   DESTINATION share/${CMAKE_PROJECT_NAME}/sprites/buildings
)

install(
   FILES
   ${CMAKE_CURRENT_SOURCE_DIR}/sprites/icons/forest.png
   ${CMAKE_CURRENT_SOURCE_DIR}/sprites/icons/winter.png
   ${CMAKE_CURRENT_SOURCE_DIR}/sprites/icons/wasteland.png
   ${CMAKE_CURRENT_SOURCE_DIR}/sprites/icons/swamp.png
   DESTINATION share/${CMAKE_PROJECT_NAME}/sprites/icons
)

install(
   FILES
   ${CMAKE_CURRENT_SOURCE_DIR}/sprites/units/units.eet
   DESTINATION share/${CMAKE_PROJECT_NAME}/sprites/units
)

install(
   FILES
   ${CMAKE_CURRENT_SOURCE_DIR}/sprites/misc/sel1x1.png
   ${CMAKE_CURRENT_SOURCE_DIR}/sprites/misc/sel2x2.png
   ${CMAKE_CURRENT_SOURCE_DIR}/sprites/misc/sel3x3.png
   ${CMAKE_CURRENT_SOURCE_DIR}/sprites/misc/sel4x4.png
   DESTINATION share/${CMAKE_PROJECT_NAME}/sprites/misc
)

install(
   FILES
   ${CMAKE_CURRENT_SOURCE_DIR}/images/efl.png
   ${CMAKE_CURRENT_SOURCE_DIR}/images/brush_big.png
   ${CMAKE_CURRENT_SOURCE_DIR}/images/brush_medium.png
   ${CMAKE_CURRENT_SOURCE_DIR}/images/brush_small.png
   ${CMAKE_CURRENT_SOURCE_DIR}/images/sel_squared.png
   ${CMAKE_CURRENT_SOURCE_DIR}/images/sel_circular.png
   ${CMAKE_CURRENT_SOURCE_DIR}/images/sel_sparkle.png
   ${CMAKE_CURRENT_SOURCE_DIR}/images/dark.png
   ${CMAKE_CURRENT_SOURCE_DIR}/images/efl.png
   ${CMAKE_CURRENT_SOURCE_DIR}/images/grass.png
   ${CMAKE_CURRENT_SOURCE_DIR}/images/ground.png
   ${CMAKE_CURRENT_SOURCE_DIR}/images/human_walls.png
   ${CMAKE_CURRENT_SOURCE_DIR}/images/light.png
   ${CMAKE_CURRENT_SOURCE_DIR}/images/orc_walls.png
   ${CMAKE_CURRENT_SOURCE_DIR}/images/rocks.png
   ${CMAKE_CURRENT_SOURCE_DIR}/images/selection.png
   ${CMAKE_CURRENT_SOURCE_DIR}/images/trees.png
   ${CMAKE_CURRENT_SOURCE_DIR}/images/water.png
   ${CMAKE_CURRENT_SOURCE_DIR}/images/tools.png
   ${CMAKE_CURRENT_SOURCE_DIR}/images/units.png
   ${CMAKE_CURRENT_SOURCE_DIR}/images/players.png

   DESTINATION share/${CMAKE_PROJECT_NAME}/images
)

install(
   FILES
   ${CMAKE_CURRENT_SOURCE_DIR}/images/war2edit.png
   DESTINATION share/icons
)

install(
   FILES
   ${CMAKE_CURRENT_SOURCE_DIR}/desktop/war2edit.desktop
   DESTINATION share/applications
)

add_subdirectory(themes)
