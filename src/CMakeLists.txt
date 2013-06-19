add_clang_executable(
  webcl-validator
  main.cpp
  WebCLAction.cpp
  WebCLConsumer.cpp
  WebCLHelper.cpp
  WebCLPreprocessor.cpp
  WebCLPrinter.cpp
  WebCLReporter.cpp
  WebCLTransformation.cpp
  WebCLTransformations.cpp
  WebCLTransformer.cpp
  WebCLTransformerConfiguration.cpp
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
