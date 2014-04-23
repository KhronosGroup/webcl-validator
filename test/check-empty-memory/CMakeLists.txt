add_wclv_test(
  check-empty-memory
  main.cpp
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${OPENCL_INCLUDE_DIRS}
)

target_link_libraries(
  check-empty-memory
  ${OPENCL_LIBRARIES}
)

install(
  TARGETS check-empty-memory RUNTIME
  DESTINATION bin
)
