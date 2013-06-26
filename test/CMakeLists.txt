
# read variable from make command
IF("$ENV{USE_POCL}" STREQUAL "1")
  SET( LINK_DIRECTLY_WITH_POCL 1 )
  message("Using Pocl for running test suite.")
ENDIF()

if (LINK_DIRECTLY_WITH_POCL)

  find_path(POCL_INCLUDE_DIR pocl/pocl.h
    HINTS $ENV{POCL_INSTALL_DIR}/include )

  find_library(POCL_LIBRARY_FULL_PATH 
    NAMES pocl libpocl
    HINTS $ENV{POCL_INSTALL_DIR}/lib )
  
  get_filename_component(POCL_LIBRARY_DIR ${POCL_LIBRARY_FULL_PATH} PATH) 

  set( POCL_INCLUDE_DIRS ${POCL_INCLUDE_DIR} )
  set( POCL_LIBRARIES pocl )
  set( POCL_LIBRARY_DIRS ${POCL_LIBRARY_DIR} )

  link_directories( ${POCL_LIBRARY_DIRS} )

  IF(NOT POCL_LIBRARY_FULL_PATH)
    message(FATAL_ERROR "Could not find POCL library, please set your POCL_INSTALL_DIR enviroment variable.")
  ENDIF()

endif (LINK_DIRECTLY_WITH_POCL)

# note that add subdirectory must be after variable declarations 
# to make sure that options are passed to subdirectories
add_subdirectory( opencl-validator )
add_subdirectory( radix-sort )

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
  DEPENDS webcl-validator opencl-validator radix-sort FileCheck
)
set_target_properties(
  check-webcl-validator
  PROPERTIES FOLDER "WebCL Validator tests"
)
