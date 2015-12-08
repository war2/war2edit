macro (check_compiler_warnings warnings)

   foreach (w ${ARGV})

      string(REPLACE "-W" "W" WARN ${w})

      CHECK_C_COMPILER_FLAG(${w} C_COMPILER_${WARN})
      if (C_COMPILER_${WARN})
         LIST(APPEND COMPILER_WARNINGS ${w})
      endif ()

   endforeach ()

endmacro ()

