add_clang_executable(
  webcl-validator
  action.cpp
  consumer.cpp
  main.cpp
  preprocessor.cpp
  reporter.cpp
  transformer.cpp
  visitor.cpp
)

target_link_libraries(
  webcl-validator
  clangTooling
  clangRewriteFrontend
)

include_directories(
  ${CLANG_SOURCE_DIR} # Internal headers such as TreeTransform.h.
)

install(
  TARGETS webcl-validator RUNTIME
  DESTINATION bin
)
