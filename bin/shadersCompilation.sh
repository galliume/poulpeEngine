#!/usr/bin/env bash

ROOT_DIR="$(dirname "$0")"
GLSLC_BIN="glslc"

if [ ! -d "./assets/shaders/spv" ]; then
    mkdir "./assets/shaders/spv"
fi

echo "Compiling shaders"

#@todo read from config/shaders.json
shaders=("main_basic" "main_basic_no_texture" "skybox"
        "2d" "grid" "ambient_light" "tangent"
        "shadowMap" "shadowMapSpot")

for shader in ${shaders[@]}; do
    ${GLSLC_BIN} ./assets/shaders/${shader}.vert -o ./assets/shaders/spv/${shader}_vert.spv
    ${GLSLC_BIN} ./assets/shaders/${shader}.frag -o ./assets/shaders/spv/${shader}_frag.spv
    echo "Shader [${shader}] compiled"
done

echo "All shaders have been compiled."
