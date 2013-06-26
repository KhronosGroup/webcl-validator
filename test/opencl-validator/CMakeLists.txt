add_executable(
  opencl-validator
  main.cpp
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

IF (LINK_DIRECTLY_WITH_POCL)

    target_link_libraries(
      opencl-validator
      pocl
    )  

ELSE (LINK_DIRECTLY_WITH_POCL)

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

ENDIF (LINK_DIRECTLY_WITH_POCL)

install(
  TARGETS opencl-validator RUNTIME
  DESTINATION bin
)
