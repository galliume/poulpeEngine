#!/usr/bin/env bash

############################################
### Script to generate Rebulkan Engine   ###
############################################

ROOT_DIR="$(dirname "$0")"
. "${ROOT_DIR}/cmakeCmd.sh"
. "${ROOT_DIR}/utils.sh"

#Default Configuration
CMAKE_BUILD_DIR="build"
CMAKE_CXX_COMPILER="clang++"
CMAKE_C_COMPILER="clang"
CMAKE_BUILD_TYPE="Debug"
REFRESH_BUILD_DIR=false
CMAKE_J=8
CMAKE_TOOLSET="ClangCL"
USE_NINJA=true
USE_CCACHE=true

while getopts ":a:b:c:d:e:f:g:hijkl:m:n" opt; do
  case $opt in
    a) CMAKE_BUILD_DIR="$OPTARG"
    ;;
    b) CMAKE_CXX_COMPILER="$OPTARG"
	;;
	c) CMAKE_C_COMPILER="$OPTARG"
    ;;
	d) CMAKE_BUILD_TYPE="$OPTARG"
	;;
	e) REFRESH_BUILD_DIR="$OPTARG"
	;;
	f) CMAKE_J="$OPTARG"
	;;
	h) showHelp
	;;
	g) CMAKE_TOOLSET="$OPTARG"
	;;
	i) build; exit 0
	;;
	j) run; exit 0
	;;
	k) build; run; exit 0
	;;
	l) USE_NINJA="$OPTARG"
	;;
	m) USE_CCACHE="$OPTARG"
	;;
	n) runCTest; exit 0
	;;
    \?) echo "Invalid option -$OPTARG" >&2
    exit 1
    ;;
  esac

  case $OPTARG in
    -*) echo "Option $opt needs a valid argument"
    exit 1
    ;;
  esac
done

configure