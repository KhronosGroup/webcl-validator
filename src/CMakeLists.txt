add_custom_command(
  OUTPUT general.h
  DEPENDS general.cl
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND ${PYTHON_EXECUTABLE} stripcr.py general.cl general-endlfix.cl
  COMMAND xxd -i general-endlfix.cl general.h
)

add_custom_command(
  OUTPUT kernel.h
  DEPENDS kernel.cl
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND ${PYTHON_EXECUTABLE} stripcr.py kernel.cl kernel-endlfix.cl
  COMMAND xxd -i kernel-endlfix.cl kernel.h
)

add_clang_executable(
  webcl-validator
  general.h
  kernel.h
  main.cpp
  WebCLAction.cpp
  WebCLArguments.cpp
  WebCLBuiltins.cpp
  WebCLConfiguration.cpp
  WebCLConsumer.cpp
  WebCLHelper.cpp
  WebCLMatcher.cpp
  WebCLPass.cpp
  WebCLPreprocessor.cpp
  WebCLPrinter.cpp
  WebCLRenamer.cpp
  WebCLReporter.cpp
  WebCLRewriter.cpp
  WebCLTool.cpp
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
