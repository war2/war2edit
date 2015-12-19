add_definitions(-DPACKAGE_COMPILE_BIN_DIR=\"${CMAKE_CURRENT_BINARY_DIR}\")
add_definitions(-DPACKAGE_COMPILE_DATA_DIR=\"${CMAKE_BINARY_DIR}/data\")

set(
   WAR2EDIT_SRC
   main.c
   editor.c
   log.c
   bitmap.c
   texture.c
   sprite.c
   file.c
   mainconfig.c
   menu.c
   toolbar.c
   cell.c
   minimap.c
   sel.c
   tile.c
   inwin.c

   editor.h
   log.h
   bitmap.h
   texture.h
   sprite.h
   file.h
   mainconfig.h
   menu.h
   toolbar.h
   cell.h
   minimap.h
   sel.h
   tile.h
   inwin.h

   war2edit.h
)
add_executable(war2edit ${WAR2EDIT_SRC})

include_directories(
   SYSTEM
   ${EINA_INCLUDE_DIRS}
   ${EET_INCLUDE_DIRS}
   ${EVAS_INCLUDE_DIRS}
   ${ECORE_INCLUDE_DIRS}
   ${ECORE_FILE_INCLUDE_DIRS}
   ${ELEMENTARY_INCLUDE_DIRS}
   ${ELM_BITMAP_INCLUDE_DIRS}
   ${LIBPUD_INCLUDE_DIRS}
   ${LIBWAR2_INCLUDE_DIRS}
)

target_link_libraries(war2edit
   ${EINA_LIBRARIES}
   ${EET_LIBRARIES}
   ${EVAS_LIBRARIES}
   ${ECORE_LIBRARIES}
   ${ECORE_FILE_LIBRARIES}
   ${ELEMENTARY_LIBRARIES}
   ${ELM_BITMAP_LIBRARIES}
   ${LIBPUD_LIBRARIES}
   ${LIBWAR2_LIBRARIES}
)

install(
   TARGETS war2edit
   RUNTIME DESTINATION bin
)

