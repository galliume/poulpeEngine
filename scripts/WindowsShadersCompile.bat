glslc.exe assets/shaders/shader.vert -o assets/shaders/spv/vert.spv
glslc.exe assets/shaders/shader.frag -o assets/shaders/spv/frag.spv

glslc.exe assets/shaders/skybox.vert -o assets/shaders/spv/skybox_vert.spv
glslc.exe assets/shaders/skybox.frag -o assets/shaders/spv/skybox_frag.spv

glslc.exe assets/shaders/2d.vert -o assets/shaders/spv/2d_vert.spv
glslc.exe assets/shaders/2d.frag -o assets/shaders/spv/2d_frag.spv

glslc.exe assets/shaders/ambient_shader.vert -o assets/shaders/spv/ambient_shader_vert.spv
glslc.exe assets/shaders/ambient_shader.frag -o assets/shaders/spv/ambient_shader_frag.spv