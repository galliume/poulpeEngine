#pragma once

#include <volk.h>

namespace Poulpe
{
    class DeviceMemory
    {
    public:
        DeviceMemory(
            VkDevice device,
            uint32_t memory_type,
            VkDeviceSize max_size,
            unsigned int index,
            VkDeviceSize alignment
        );

        void bindBufferToMemory(VkBuffer& buffer, VkDeviceSize const offset);
        void bindImageToMemory(VkImage& image, VkDeviceSize const offset);
        void clear();
        unsigned int getID() const { return _index; }
        VkDeviceMemory* getMemory();
        uint32_t getOffset() const { return _offset; }
        VkDeviceSize getSize() const { return _max_size; }
        VkDeviceSize getSpaceLeft() const { return _max_size - _offset; }
        VkDeviceSize getType() const { return _memory_type; }
        bool hasEnoughSpaceLeft(VkDeviceSize size);
        bool isFull() const {  return _is_full; }
        void lock() { _mutex_memory.lock(); }
        void unLock() { _mutex_memory.unlock(); }

    private:
        void allocateToMemory();

    private:
        unsigned int _index{0};

        bool _is_allocated{false};
        bool _is_full{false};

        VkDeviceSize _alignment;
        VkDeviceSize _max_size;
        uint32_t _memory_type;
        //@todo check with deviceProps.limits.bufferImageGranularity;
        VkDeviceSize _offset{0};

        std::vector<VkBuffer> _buffer {};
        VkDevice _device;
        std::unique_ptr<VkDeviceMemory> _memory{nullptr};
        std::mutex _mutex_memory;

    };
}
