add_clang_executable(
  webcl-validator
  main.cpp
  WebCLAction.cpp
  WebCLConsumer.cpp
  WebCLPreprocessor.cpp
  WebCLPrinter.cpp
  WebCLReporter.cpp
  WebCLTransformer.cpp
  WebCLVisitor.cpp
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
