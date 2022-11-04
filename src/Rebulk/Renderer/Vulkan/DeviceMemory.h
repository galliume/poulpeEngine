#pragma once
#include <volk.h>

namespace Rbk
{
    class DeviceMemory
    {
    public:
        DeviceMemory(VkDevice device, uint32_t memoryType, VkBufferUsageFlags usage, VkDeviceSize maxMemoryAllocationSize);
        void BindBufferToMemory(VkBuffer buffer, VkDeviceSize size);
        void BindImageToMemory(VkImage image, VkDeviceSize size);
        bool IsFull() { return m_IsFull; }
        uint32_t GetOffset() { return m_Offset; }
        std::shared_ptr<VkDeviceMemory> GetMemory();
        bool HasEnoughSpaceLeft(VkDeviceSize size) { return (m_MaxMemoryAllocationSize > m_Offset + size); }

    private:
        void AllocateToMemory();

    private:
        bool m_IsFull = false;
        bool m_IsAllocated = false;

        std::shared_ptr<VkDeviceMemory> m_Memory = nullptr;
        VkBuffer m_Buffer;

        //@todo maxAllocation
        VkDevice m_Device;
        uint32_t m_MemoryType;
        VkBufferUsageFlags m_Usage;
        VkDeviceSize m_MaxMemoryAllocationSize;

        //@todo check with deviceProps.limits.bufferImageGranularity;
        uint32_t m_Offset = 0;
    };
}