add_custom_command(
   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dosbox.epb
   COMMAND elm_prefs_cc ${CMAKE_CURRENT_SOURCE_DIR}/dosbox.epc ${CMAKE_CURRENT_BINARY_DIR}/dosbox.epb
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/dosbox.epc
)
add_custom_target(
   prefs
   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/dosbox.epb
)

list(APPEND CONFIG_FILES
   ${CMAKE_CURRENT_BINARY_DIR}/dosbox.epb
)

install(
   FILES ${CONFIG_FILES}
   DESTINATION share/${CMAKE_PROJECT_NAME}/prefs
)

