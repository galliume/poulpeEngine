function(set_poulpe_compiler_flags target)

  message(STATUS "Setting compiler flags for ${target}.")

  target_compile_features(${target}
    PRIVATE   cxx_std_26
    INTERFACE cxx_std_26)

  set_target_properties(${target} PROPERTIES
    CXX_SCAN_FOR_MODULES ON
    CMAKE_CXX_MODULE_STD ON)

  set(DEBUG_OPTIONS
    -g
    -Og
    -Werror
    -fno-inline
    -fno-inline-functions
    -fno-omit-frame-pointer)

  set(RELEASE_OPTIONS
    -O3
    -Werror
    #-ffast-math
    -march=native
    -fvisibility=hidden
    -DNDEBUG)

  set(RELWITHDEBINFO_OPTIONS
    -g
    -O3
    -Werror
    #-ffast-math
    -march=native
    -fvisibility=hidden
    -fno-omit-frame-pointer
    -DNDEBUG)

  target_compile_options(${target} PRIVATE
    $<$<CONFIG:DEBUG>:${DEBUG_OPTIONS}>
    $<$<CONFIG:RELEASE>:${RELEASE_OPTIONS}>
    $<$<CONFIG:RELWITHDEBINFO>:${RELWITHDEBINFO_OPTIONS}>
    -Weverything
    -fno-strict-aliasing
    -Wno-c++98-compat
    -Wno-c++98-compat-pedantic
    -Wno-c++20-compat
    -Wno-old-style-cast
    -Wno-padded
    -Wno-documentation
    -Wno-documentation-unknown-command
    -Wno-decls-in-multiple-modules
    -Wno-unsafe-buffer-usage
    -Wno-covered-switch-default
    -Wno-deprecated-declarations)

endfunction()