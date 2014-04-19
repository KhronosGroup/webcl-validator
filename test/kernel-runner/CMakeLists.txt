SET(LLVM_LINK_COMPONENTS
  Support
)

add_wclv_test(
  kernel-runner
  main.cpp
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${OPENCL_INCLUDE_DIRS}
)

target_link_libraries(
  kernel-runner
  ${OPENCL_LIBRARIES}
)

install(
  TARGETS kernel-runner RUNTIME
  DESTINATION bin
)
