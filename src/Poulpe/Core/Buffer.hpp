#pragma once

namespace Poulpe
{
    struct UniformBufferObject
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 projection;
    };

    struct CubeUniformBufferObject : UniformBufferObject
    {
        int index;
    };
}
