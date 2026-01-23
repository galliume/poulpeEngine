function(set_poulpe_links target)

  if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND EXTRA_LIBS
      Ws2_32 user32 gdi32 winspool shell32 ole32 oleaut32 uuid comdlg32 advapi32)
  endif()

  set(LINK_OPTIONS
    -pthread)

  target_link_options(${target} PRIVATE
    $<$<CONFIG:DEBUG>:${LINK_OPTIONS}>
    $<$<CONFIG:RELEASE>:${LINK_OPTIONS}>)

  message(STATUS "EXTRA_LIBS: ${EXTRA_LIBS}")
  target_link_libraries(${target} PRIVATE ${EXTRA_LIBS})

endfunction()