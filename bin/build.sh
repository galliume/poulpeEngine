#!/usr/bin/env bash
############################################
### Script to generate Poulpe Engine     ###
############################################

showusage() {
    echo "~~~~~~~~~~~~~~ PoulpeEngine ~~~~~~~~~~~~~~~~~"
    echo "-c|--config           CMake config		   "
    echo "-b|--build            Build PoulpeEdit	   "
    echo "-h|--help             Show this help		   "
    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
}

configPoulpeEngine=0
buildPoulpeEngine=0
buildSystem="Ninja"
toolSet=""

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
      -c|--config)
        configPoulpeEngine=1
        shift
        ;;
      -b|--build)
        buildPoulpeEngine=1
        shift
        ;;
      -h|--help)
        showusage
        exit 0
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
          -DCMAKE_C_COMPILER:STRING="clang" \
          -DCMAKE_BUILD_TYPE:STRING="Debug" \
          -DCMAKE_GENERATOR_TOOLSET:STRING="$toolSet" \
		  -DCMAKE_CXX_COMPILER:STRING="clang" \
		  -DCMAKE_C_COMPILER:STRING="clang" \
		  -DUSE_CCACHE:BOOL="ON"
		  
fi

if [[ buildPoulpeEngine -eq 1 ]]
then
    cmake --build ./build/ -j20
fi
