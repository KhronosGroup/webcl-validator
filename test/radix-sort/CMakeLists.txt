add_executable(
  radix-sort
  main.cpp
  sorter.cpp
  verifier.cpp
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

IF (APPLE)

  target_link_libraries(
    radix-sort
  )
  set_target_properties(
    radix-sort PROPERTIES
    LINK_FLAGS "-framework OpenCL"
  )

ELSE (APPLE)

  target_link_libraries(
    radix-sort
    OpenCL
  )

ENDIF (APPLE)

install(
  TARGETS radix-sort RUNTIME
  DESTINATION bin
)
