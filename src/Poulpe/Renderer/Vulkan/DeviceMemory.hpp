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
            VkDeviceSize maxSize,
            unsigned int index
        );

        void bindBufferToMemory(VkBuffer& buffer, VkDeviceSize size);
        void bindImageToMemory(VkImage& image, VkDeviceSize size);
        void clear();
        unsigned int getID() const { return m_Index; }
        VkDeviceMemory* getMemory();
        VkDeviceSize getOffset() const { return m_Offset; }
        VkDeviceSize getSize() const { return m_MaxSize; }
        VkDeviceSize getSpaceLeft() const { return m_MaxSize - m_Offset; }
        VkDeviceSize getType() const { return m_MemoryType; }
        bool hasEnoughSpaceLeft(VkDeviceSize size);
        bool isFull() const {  return m_IsFull; }
        void lock() { m_MutexMemory.lock(); }
        void unLock() { m_MutexMemory.unlock(); }

    private:
        void allocateToMemory();

    private:
        unsigned int m_Index{0};
        
        bool m_IsAllocated{false};
        bool m_IsFull{false};

        VkDeviceSize m_MaxSize;
        uint32_t m_MemoryType;
        //@todo check with deviceProps.limits.bufferImageGranularity;
        VkDeviceSize m_Offset{0};
 
        std::vector<VkBuffer> m_Buffer {};
        VkDevice m_Device;
        std::unique_ptr<VkDeviceMemory> m_Memory{nullptr};
        std::mutex m_MutexMemory;

    };
}
