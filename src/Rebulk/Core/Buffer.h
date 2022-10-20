#pragma once

namespace Rbk
{
    struct UniformBufferObject 
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };

    struct CubeUniformBufferObject : UniformBufferObject
    {
        int index;
    };
}
