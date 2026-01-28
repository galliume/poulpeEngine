message(NOTICE "Fetching Tcl/TK ...")

#@todo required 9 when tcl/tk 9 available on github ubuntu
find_package(TCL REQUIRED)

message(STATUS "Found Tcl version: ${TCL_VERSION}")
message(STATUS "TCL_INCLUDE_PATH = ${TCL_INCLUDE_PATH}")
message(STATUS "TCL_LIBRARY = ${TCL_LIBRARY}")
message(STATUS "TCL_TCLSH = ${TCL_TCLSH}")
message(STATUS "TK_INCLUDE_PATH = ${TK_INCLUDE_PATH}")
message(STATUS "TK_LIBRARY = ${TK_LIBRARY}")

#todo: add flag to activate or not the tcl/tk editor

add_library(tcl INTERFACE)

target_include_directories(tcl
  SYSTEM INTERFACE ${TCL_INCLUDE_PATH})

target_link_libraries(tcl INTERFACE ${TCL_LIBRARY})

add_library(tk INTERFACE)

target_include_directories(tk
  SYSTEM INTERFACE ${TK_INCLUDE_PATH})

target_link_libraries(tk INTERFACE ${TK_LIBRARY})