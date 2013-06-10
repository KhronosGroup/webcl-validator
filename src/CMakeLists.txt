add_clang_executable(
  webcl-validator
  action.cpp
  consumer.cpp
  main.cpp
  preprocessor.cpp
  printer.cpp
  reporter.cpp
  transformer.cpp
  visitor.cpp
)

target_link_libraries(
  webcl-validator
  clangTooling
  clangRewriteFrontend
)

install(
  TARGETS webcl-validator RUNTIME
  DESTINATION bin
)
