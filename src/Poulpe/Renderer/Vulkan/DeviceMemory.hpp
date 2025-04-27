#pragma once

#include <volk.h>

namespace Poulpe
{
    class DeviceMemory
    {
    public:
        DeviceMemory(
            VkDevice device,
            VkMemoryPropertyFlags memory_type,
            VkDeviceSize max_size,
            unsigned int index,
            VkDeviceSize alignment
        );

        unsigned int bindBufferToMemory(VkBuffer& buffer, VkDeviceSize const offset);
        void bindImageToMemory(VkImage& image, VkDeviceSize const offset);
        void clear();
        unsigned int getID() const { return _index; }
        VkDeviceMemory* getMemory();
        uint32_t getOffset() const { return _offset; }
        VkDeviceSize getSize() const { return _max_size; }
        VkDeviceSize getSpaceLeft() const { return _max_size - _offset; }
        VkMemoryPropertyFlags getType() const { return _memory_type; }
        bool hasEnoughSpaceLeft(VkDeviceSize size);
        bool isFull() const {  return _is_full; }
        void lock() { _mutex_memory.lock(); }
        void unLock() { _mutex_memory.unlock(); }
        VkBuffer& getBuffer(unsigned int index) { return _buffer.at(index); }
        unsigned int getOffset(unsigned int index) const { return _buffer_offsets.at(index); }

    private:
        void allocateToMemory();

    private:
        unsigned int _index{0};

        bool _is_allocated{false};
        bool _is_full{false};

        VkDeviceSize _alignment;
        VkDeviceSize _max_size;
        VkMemoryPropertyFlags _memory_type;
        //@todo check with deviceProps.limits.bufferImageGranularity;
        VkDeviceSize _offset{0};
        unsigned int _buffer_count{ 0 };
        std::vector<VkDeviceSize> _buffer_offsets{ };

        std::vector<VkBuffer> _buffer {};
        VkDevice _device;
        std::unique_ptr<VkDeviceMemory> _memory{nullptr};
        std::mutex _mutex_memory;

    };
}
