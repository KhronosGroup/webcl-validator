add_executable(
  opencl-validator
  main.cpp
)

IF (APPLE)

  target_link_libraries(
    opencl-validator
  )
  set_target_properties(
    opencl-validator PROPERTIES
    LINK_FLAGS "-framework OpenCL"
  )

ELSE (APPLE)

  target_link_libraries(
    opencl-validator
    OpenCL
  )

ENDIF (APPLE)

install(
  TARGETS opencl-validator RUNTIME
  DESTINATION bin
)
