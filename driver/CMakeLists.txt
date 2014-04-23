set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
)

add_clang_executable(webcl-validator
  main.cpp
  WebCLHeader.cpp
)

include_directories(
  ${OPENCL_INCLUDE_DIRS}
)

target_link_libraries(webcl-validator
  clv_standalone
)

install(
  TARGETS webcl-validator RUNTIME
  DESTINATION bin
)
