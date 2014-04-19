function(add_wclv_test test_name)
  add_llvm_executable(${test_name} ${ARGN})
  set_target_properties(${test_name} PROPERTIES FOLDER "WebCL Validator tests")
endfunction()

# note that add subdirectory must be after variable declarations 
# to make sure that options are passed to subdirectories
add_subdirectory( kernel-runner )
add_subdirectory( opencl-validator )
add_subdirectory( radix-sort )
add_subdirectory( check-empty-memory )

set(
  WEBCL_VALIDATOR_TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
)

set(WCLV_TEST_PARAMS
  wclv_site_config=${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
)

configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
)
add_lit_testsuite(
  check-webcl-validator  "Running WebCL Validator regression tests"
  ${CMAKE_CURRENT_BINARY_DIR}
  PARAMS ${WCLV_TEST_PARAMS}
  DEPENDS webcl-validator kernel-runner opencl-validator radix-sort check-empty-memory FileCheck
)
set_target_properties(
  check-webcl-validator
  PROPERTIES FOLDER "WebCL Validator tests"
)
