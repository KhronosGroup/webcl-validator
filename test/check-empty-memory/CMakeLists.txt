add_wclv_test(
  check-empty-memory
  main.cpp
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

IF (LINK_DIRECTLY_WITH_POCL)

    target_link_libraries(
      check-empty-memory
      pocl
    )

ELSE (LINK_DIRECTLY_WITH_POCL)

  IF (APPLE)

    target_link_libraries(
      check-empty-memory
    )
    set_target_properties(
      check-empty-memory PROPERTIES
      LINK_FLAGS "-framework OpenCL"
    )

  ELSE (APPLE)

    target_link_libraries(
      check-empty-memory
      OpenCL
    )

  ENDIF (APPLE)

ENDIF (LINK_DIRECTLY_WITH_POCL)

install(
  TARGETS check-empty-memory RUNTIME
  DESTINATION bin
)
