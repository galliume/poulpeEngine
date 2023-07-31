#pragma once

namespace Poulpe
{
    struct UniformBufferObject 
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 proj;
    };

    struct CubeUniformBufferObject : UniformBufferObject
    {
        int index;
    };
}
