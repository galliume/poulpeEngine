#!/bin/bash
./scripts/glslc ./assets/shaders/shader.vert -o ./assets/shaders/spv/vert.spv
./scripts/glslc ./assets/shaders/shader.frag -o ./assets/shaders/spv/frag.spv

./scripts/glslc ./assets/shaders/skybox.vert -o ./assets/shaders/spv/skybox_vert.spv
./scripts/glslc ./assets/shaders/skybox.frag -o ./assets/shaders/spv/skybox_frag.spv

./scripts/glslc ./assets/shaders/2d.vert -o assets/shaders/spv/2d_vert.spv
./scripts/glslc ./assets/shaders/2d.frag -o assets/shaders/spv/2d_frag.spv

./scripts/glslc ./assets/shaders/ambient_shader.vert -o assets/shaders/spv/ambient_shader_vert.spv
./scripts/glslc ./assets/shaders/ambient_shader.frag -o assets/shaders/spv/ambient_shader_frag.spv

./scripts/glslc ./assets/shaders/grid.vert -o assets/shaders/spv/grid_vert.spv
./scripts/glslc ./assets/shaders/grid.frag -o assets/shaders/spv/grid_frag.spv

./scripts/glslc ./assets/shaders/bbox.vert -o assets/shaders/spv/bbox_vert.spv
./scripts/glslc ./assets/shaders/bbox.frag -o assets/shaders/spv/bbox_frag.spv
