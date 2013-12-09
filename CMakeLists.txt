set(WCLV_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(WCLV_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

install(DIRECTORY include/
  DESTINATION include
  FILES_MATCHING
  PATTERN "*.h"
  PATTERN "config.h" EXCLUDE
  )

include_directories(BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  )

add_subdirectory( lib )
add_subdirectory( driver )
add_subdirectory( test )
