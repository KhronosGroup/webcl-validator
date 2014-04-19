SET(LLVM_LINK_COMPONENTS
  Support
)

add_wclv_test(
  radix-sort
  main.cpp
  sorter.cpp
  verifier.cpp
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${OPENCL_INCLUDE_DIRS}
)

target_link_libraries(
  radix-sort
  ${OPENCL_LIBRARIES}
)

install(
  TARGETS radix-sort RUNTIME
  DESTINATION bin
)
