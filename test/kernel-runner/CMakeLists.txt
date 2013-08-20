add_executable(
  kernel-runner
  main.cpp
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

IF (LINK_DIRECTLY_WITH_POCL)

    target_link_libraries(
      kernel-runner
      pocl
    )  

ELSE (LINK_DIRECTLY_WITH_POCL)

  IF (APPLE)
    target_link_libraries(
      kernel-runner
    )  
    set_target_properties(
      kernel-runner PROPERTIES
      LINK_FLAGS "-framework OpenCL"
    )

  ELSE (APPLE)
    target_link_libraries(
      kernel-runner
      OpenCL
    )

  ENDIF (APPLE)

ENDIF (LINK_DIRECTLY_WITH_POCL)

install(
  TARGETS kernel-runner RUNTIME
  DESTINATION bin
)
