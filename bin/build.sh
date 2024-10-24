#!/usr/bin/env bash
############################################
### Script to generate Poulpe Engine     ###
############################################

showusage() {
cat << EOF 
~~~~~~~~~~~~~~ PoulpeEngine ~~~~~~~~~~~~~~~~~
-c|--config           CMake config		   
-b|--build            Build PoulpeEdit	   
-h|--help             Show this help		   
-d|--debug            Debug build (default)  
-r|--release          Release build 		   

Examples:

Configure and start a release build:
 > bin/build -c -r -b

Configure and start a debug build:
 > bin/build -c -b

* use git bash for Windows
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
EOF
}

configPoulpeEngine=0
buildPoulpeEngine=0
buildSystem="Ninja"
toolSet=""
configuration="Debug"

ROOT_DIR="$(dirname "$0")"
. "${ROOT_DIR}/utils.sh"

detectOs
OS=$?

if [[ "$OS" -ne 2 ]]; then
	buildSystem="Visual Studio 17 2022"
	toolSet="ClangCL"
fi
	
while [[ $# -gt 0 ]]; do
    argument="$1"
    case "$argument" in
	  -b|--build)
        buildPoulpeEngine=1
        shift
        ;;
      -c|--config)
        configPoulpeEngine=1
        shift
        ;;
      -d|--debug)
        configuration="Debug"
        shift
        ;;		
      -h|--help)
        showusage
        exit 0
        ;;		
      -r|--release)
        configuration="Release"
        shift
        ;;				
      *|-*|--*)
        echo "Unknown option $argument"

        showusage
        exit 1
        ;;
    esac
done

if [[ configPoulpeEngine -eq 1 ]]
then
    cmake -B ./build \
		  -G "$buildSystem" \
		  -DCMAKE_GENERATOR_TOOLSET:STRING="$toolSet" \
		  -DUSE_CCACHE:BOOL="ON" \
		  -DASSIMP_BUILD_ASSIMP_TOOLS:BOOL="ON" \
		  -DASSIMP_INSTALL:BOOL="OFF" \
		  -DCMAKE_BUILD_TYPE:STRING="$configuration" \
		  -DCMAKE_C_COMPILER:STRING="clang" \
		  -DCMAKE_CXX_COMPILER:STRING="clang"
fi

if [[ buildPoulpeEngine -eq 1 ]]
then
    cmake --build ./build/ --config $configuration -j20
fi
