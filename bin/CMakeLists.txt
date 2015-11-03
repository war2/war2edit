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
)
add_executable(war2edit ${WAR2EDIT_SRC})

target_include_directories(
   war2edit
   PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
   PUBLIC ${EINA_INCLUDE_DIRS}
   PUBLIC ${EET_INCLUDE_DIRS}
   PUBLIC ${EVAS_INCLUDE_DIRS}
   PUBLIC ${ECORE_INCLUDE_DIRS}
   PUBLIC ${ECORE_FILE_INCLUDE_DIRS}
   PUBLIC ${ELEMENTARY_INCLUDE_DIRS}
   PUBLIC ${ELM_BITMAP_INCLUDE_DIRS}
   PUBLIC ${LIBPUD_INCLUDE_DIRS}
   PUBLIC ${LIBWAR2_INCLUDE_DIRS}
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

