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
configuration="windows-x64-debug"

ROOT_DIR="$(dirname "$0")"
. "${ROOT_DIR}/utils.sh"

detectOs
OS=$?

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
    cmake --preset "$configuration"
fi

if [[ buildPoulpeEngine -eq 1 ]]
then
    cmake --build --preset "$configuration" -j20
fi
