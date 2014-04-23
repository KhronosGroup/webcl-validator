add_wclv_test(
  opencl-validator
  main.cpp
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${OPENCL_INCLUDE_DIRS}
)

target_link_libraries(
  opencl-validator
  ${OPENCL_LIBRARIES}
)

install(
  TARGETS opencl-validator RUNTIME
  DESTINATION bin
)
