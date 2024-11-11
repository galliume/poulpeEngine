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
            unsigned int index,
            VkDeviceSize alignment
        );

        void bindBufferToMemory(VkBuffer& buffer, VkDeviceSize size);
        void bindImageToMemory(VkImage& image, VkDeviceSize size);
        void clear();
        unsigned int getID() const { return _Index; }
        VkDeviceMemory* getMemory();
        uint32_t getOffset() const { return _Offset; }
        VkDeviceSize getSize() const { return _MaxSize; }
        VkDeviceSize getSpaceLeft() const { return _MaxSize - _Offset; }
        VkDeviceSize getType() const { return _MemoryType; }
        bool hasEnoughSpaceLeft(VkDeviceSize size);
        bool isFull() const {  return _IsFull; }
        void lock() { _MutexMemory.lock(); }
        void unLock() { _MutexMemory.unlock(); }

    private:
        void allocateToMemory();

    private:
        unsigned int _Index{0};
        
        bool _IsAllocated{false};
        bool _IsFull{false};

        VkDeviceSize _Alignment;
        VkDeviceSize _MaxSize;
        uint32_t _MemoryType;
        //@todo check with deviceProps.limits.bufferImageGranularity;
        uint32_t _Offset{0};
 
        std::vector<VkBuffer> _Buffer {};
        VkDevice _Device;
        std::unique_ptr<VkDeviceMemory> _Memory{nullptr};
        std::mutex _MutexMemory;

    };
}
