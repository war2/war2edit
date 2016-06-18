#add_definitions(-DPACKAGE_BIN_DIR=\"${CMAKE_INSTALL_PREFIX}/bin\")
#add_definitions(-DPACKAGE_DATA_DIR=\"${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME}\")
#add_definitions(-DPACKAGE_LIB_DIR=\"${CMAKE_INSTALL_PREFIX}/lib\")
#add_definitions(-DBUILD_DATA_DIR=\"${CMAKE_BINARY_DIR}/data\")

add_executable(war2edit
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
   prefs.c
   ipc.c
   undo.c
   str.h
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
   prefs.h
   ipc.h
   undo.h
   war2edit.h
)

include_directories(
   SYSTEM
   ${EINA_INCLUDE_DIRS}
   ${EET_INCLUDE_DIRS}
   ${EVAS_INCLUDE_DIRS}
   ${ECORE_INCLUDE_DIRS}
   ${ECORE_FILE_INCLUDE_DIRS}
   ${ELEMENTARY_INCLUDE_DIRS}
   ${LIBPUD_INCLUDE_DIRS}
   ${LIBWAR2_INCLUDE_DIRS}
#  ${CAIRO_INCLUDE_DIRS}
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
#  ${CAIRO_LIBRARIES}
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
