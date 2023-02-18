#!/usr/bin/env bash

ROOT_DIR="$(dirname "$0")"
OS=2 #Linux by default
GLSLC_BIN="glslc"

. "${ROOT_DIR}/utils.sh"

detectOs
OS=$?

if ! [[ "$OS" == 2 ]]; then
GLSLC_BIN="glslc.exe"
fi

echo "Compiling shaders"

${GLSLC_BIN} ./assets/shaders/shader.vert -o ./assets/shaders/spv/vert.spv
${GLSLC_BIN} ./assets/shaders/shader.frag -o ./assets/shaders/spv/frag.spv
echo "Shader [shader] compiled"

${GLSLC_BIN} ./assets/shaders/skybox.vert -o ./assets/shaders/spv/skybox_vert.spv
${GLSLC_BIN} ./assets/shaders/skybox.frag -o ./assets/shaders/spv/skybox_frag.spv
echo "Shader [skybox] compiled"

${GLSLC_BIN} ./assets/shaders/2d.vert -o assets/shaders/spv/2d_vert.spv
${GLSLC_BIN} ./assets/shaders/2d.frag -o assets/shaders/spv/2d_frag.spv
echo "Shader [2d] compiled"

${GLSLC_BIN} ./assets/shaders/ambient_shader.vert -o assets/shaders/spv/ambient_shader_vert.spv
${GLSLC_BIN} ./assets/shaders/ambient_shader.frag -o assets/shaders/spv/ambient_shader_frag.spv
echo "Shader [ambient_shader] compiled"

${GLSLC_BIN} ./assets/shaders/grid.vert -o assets/shaders/spv/grid_vert.spv
${GLSLC_BIN} ./assets/shaders/grid.frag -o assets/shaders/spv/grid_frag.spv
echo "Shader [grid] compiled"

${GLSLC_BIN} ./assets/shaders/bbox.vert -o assets/shaders/spv/bbox_vert.spv
${GLSLC_BIN} ./assets/shaders/bbox.frag -o assets/shaders/spv/bbox_frag.spv
echo "Shader [shaders] compiled"

echo "All shaders compiled"
