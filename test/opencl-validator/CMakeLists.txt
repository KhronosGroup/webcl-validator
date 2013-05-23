add_executable(
  opencl-validator
  main.cpp
)

target_link_libraries(
  opencl-validator
  OpenCL
)

install(
  TARGETS opencl-validator RUNTIME
  DESTINATION bin
)
