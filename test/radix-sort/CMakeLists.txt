SET(LLVM_LINK_COMPONENTS
  Support
  )

add_wclv_test(
  radix-sort
  main.cpp
  sorter.cpp
  verifier.cpp
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

IF (LINK_DIRECTLY_WITH_POCL)

    target_link_libraries(
      radix-sort
      pocl
    )  

ELSE (LINK_DIRECTLY_WITH_POCL)

  IF (APPLE)

    target_link_libraries(
      radix-sort
    )  
    set_target_properties(
      radix-sort PROPERTIES
      LINK_FLAGS "-framework OpenCL"
    )

  ELSE (APPLE)

    target_link_libraries(
      radix-sort
      OpenCL
    )

  ENDIF (APPLE)

ENDIF (LINK_DIRECTLY_WITH_POCL)

install(
  TARGETS radix-sort RUNTIME
  DESTINATION bin
)
