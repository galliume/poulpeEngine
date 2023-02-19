#!/usr/bin/env bash

detectOs() {
	case "$(uname -sr)" in

	   Linux*Microsoft*)
		 echo 'OS detected : WSL'  # Windows Subsystem for Linux
		 return 1
		 ;;

	   Linux*)
		 echo 'OS detected : Linux'
		 return 2
		 ;;

	   CYGWIN*|MINGW*|MINGW32*|MSYS*)
		 echo 'OS detected : MS Windows'
		 return 3
		 ;;

	esac
}

showHelp() {
cat << EOF  

Usage: ./bin/rbkBuild.sh [-abcdefgh]

-a: Build directory [default to CMAKE_BUILD_DIR="build"]

-b: CXX compiler [default to CMAKE_CXX_COMPILER="clang++"]

-c: C compiler [default to CMAKE_C_COMPILER="clang"]

-d: Cmake build type [default to CMAKE_BUILD_TYPE="Debug"]

-e: Clean build dir [default to REFRESH_BUILD_DIR=false]

-f: paralls job number [default to CMAKE_J=8]

-g: Compile now with cmake --build [default to BUILD_IT=false]

-h Display help	

EOF
	
	exit 0
}