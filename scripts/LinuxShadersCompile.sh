#!/bin/bash
glslc shaders/shader.vert -o shaders/spv/vert.spv
glslc shaders/shader.frag -o shaders/spv/frag.spv

glslc shaders/skybox.vert -o shaders/spv/skybox_vert.spv
glslc shaders/skybox.frag -o shaders/spv/skybox_frag.spv