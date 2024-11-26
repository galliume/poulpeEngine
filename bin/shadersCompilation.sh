#!/usr/bin/env bash

ROOT_DIR="$(dirname "$0")"
GLSLC_BIN="glslc"

if [ ! -d "./assets/shaders/spv" ]; then
    mkdir "./assets/shaders/spv"
fi

echo "Compiling shaders"

#@todo read from config/shaders.json
shaders=("main_basic" "main_basic_no_texture" "skybox"
        "shadowMap" "pointLightShadowMap")

for shader in ${shaders[@]}; do
    ${GLSLC_BIN} -O ./assets/shaders/${shader}.vert -o ./assets/shaders/spv/${shader}_vert.spv
    ${GLSLC_BIN} -O ./assets/shaders/${shader}.frag -o ./assets/shaders/spv/${shader}_frag.spv
    echo "Shader [${shader}] compiled"
done

${GLSLC_BIN} -O ./assets/shaders/pointLightShadowMap.geom -o ./assets/shaders/spv/pointLightShadowMap_geom.spv

echo "All shaders have been compiled."
