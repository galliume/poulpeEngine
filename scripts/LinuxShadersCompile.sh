#!/bin/bash
./scripts/glslc ./assets/shaders/shader.vert -o ./assets/shaders/spv/vert.spv
./scripts/glslc ./assets/shaders/shader.frag -o ./assets/shaders/spv/frag.spv

./scripts/glslc ./assets/shaders/skybox.vert -o ./assets/shaders/spv/skybox_vert.spv
./scripts/glslc ./assets/shaders/skybox.frag -o ./assets/shaders/spv/skybox_frag.spv