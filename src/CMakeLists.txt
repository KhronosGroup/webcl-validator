add_custom_command(
  OUTPUT general.h
  DEPENDS general.cl
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND xxd -i general.cl general.h
)

add_clang_executable(
  webcl-validator
  general.h
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
