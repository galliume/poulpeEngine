#!/usr/bin/env bash

detectOs() {
	case "$(uname -sr)" in

	   Linux*Microsoft*)
		 #echo 'OS detected : WSL'  # Windows Subsystem for Linux
		 return 1
		 ;;

	   Linux*)
		 #echo 'OS detected : Linux'
		 return 2
		 ;;

	   CYGWIN*|MINGW*|MINGW32*|MSYS*)
		 #echo 'OS detected : MS Windows'
		 return 3
		 ;;

	esac
}

showHelp() {
cat << EOF  

Usage: ./bin/plpBuild.sh [-abcdefgh]

-a: Build directory [default to CMAKE_BUILD_DIR="build"]
-b: CXX compiler [default to CMAKE_CXX_COMPILER="clang++"]
-c: C compiler [default to CMAKE_C_COMPILER="clang"]
-d: Cmake build type [default to CMAKE_BUILD_TYPE="Debug"]
-e: Clean build dir [default to REFRESH_BUILD_DIR=true]
-f: Number of parallels jobs [default to CMAKE_J=8]
-g: CMake toolset (-T) [default to ClangCL]
-h: Help	
-i: Build
-j: Run PoulpeEngine
-k: Build and Run
-l: Use Ninja
-m: Use ccache [default to USE_CCACHE=true]

EOF
	
	exit 0
}