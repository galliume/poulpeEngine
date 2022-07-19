#pragma once

#include <volk.h>

namespace Rbk
{
    //see https://zeux.io/2019/07/17/serializing-pipeline-cache/

    struct PipelineCachePrefixHeader
    {
        uint32_t magic;    // an arbitrary magic header to make sure this is actually our file
        uint32_t dataSize; // equal to *pDataSize returned by vkGetPipelineCacheData
        uint64_t dataHash; // a hash of pipeline cache data, including the header

        uint32_t vendorID;      // equal to VkPhysicalDeviceProperties::vendorID
        uint32_t deviceID;      // equal to VkPhysicalDeviceProperties::deviceID
        uint32_t driverVersion; // equal to VkPhysicalDeviceProperties::driverVersion
        uint32_t driverABI;     // equal to sizeof(void*)

        uint8_t uuid[VK_UUID_SIZE]; // equal to VkPhysicalDeviceProperties::pipelineCacheUUID
    };


    class CacheManager
    {
    public:
        bool SavePipelineCache();

    private:
        const std::string m_CachePath = "cache";
    };
}