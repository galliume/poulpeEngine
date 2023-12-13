#!/usr/bin/env bash

ROOT_DIR="$(dirname "$0")"
OS=2 #Linux by default
GLSLC_BIN="glslc"

. "${ROOT_DIR}/utils.sh"

detectOs
OS=$?

if [ ! -d "./assets/shaders/spv" ]; then
	mkdir "./assets/shaders/spv"
fi

if ! [[ "$OS" == 2 ]]; then
GLSLC_BIN="glslc.exe"
fi

echo "Compiling shaders"

#@todo read from config/shaders.json
shaders=("main_basic" "main_basic_no_texture" "skybox"
		"2d" "grid" "ambient_light" "tangent"
		"shadowMap")

for shader in ${shaders[@]}; do
	${GLSLC_BIN} ./assets/shaders/${shader}.vert -o ./assets/shaders/spv/${shader}_vert.spv
	${GLSLC_BIN} ./assets/shaders/${shader}.frag -o ./assets/shaders/spv/${shader}_frag.spv
	echo "Shader [${shader}] compiled"
done

echo "All shaders have been compiled."
