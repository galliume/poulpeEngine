#!/bin/bash

############################################
### Script to generate Rebulkan Engine   ###
###                                      ###
### - create a build dir                 ###
### - generate cmake with default params ###
### - make                               ###
### - debug & release                    ###
### - launch tests ?                     ###
############################################

CMAKE_BUILD_DIR="build"
CMAKE_CXX_COMPILER=${1:-"clang++"}
CMAKE_C_COMPILER=${2:-"clang"}
CMAKE_BUILD_TYPE=Debug 
REFRESH_BUILD_DIR=${3:-false}
MAKE_J=8

 [ -d "./${CMAKE_BUILD_DIR}" ] && 
    echo "Directory ${CMAKE_BUILD_DIR} exists."

    if [ $REFRESH_BUILD_DIR = true ]
    then
        rm "./${CMAKE_BUILD_DIR}" -rf
        echo "Directory ${CMAKE_BUILD_DIR} deleted."
        mkdir "./${CMAKE_BUILD_DIR}"
        echo "Directory ${CMAKE_BUILD_DIR} freshly recreated."
    fi

echo "Moving to ${CMAKE_BUILD_DIR}"
cd "./${CMAKE_BUILD_DIR}"

echo "Starting configuration with options : "
echo "Build dir: ${CMAKE_BUILD_DIR}"
echo "Building from: ../${CMAKE_BUILD_DIR}"
echo "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}"
echo "CMAKE_C_COMPILER : ${CMAKE_C_COMPILER}"

cmake "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}" "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}" "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}" ..

echo "configuration done"

echo "Switching to ImGui docking branch"
cd "../vendor/imgui"
git checkout docking

cd "../../build"

echo "Starting building"
cmake --build . -j${MAKE_J}

echo "build done"