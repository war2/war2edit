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

   DESTINATION share/${CMAKE_PROJECT_NAME}/images
)

add_subdirectory(prefs)
add_subdirectory(themes)
