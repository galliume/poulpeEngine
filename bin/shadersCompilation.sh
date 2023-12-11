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

${GLSLC_BIN} ./assets/shaders/main_basic.vert -o ./assets/shaders/spv/main_basic_vert.spv
${GLSLC_BIN} ./assets/shaders/main_basic.frag -o ./assets/shaders/spv/main_basic_frag.spv
echo "Shader [main_basic] compiled"

${GLSLC_BIN} ./assets/shaders/main_basic_no_texture.vert -o ./assets/shaders/spv/main_basic_no_texture_vert.spv
${GLSLC_BIN} ./assets/shaders/main_basic_no_texture.frag -o ./assets/shaders/spv/main_basic_no_texture_frag.spv
echo "Shader [main_basic_no_texture] compiled"

${GLSLC_BIN} ./assets/shaders/skybox.vert -o ./assets/shaders/spv/skybox_vert.spv
${GLSLC_BIN} ./assets/shaders/skybox.frag -o ./assets/shaders/spv/skybox_frag.spv
echo "Shader [skybox] compiled"

${GLSLC_BIN} ./assets/shaders/2d.vert -o assets/shaders/spv/2d_vert.spv
${GLSLC_BIN} ./assets/shaders/2d.frag -o assets/shaders/spv/2d_frag.spv
echo "Shader [2d] compiled"

${GLSLC_BIN} ./assets/shaders/grid.vert -o assets/shaders/spv/grid_vert.spv
${GLSLC_BIN} ./assets/shaders/grid.frag -o assets/shaders/spv/grid_frag.spv
echo "Shader [grid] compiled"

${GLSLC_BIN} ./assets/shaders/ambient_light.vert -o assets/shaders/spv/ambient_light_vert.spv
${GLSLC_BIN} ./assets/shaders/ambient_light.frag -o assets/shaders/spv/ambient_light_frag.spv
echo "Shader [ambient_light] compiled"

${GLSLC_BIN} ./assets/shaders/tangent.vert -o assets/shaders/spv/tangent_vert.spv
${GLSLC_BIN} ./assets/shaders/tangent.frag -o assets/shaders/spv/tangent_frag.spv
echo "Shader [tangent] compiled"

${GLSLC_BIN} ./assets/shaders/depth.vert -o assets/shaders/spv/depth_vert.spv
${GLSLC_BIN} ./assets/shaders/depth.frag -o assets/shaders/spv/depth_frag.spv

echo "Shader [depth] compiled"

echo "All shaders have been compiled."
