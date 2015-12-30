install(
   FILES
   ${CMAKE_CURRENT_SOURCE_DIR}/tiles/forest.eet
   ${CMAKE_CURRENT_SOURCE_DIR}/tiles/winter.eet
   ${CMAKE_CURRENT_SOURCE_DIR}/tiles/wasteland.eet
   ${CMAKE_CURRENT_SOURCE_DIR}/tiles/swamp.eet
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
   ${CMAKE_CURRENT_SOURCE_DIR}/sprites/misc/sel.eet
   DESTINATION share/${CMAKE_PROJECT_NAME}/sprites/misc
)

install(
   FILES
   ${CMAKE_CURRENT_SOURCE_DIR}/images/efl.png
   DESTINATION share/${CMAKE_PROJECT_NAME}/images
)

add_subdirectory(prefs)

