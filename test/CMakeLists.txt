add_subdirectory( opencl-validator )

set(
  WEBCL_VALIDATOR_TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
)
configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
)
add_lit_testsuite(
  check-webcl-validator  "Running WebCL Validator regression tests"
  ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS webcl-validator opencl-validator FileCheck
)
set_target_properties(
  check-webcl-validator
  PROPERTIES FOLDER "WebCL Validator tests"
)
