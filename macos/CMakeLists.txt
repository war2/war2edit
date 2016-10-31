if (APPLE)
   configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/config.py.in
      ${CMAKE_CURRENT_BINARY_DIR}/config.py
   )

   add_custom_target(osx-package
      osx-packager -c ${CMAKE_CURRENT_BINARY_DIR}/config.py
      DEPENDS war2edit
      COMMENT "Generating APP package"
   )
endif ()
