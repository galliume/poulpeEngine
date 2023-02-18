#!/usr/bin/env bash

############################################
### Script to generate Rebulkan Engine   ###
###                                      ###
### - create a build dir                 ###
### - generate cmake with default params ###
### - make                               ###
### - debug & release                    ###
### - launch tests ?                     ###
############################################

#Default Configuration
CMAKE_BUILD_DIR="build"
CMAKE_CXX_COMPILER="clang++"
CMAKE_C_COMPILER="clang"
CMAKE_BUILD_TYPE="Debug"
REFRESH_BUILD_DIR=false
CMAKE_J=8

while getopts ":a:b:c:d:e:f" opt; do
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

if [ -d "./${CMAKE_BUILD_DIR}/${CMAKE_BUILD_TYPE}" ]; then
	echo "Directory ${CMAKE_BUILD_DIR}/${CMAKE_BUILD_TYPE} exists."

	if [ $REFRESH_BUILD_DIR = true ]; then
		rm "./${CMAKE_BUILD_DIR}/${CMAKE_BUILD_TYPE}" -rf
		echo "Directory ${CMAKE_BUILD_DIR}/${CMAKE_BUILD_TYPE} deleted."
		mkdir "./${CMAKE_BUILD_DIR}/${CMAKE_BUILD_TYPE}"
		echo "Directory ${CMAKE_BUILD_DIR}/${CMAKE_BUILD_TYPE} freshly recreated."
	fi
else
	if [ ! -d "./${CMAKE_BUILD_DIR}" ]; then
		mkdir "./${CMAKE_BUILD_DIR}"
	fi
	
	mkdir "./${CMAKE_BUILD_DIR}/${CMAKE_BUILD_TYPE}"
	echo "Directory ${CMAKE_BUILD_DIR}/${CMAKE_BUILD_TYPE} freshly recreated."
fi

echo "Moving to ${CMAKE_BUILD_DIR}/${CMAKE_BUILD_TYPE}"
cd "./${CMAKE_BUILD_DIR}/${CMAKE_BUILD_TYPE}"

echo "Starting configuration with options : "
echo "Build dir: ${CMAKE_BUILD_DIR}/${CMAKE_BUILD_TYPE}"
echo "Building from: ../${CMAKE_BUILD_DIR}"
echo "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}"
echo "CMAKE_C_COMPILER : ${CMAKE_C_COMPILER}"
echo "CMAKE_BUILD_TYPE : ${CMAKE_BUILD_TYPE}"
echo "CMAKE_J : ${CMAKE_J}"

cmake -DCMAKE_CONFIGURATION_TYPES="Debug;Release;RelWithDebInfo" -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} ../..

echo "configuration done"

#Just to be sure to be on the docking branch
echo "Switching to ImGui docking branch"
cd "../../vendor/imgui"
git checkout docking

cd "../../${CMAKE_BUILD_DIR}/${CMAKE_BUILD_TYPE}"

echo "Starting building"
cmake --build . -j${CMAKE_J}

echo "build done"