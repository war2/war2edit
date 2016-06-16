add_custom_command(
   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/default.edj
   COMMAND edje_cc ${CMAKE_CURRENT_SOURCE_DIR}/default.edc -id ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/default.edj
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/default.edc
)
add_custom_target(
   themes
   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/default.edj
)

list(APPEND THEME_FILES
   ${CMAKE_CURRENT_BINARY_DIR}/default.edj
)

install(
   FILES ${THEME_FILES}
   DESTINATION share/${CMAKE_PROJECT_NAME}/themes
)
