message(NOTICE "Fetching Tcl/TK ...")

find_package(TCL 9 REQUIRED)

message(STATUS "TCL_INCLUDE_PATH = ${TCL_INCLUDE_PATH}")
message(STATUS "TCL_LIBRARY = ${TCL_LIBRARY}")
message(STATUS "TCL_TCLSH = ${TCL_TCLSH}")

#todo: add flag to activate or not the tcl/tk editor
target_include_directories(${PROJECT_NAME}
SYSTEM PRIVATE 
  ${TCL_INCLUDE_PATH}
  ${TK_INCLUDE_PATH})

target_link_libraries(${PROJECT_NAME}
PRIVATE
  ${TCL_LIBRARY}
  ${TK_LIBRARY})
