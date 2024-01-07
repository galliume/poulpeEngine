#pragma once

#include "Poulpe/Core/PlpTypedef.hpp"

#include "Poulpe/Renderer/IRenderer.hpp"

namespace Poulpe
{
    class IVisitable
    {
    public:
        virtual ~IVisitable() {}

        virtual Data* getData() = 0;
        virtual material_t const getMaterial() const = 0;
        virtual const std::string getName() const = 0;
        virtual const std::string getShaderName() const = 0;
        virtual std::vector<Buffer>* getStorageBuffers() = 0;
        virtual std::vector<Buffer>* getUniformBuffers() = 0;

        virtual void addStorageBuffer(Buffer& buffer) = 0;
        virtual bool isDirty() = 0;

        virtual void setApplyPushConstants(
          std::function<void(VkCommandBuffer& commandBuffer,
          VkPipelineLayout pipelineLayout,
          IRenderer* const vulkanAdapter,
          IVisitable* const mesh)> fn) = 0;

        virtual void setData(Data data) = 0;
        virtual void setDescSet(VkDescriptorSet descSet) = 0;
        virtual void setHasBufferStorage(bool has = true) = 0;
        virtual void setHasPushConstants(bool has = true) = 0;
        virtual void setIsDirty(bool is) = 0;
        virtual void setName(std::string_view name) = 0;
        virtual void setShaderName(std::string_view name) = 0;
    };
}
