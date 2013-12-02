set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
)

add_clang_executable(webcl-validator
  main.cpp
  WebCLHeader.cpp
)

target_link_libraries(webcl-validator
  libwclv
)

install(
  TARGETS webcl-validator RUNTIME
  DESTINATION bin
)
