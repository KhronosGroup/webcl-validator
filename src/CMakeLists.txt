add_custom_command(
  OUTPUT general.h
  DEPENDS general.cl
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND xxd -i general.cl general.h
)

add_custom_command(
  OUTPUT kernel.h
  DEPENDS kernel.cl
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND xxd -i kernel.cl kernel.h
)

add_clang_executable(
  webcl-validator
  general.h
  kernel.h
  main.cpp
  WebCLAction.cpp
  WebCLArguments.cpp
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
