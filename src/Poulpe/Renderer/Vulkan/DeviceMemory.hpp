#pragma once
#include <volk.h>

namespace Poulpe
{
    class DeviceMemory
    {
    public:
        DeviceMemory(
            VkDevice device,
            uint32_t memoryType,
            VkBufferUsageFlags usage,
            VkDeviceSize maxSize
        );

        void bindBufferToMemory(VkBuffer& buffer, VkDeviceSize size);
        void bindImageToMemory(VkImage& image, VkDeviceSize size);
        bool isFull() { return m_IsFull; }
        uint32_t getOffset() { return m_Offset; }
        VkDeviceMemory* getMemory();
        bool hasEnoughSpaceLeft(VkDeviceSize size);
        void lock() { m_MutexMemory.lock(); }
        void unLock() { m_MutexMemory.unlock(); }
        void clear();

    private:
        void allocateToMemory();

    private:
        bool m_IsFull = false;
        bool m_IsAllocated = false;

        std::unique_ptr<VkDeviceMemory> m_Memory = nullptr;

        VkDevice m_Device;
        uint32_t m_MemoryType;
        //VkBufferUsageFlags m_Usage;
        VkDeviceSize m_MaxSize = 429287014;
        std::mutex m_MutexMemory;

        //@todo check with deviceProps.limits.bufferImageGranularity;
        uint32_t m_Offset = 0;
        std::vector<VkBuffer> m_Buffer {};
    };
}
