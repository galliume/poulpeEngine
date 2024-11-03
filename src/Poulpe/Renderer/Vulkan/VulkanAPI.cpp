#include "VulkanAPI.hpp"

#include <iostream>
#include <filesystem>
#include <set>
#include <volk.h>

//@todo this class needs a huge clean up
namespace Poulpe {

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT const * pCreateInfo,
        VkAllocationCallbacks const * pAllocator, VkDebugUtilsMessengerEXT* pCallback)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pCallback);
        } else {
            std::cerr << "Debug utils extension not present";
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    //static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
    //    VkAllocationCallbacks const * pAllocator)
    //{
    //    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
    //        "vkDestroyDebugUtilsMessengerEXT");
    //    if (func != nullptr) {
    //        func(instance, debugMessenger, pAllocator);
    //    }
    //}

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        VkDebugUtilsMessengerCallbackDataEXT const * pCallbackData,
          void* pUserData)
    {
        spdlog::set_pattern("%^[%T] %n: %v%$");

        switch (messageSeverity)
        {
          case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
          {
              PLP_FATAL("{} : {}", messageType, pCallbackData->pMessage);
              break;
          }
          case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
          {
              PLP_WARN("{} : {}", messageType, pCallbackData->pMessage);
              break;
          }
          case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
          case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
          case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
          {
              PLP_TRACE("{} : {}", messageType, pCallbackData->pMessage);
          }
        }
        return VK_FALSE;
    }

    VulkanAPI::VulkanAPI(Window* window)
        : m_Window(window)
    {
#ifdef PLP_DEBUG_BUILD
        m_EnableValidationLayers = true;
#else
        m_EnableValidationLayers = false;
#endif

        if (!glfwVulkanSupported()) {
            throw std::runtime_error("vulkan not supported");
        }

        if (volkInitialize() != VK_SUCCESS) {
            throw std::runtime_error("Failed to initialize volk");
        }

        loadRequiredExtensions();
        enumerateExtensions();
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        initDetails();
        initMemoryPool();

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        //VkFence fence;

        vkCreateFence(m_Device, & fenceInfo, nullptr, & m_FenceAcquireImage);
        vkCreateFence(m_Device, & fenceInfo, nullptr, & m_FenceSubmit);
        vkCreateFence(m_Device, & fenceInfo, nullptr, & m_FenceBuffer);
    }

    void VulkanAPI::initMemoryPool()
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, & memProperties);

        m_DeviceMemoryPool = std::make_unique<DeviceMemoryPool>(
          m_DeviceProperties2, m_DeviceMaintenance3Properties, memProperties);
    }

    std::string VulkanAPI::getAPIVersion()
    {
        if (m_apiVersion.empty()) {
            uint32_t instanceVersion = VK_API_VERSION_1_3;

            //@todo fix unused ?
            auto tmp = PFN_vkEnumerateInstanceVersion(vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));
            (void)tmp;

            if (vkEnumerateInstanceVersion) {
                vkEnumerateInstanceVersion(&instanceVersion);
            }

            uint32_t major = VK_VERSION_MAJOR(instanceVersion);
            uint32_t minor = VK_VERSION_MINOR(instanceVersion);
            uint32_t patch = VK_VERSION_PATCH(instanceVersion);

            m_apiVersion = std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
        }
        return m_apiVersion;
    }

    void VulkanAPI::createInstance()
    {
        std::string message;

        if (!isValidationLayersEnabled() && !checkValidationLayerSupport()) {
            PLP_WARN("Validations layers not available !");
        }

        VkApplicationInfo appInfo{};

        appInfo.apiVersion = VK_API_VERSION_1_3;
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "PoulpeEngine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(m_RequiredExtensions.size());
        createInfo.ppEnabledExtensionNames = m_RequiredExtensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

        if (m_EnableValidationLayers) {
            debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT 
                | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugCreateInfo.pfnUserCallback = DebugCallback;
            debugCreateInfo.pUserData = nullptr;

            createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
            createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
            PLP_TRACE("Validations enabled");
        } else {
            PLP_TRACE("Validations disabled");
            createInfo.enabledLayerCount = 0;
        }

        VkResult result = VK_SUCCESS;

        result = vkCreateInstance(& createInfo, nullptr, &m_Instance);

        if (VK_SUCCESS != result) {
            PLP_FATAL("Can't create VK instance : {}", result);
            throw std::runtime_error("Can't create VK instance.");
        }

        m_InstanceCreated = true;

        volkLoadInstance(m_Instance);
    }

    void VulkanAPI::initDetails()
    {
        m_SwapChainSupport = querySwapChainSupport(m_PhysicalDevice);
        m_SurfaceFormat = chooseSwapSurfaceFormat(m_SwapChainSupport.formats);
        m_PresentMode = chooseSwapPresentMode(m_SwapChainSupport.presentModes);
        m_SwapChainExtent = chooseSwapExtent(m_SwapChainSupport.capabilities);
        m_SwapChainImageFormat = m_SurfaceFormat.format;
    }

    void VulkanAPI::enumerateExtensions()
    {
        vkEnumerateInstanceExtensionProperties(nullptr, & m_ExtensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(m_ExtensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, & m_ExtensionCount, extensions.data());
        m_Extensions = extensions;
    }

    bool VulkanAPI::checkValidationLayerSupport()
    {
        uint32_t layerCount{ 0 };
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (std::string const layerName : m_ValidationLayers) {
            bool layerFound = false;

            for (auto const & layerProperties : availableLayers) {
                if (strcmp(layerName.c_str(), layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound) {
                return false;
            }
        }
        m_LayersAvailable = availableLayers;

        return false;
    }

    void VulkanAPI::loadRequiredExtensions()
    {
        uint32_t glfwExtensionCount{ 0 };
        char const ** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(& glfwExtensionCount);

        std::vector<char const*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        
        if (m_EnableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            //extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME); deprecated
            //extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME); deprecated
        }
        m_RequiredExtensions = extensions;
    }

    void VulkanAPI::setupDebugMessenger()
    {
        if (!m_EnableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT 
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;
        createInfo.pUserData = nullptr;

        if (CreateDebugUtilsMessengerEXT(m_Instance, & createInfo, nullptr, &m_DebugMessengerCallback) != VK_SUCCESS) {
            PLP_ERROR("Can't create debug messenger.");
        }
    }

    void VulkanAPI::pickPhysicalDevice()
    {
        uint32_t deviceCount{ 0 };
        vkEnumeratePhysicalDevices(m_Instance, & deviceCount, nullptr);

        if (deviceCount == 0) {
            PLP_FATAL("failed to find GPUs with Vulkan support!");
            exit(-1);
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_Instance, & deviceCount, devices.data());

        for (auto const & device : devices) {
            if (isDeviceSuitable(device)) {
                m_PhysicalDevice = device;

                VkPhysicalDeviceProperties deviceProperties;
                VkPhysicalDeviceFeatures deviceFeatures;
                VkPhysicalDeviceMaintenance3Properties deviceMaintenance3Properties{};
                deviceMaintenance3Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES;
            
                VkPhysicalDeviceProperties2 deviceProperties2{};
                deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
                deviceProperties2.pNext = &deviceMaintenance3Properties;

                vkGetPhysicalDeviceProperties(device, &deviceProperties);
                vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
                vkGetPhysicalDeviceProperties2(device, &deviceProperties2);

                m_DeviceProps = deviceProperties;
                m_DeviceFeatures = deviceFeatures;
                m_DeviceProperties2 = deviceProperties2;
                m_DeviceMaintenance3Properties = deviceMaintenance3Properties;

                m_MsaaSamples = getMaxUsableSampleCount();

                break;
            }
        }
        if (m_PhysicalDevice == VK_NULL_HANDLE) {
            PLP_FATAL("failed to find a suitable GPU");
            exit(-1);
        }
    }

    bool VulkanAPI::isDeviceSuitable(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices = findQueueFamilies(device);
        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;

        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    bool VulkanAPI::checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount{ 0 };
        vkEnumerateDeviceExtensionProperties(device, nullptr, & extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, & extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end());

        for (auto const & extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }
        return requiredExtensions.empty();
    }

    QueueFamilyIndices VulkanAPI::findQueueFamilies(VkPhysicalDevice device)
    {
        uint32_t queueFamilyCount{ 0 };
        vkGetPhysicalDeviceQueueFamilyProperties(device, & queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, & queueFamilyCount, queueFamilies.data());

        uint32_t i{ 0 };
        for (auto const & queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                m_QueueFamilyIndices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, & presentSupport);

            if (presentSupport) {
                m_QueueFamilyIndices.presentFamily = i;
            }
            if (m_QueueFamilyIndices.isComplete()) {
                break;
            }
            i++;
        }
        return m_QueueFamilyIndices;
    }

    void VulkanAPI::createLogicalDevice()
    {
        QueueFamilyIndices indices = findQueueFamilies(m_PhysicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        float queuePriority{ 1.0f };
        m_GraphicsQueues.resize(m_queueCount);
        m_PresentQueues.resize(m_queueCount);

        for (uint32_t queueFamily : uniqueQueueFamilies) {

            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = m_queueCount;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.fillModeNonSolid =  VK_TRUE;
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.sampleRateShading = VK_TRUE;
        deviceFeatures.imageCubeArray = VK_TRUE;
        deviceFeatures.geometryShader = VK_TRUE;

        VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexing{};
        descriptorIndexing.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
        descriptorIndexing.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        descriptorIndexing.runtimeDescriptorArray = VK_TRUE;
        descriptorIndexing.descriptorBindingVariableDescriptorCount = VK_TRUE;
        descriptorIndexing.descriptorBindingPartiallyBound = VK_TRUE;

        VkPhysicalDeviceVulkan13Features vkFeatures13 = {};
        vkFeatures13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        vkFeatures13.dynamicRendering = VK_TRUE;
        vkFeatures13.synchronization2 = VK_TRUE;
        vkFeatures13.pipelineCreationCacheControl = VK_TRUE;
        vkFeatures13.pNext = &descriptorIndexing;

        VkPhysicalDeviceShaderDrawParametersFeatures shader_draw_parameters_features = {};
        shader_draw_parameters_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
        shader_draw_parameters_features.pNext = &vkFeatures13;
        shader_draw_parameters_features.shaderDrawParameters = VK_TRUE;

        VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extDynamicState{};
        extDynamicState.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT;
        extDynamicState.extendedDynamicState3DepthClampEnable = VK_TRUE;
        extDynamicState.extendedDynamicState3RasterizationSamples = VK_TRUE;
        extDynamicState.pNext = &shader_draw_parameters_features;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(m_DeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();
        createInfo.pNext = &extDynamicState;

        if (vkCreateDevice(m_PhysicalDevice, & createInfo, nullptr, &m_Device) != VK_SUCCESS) {
            PLP_FATAL("failed to create logical device!");
            return;
        }

        for (uint32_t i { 0 }; i < m_queueCount; i++) {
            m_GraphicsQueues[i] = VK_NULL_HANDLE;
            m_PresentQueues[i] = VK_NULL_HANDLE;
            vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, & m_GraphicsQueues[i]);
            vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, & m_PresentQueues[i]);
        }
        volkLoadDevice(m_Device);
    }

    void VulkanAPI::createSurface()
    {
        VkResult result = glfwCreateWindowSurface(m_Instance, m_Window->get(), nullptr, & m_Surface);

        if (result != VK_SUCCESS) {
            PLP_FATAL("failed to create window surface!");
            throw std::runtime_error("failed to create window surface!");
        }
    }

    const SwapChainSupportDetails VulkanAPI::querySwapChainSupport(VkPhysicalDevice device) const
    {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, & details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, & formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, & formatCount, details.formats.data());
        }
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, & presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, details.presentModes.data());
        }
        return details;
    }

    VkSurfaceFormatKHR VulkanAPI::chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> const & availableFormats)
    {
        for (auto const & availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM 
                && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    VkPresentModeKHR VulkanAPI::chooseSwapPresentMode(std::vector<VkPresentModeKHR> const & availablePresentModes)
    {
        for (auto const & availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VulkanAPI::chooseSwapExtent( VkSurfaceCapabilitiesKHR const & capabilities)
    {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        }

        int width, height;
        glfwGetFramebufferSize(m_Window->get(), &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }

    uint32_t VulkanAPI::getImageCount() const
    {
        uint32_t imageCount = m_SwapChainSupport.capabilities.minImageCount;

        if (m_SwapChainSupport.capabilities.maxImageCount > 0 
            && imageCount > m_SwapChainSupport.capabilities.maxImageCount) {
            imageCount = m_SwapChainSupport.capabilities.maxImageCount;
        }
        return imageCount;
    }

    VkSwapchainKHR VulkanAPI::createSwapChain(std::vector<VkImage> & swapChainImages,
        VkSwapchainKHR const & oldSwapChain)
    {
        VkSwapchainKHR swapChain = VK_NULL_HANDLE;

        uint32_t imageCount = getImageCount();

        VkImageSubresourceLayers srcImgsubrcLayers;
        srcImgsubrcLayers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        srcImgsubrcLayers.mipLevel = 1;
        srcImgsubrcLayers.baseArrayLayer = 1;
        srcImgsubrcLayers.layerCount = 1;

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_Surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = m_SurfaceFormat.format;
        createInfo.imageColorSpace = m_SurfaceFormat.colorSpace;
        createInfo.imageExtent = m_SwapChainExtent;
        createInfo.imageArrayLayers = 1;//1 unless stereoscopic app
        //use of VK_IMAGE_USAGE_TRANSFER_DST_BIT if post process is needed
        createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT 
            | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        createInfo.preTransform = m_SwapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = m_PresentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = oldSwapChain;

        QueueFamilyIndices indices = findQueueFamilies(m_PhysicalDevice);
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }
        VkResult result;

        result = vkCreateSwapchainKHR(m_Device, & createInfo, nullptr, & swapChain);

        if (result != VK_SUCCESS) {
            PLP_FATAL("Swap chain failed " + std::to_string(result));
        }
        vkGetSwapchainImagesKHR(m_Device, swapChain, & imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_Device, swapChain, & imageCount, swapChainImages.data());

        return swapChain;
    }

    bool VulkanAPI::souldResizeSwapChain()
    {
        VkExtent2D currentExtent = getSwapChainExtent();

        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_PhysicalDevice);
        //VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
        //VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t newWidth = extent.width;
        uint32_t newHeight = extent.height;

        return (currentExtent.width == newWidth && currentExtent.height == newHeight) ? false : true;
    }

    VkImageView VulkanAPI::createImageView(VkImage image, VkFormat format, uint32_t mipLevels,
        VkImageAspectFlags aspectFlags)
    {
        VkImageView swapChainImageView{};

        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = aspectFlags;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = mipLevels;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VkResult result{ VK_SUCCESS };

        result = vkCreateImageView(m_Device, & createInfo, nullptr, & swapChainImageView);

        if (result != VK_SUCCESS) {
            PLP_ERROR("failed to create image views!");
        }
        return swapChainImageView;
    }

    VkImageView VulkanAPI::createSkyboxImageView(VkImage image, VkFormat format, uint32_t mipLevels,
        VkImageAspectFlags aspectFlags)
    {
        VkImageView swapChainImageView{};

        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        createInfo.format = format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_A;
        createInfo.subresourceRange = { aspectFlags, 0, 1, 0, 1 };
        createInfo.subresourceRange.levelCount = mipLevels;
        createInfo.subresourceRange.layerCount = 6;

        VkResult result{ VK_SUCCESS };

        result = vkCreateImageView(m_Device, & createInfo, nullptr, & swapChainImageView);

        if (result != VK_SUCCESS) {
            PLP_ERROR("failed to create image views!");
        }
        return swapChainImageView;
    }

    VkDescriptorSetLayout VulkanAPI::createDescriptorSetLayout(
        std::vector<VkDescriptorSetLayoutBinding> const & pBindings)
    {
        VkDescriptorSetLayout descriptorSetLayout{};

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(pBindings.size());
        layoutInfo.pBindings = pBindings.data();
        layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        //VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
        if (vkCreateDescriptorSetLayout(m_Device, & layoutInfo, nullptr, & descriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
        return descriptorSetLayout;
    }

    VkPipelineLayout VulkanAPI::createPipelineLayout(std::vector<VkDescriptorSetLayout> const & descriptorSetLayouts,
        std::vector<VkPushConstantRange> const & pushConstants)
    {
        VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        if (0 != pushConstants.size()) {
            pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
            pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();
        }
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

        VkResult result = vkCreatePipelineLayout(m_Device, & pipelineLayoutInfo, nullptr, & graphicsPipelineLayout);

        if (result != VK_SUCCESS) {
            PLP_ERROR("failed to create pipeline layout!");
        }
        return graphicsPipelineLayout;
    }

    //@todo refactor this...
    VkPipeline VulkanAPI::createGraphicsPipeline(
        VkPipelineLayout pipelineLayout,
        std::string_view name,
        std::vector<VkPipelineShaderStageCreateInfo> shadersCreateInfos,
        VkPipelineVertexInputStateCreateInfo & vertexInputInfo,
        VkCullModeFlagBits cullMode,
        bool depthTestEnable,
        bool depthWriteEnable,
        bool stencilTestEnable,
        int polygoneMode,
        bool hasColorAttachment,
        bool dynamicDepthBias)
    {
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_SwapChainExtent.width);
        viewport.height = static_cast<float>(m_SwapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = m_SwapChainExtent;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = & viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = & scissor;

        //@todo extension not working ?
        VkPipelineRasterizationDepthClipStateCreateInfoEXT depthClipState{};
        depthClipState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT;
        depthClipState.depthClipEnable = VK_TRUE;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = static_cast<VkPolygonMode>(polygoneMode);
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = cullMode;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.f;
        rasterizer.depthBiasClamp = 0.0f;
        rasterizer.depthBiasSlopeFactor = 0.0f;
        //rasterizer.pNext = &depthClipState;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;//m_MsaaSamples;
        multisampling.minSampleShading = 0.2f;
        multisampling.pSampleMask = nullptr;
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable = VK_FALSE;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = (depthTestEnable == true) ? VK_TRUE : VK_FALSE;
        depthStencil.depthWriteEnable = (depthWriteEnable == true) ? VK_TRUE : VK_FALSE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f;
        depthStencil.maxDepthBounds = 1.0f;
        depthStencil.stencilTestEnable = (stencilTestEnable == true) ? VK_TRUE : VK_FALSE;
        depthStencil.front = {};
        depthStencil.back = {};

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
            | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = & colorBlendAttachment;
        colorBlending.blendConstants[0] = 1.0f;
        colorBlending.blendConstants[1] = 1.0f;
        colorBlending.blendConstants[2] = 1.0f;
        colorBlending.blendConstants[3] = 1.0f;

        std::vector<VkDynamicState> dynamicStates{
            VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        if (dynamicDepthBias) dynamicStates.emplace_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
        
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = dynamicStates.size();
        dynamicState.pDynamicStates = dynamicStates.data();

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = static_cast<uint32_t>(shadersCreateInfos.size());
        pipelineInfo.pStages = shadersCreateInfos.data();
        pipelineInfo.pVertexInputState = & vertexInputInfo;
        pipelineInfo.pInputAssemblyState = & inputAssembly;
        pipelineInfo.pViewportState = & viewportState;
        pipelineInfo.pRasterizationState = & rasterizer;
        pipelineInfo.pMultisampleState = & multisampling;
        pipelineInfo.pDepthStencilState = & depthStencil;
        pipelineInfo.pColorBlendState = & colorBlending;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = VK_NULL_HANDLE;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;
        pipelineInfo.pDynamicState = & dynamicState;

        VkFormat format = getSwapChainImageFormat();
        VkFormat depthFormat = findDepthFormat();

        VkPipelineRenderingCreateInfoKHR renderingCreateInfo = { };
        renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
        renderingCreateInfo.colorAttachmentCount = hasColorAttachment ? 1 : 0;
        if (hasColorAttachment) renderingCreateInfo.pColorAttachmentFormats = & format;
        renderingCreateInfo.depthAttachmentFormat = (hasColorAttachment) ? depthFormat : VK_FORMAT_D32_SFLOAT; //(VK_FORMAT_D32_SFLOAT) 

        pipelineInfo.pNext = & renderingCreateInfo;

        VkPipeline graphicsPipeline = nullptr;

        {
            //std::lock_guard<std::mutex> guard(m_MutexGraphicsPipeline);

            //@todo move to a FileManager
            //@todo option to enable / disable pipeline cache
            std::string cacheFileName = "./cache/pipeline_cache_data_" + std::to_string(m_DeviceProps.vendorID) +
                "_" + std::to_string(m_DeviceProps.deviceID) + "_" + name.data() + ".bin";

            bool badCache = false;
            std::streamsize cacheFileSize = 0;
            void* cacheFileData = nullptr;
            std::string pReadFile = "";

            if (std::filesystem::exists(cacheFileName)) {
                std::ifstream fStream(cacheFileName, std::ios::in | std::ios::binary);
                cacheFileSize = std::filesystem::file_size(cacheFileName);
                pReadFile.resize(cacheFileSize);
                fStream.read(pReadFile.data(), cacheFileSize);
                fStream.close();
            } else {
                //PLP_TRACE("Pipeline cache miss!");
                badCache = true;
            }

            if (!pReadFile.empty()) {
                cacheFileData = (char*)malloc(sizeof(char) * cacheFileSize);
                cacheFileData = pReadFile.data();

                if (cacheFileData == nullptr) {
                    PLP_WARN("Cannot allocate memory to pipeline cache");
                }
                //PLP_TRACE("Pipeline cache HIT from {}", cacheFileName);
            }

            if (cacheFileData != nullptr) {
                uint32_t headerLength = 0;
                uint32_t cacheHeaderVersion = 0;
                uint32_t vendorID = 0;
                uint32_t deviceID = 0;
                uint8_t pipelineCacheUUID[VK_UUID_SIZE] = {};

                memcpy(& headerLength, static_cast<uint8_t*>(cacheFileData) + 0, 4);
                memcpy(& cacheHeaderVersion, static_cast<uint8_t*>(cacheFileData) + 4, 4);
                memcpy(& vendorID, static_cast<uint8_t*>(cacheFileData) + 8, 4);
                memcpy(& deviceID, static_cast<uint8_t*>(cacheFileData) + 12, 4);
                memcpy(& pipelineCacheUUID, static_cast<uint8_t*>(cacheFileData) + 16, VK_UUID_SIZE);

                if (headerLength <= 0) {
                    badCache = true;
                    PLP_ERROR("Bad header length in {} - {}", cacheFileName, headerLength);
                }
                if (cacheHeaderVersion != VK_PIPELINE_CACHE_HEADER_VERSION_ONE) {
                    badCache = true;
                    PLP_ERROR("Unsupported cache header version in {} got {}", cacheFileName, cacheHeaderVersion);
                }
                if (vendorID != static_cast<uint32_t>(m_DeviceProps.vendorID)) {
                    badCache = true;
                    PLP_ERROR("Vendor ID mismatch in {} got {} expect {}", cacheFileName, vendorID,
                        m_DeviceProps.vendorID);
                }
                if (deviceID != static_cast<uint32_t>(m_DeviceProps.deviceID)) {
                    badCache = true;
                    PLP_ERROR("Device ID mismatch in {} got {} expect {}", cacheFileName, deviceID,
                        m_DeviceProps.deviceID);
                }
                if (memcmp(pipelineCacheUUID,  m_DeviceProps.pipelineCacheUUID, sizeof(pipelineCacheUUID)) != 0) {
                    PLP_ERROR("UUID mismatch in {} got {} expect {}", cacheFileName, *pipelineCacheUUID, 
                        *m_DeviceProps.pipelineCacheUUID);
                }

                if (badCache) {
                    free(cacheFileData);
                    cacheFileSize = 0;
                    cacheFileData = nullptr;

                    PLP_TRACE("Deleting cache entry {} to repopulate.", cacheFileName);

                    if (remove(cacheFileName.c_str()) != 0) {
                        PLP_ERROR("Reading error");
                    }
                }
            }

            VkPipelineCache pipelineCache;
            VkPipelineCacheCreateInfo pCreateInfo = { };
            pCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
            pCreateInfo.initialDataSize = cacheFileSize;
            pCreateInfo.pInitialData = &cacheFileData;

            VkResult result = vkCreatePipelineCache(m_Device, & pCreateInfo, nullptr, & pipelineCache);

            if (result != VK_SUCCESS) {
                PLP_ERROR("failed to get graphics pipeline cache size!");
            }
            result = vkCreateGraphicsPipelines(m_Device, pipelineCache, 1, & pipelineInfo, nullptr, & graphicsPipeline);

            if (result != VK_SUCCESS) {
                PLP_ERROR("failed to create graphics pipeline cache!");
            }
            if (result == VK_SUCCESS && badCache) {
                size_t pDataSize = 0;
                void* data = nullptr;

                //first call to get cache size with nullptr
                result = vkGetPipelineCacheData(m_Device, pipelineCache, & pDataSize, nullptr);

                if (result != VK_SUCCESS) {
                    PLP_ERROR("failed to get graphics pipeline cache size!");
                }
                data = (char*)malloc(sizeof(char) * pDataSize);

                if (!data) {
                    PLP_ERROR("failed to resize cache buffer!");
                } else {
                    result = vkGetPipelineCacheData(m_Device, pipelineCache, & pDataSize, data);

                    //@todo move to a FileManager
                    if (result == VK_SUCCESS) {
                        std::ofstream ostrm(cacheFileName, std::ios::binary);
                        ostrm.write(static_cast<const char*>(data), pDataSize);
                        ostrm.close();
                        PLP_TRACE("cacheData written to {}", cacheFileName);
                    }
                }
            }
        }
        return graphicsPipeline;
    }

    VkShaderModule VulkanAPI::createShaderModule(std::vector<char> const & code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule{};
        VkResult result{ VK_SUCCESS };

        result = vkCreateShaderModule(m_Device, & createInfo, nullptr, &shaderModule);

        if (result != VK_SUCCESS) {
            PLP_ERROR("failed to create shader module!");
        }
        return shaderModule;
    }

    VkRenderPass* VulkanAPI::createRenderPass(VkSampleCountFlagBits const & msaaSamples)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_PhysicalDevice);
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);

        VkRenderPass* renderPass = new VkRenderPass();

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = surfaceFormat.format;
        colorAttachment.samples = msaaSamples;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription colorAttachmentResolve{};
        colorAttachmentResolve.format = surfaceFormat.format;
        colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentResolveRef{};
        colorAttachmentResolveRef.attachment = 2;
        colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
            | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
            | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = msaaSamples;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = & colorAttachmentRef;
        subpass.pDepthStencilAttachment = & depthAttachmentRef;
        subpass.pResolveAttachments = & colorAttachmentResolveRef;

        std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = & subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = & dependency;

        VkResult result = vkCreateRenderPass(m_Device, & renderPassInfo, nullptr, renderPass);

        if (result != VK_SUCCESS) {
            PLP_FATAL("failed to create render pass!");
        }
        return renderPass;
    }

    std::vector<VkFramebuffer> VulkanAPI::createFramebuffers(VkRenderPass renderPass,
        std::vector<VkImageView> swapChainImageViews, std::vector<VkImageView> depthImageView,
        std::vector<VkImageView> colorImageView)
    {
        std::vector<VkFramebuffer> swapChainFramebuffers;

        swapChainFramebuffers.resize(swapChainImageViews.size());

        for (size_t i { 0 }; i < swapChainImageViews.size(); i++) {

            std::array<VkImageView, 3> attachments = {
                *colorImageView.data(),
                *depthImageView.data(),
                swapChainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = m_SwapChainExtent.width;
            framebufferInfo.height = m_SwapChainExtent.height;
            framebufferInfo.layers = 1;

            VkResult result = vkCreateFramebuffer(m_Device, & framebufferInfo, nullptr, & swapChainFramebuffers[i]);

            if (result != VK_SUCCESS) {
                PLP_ERROR("failed to create framebuffer!");
            }
        }
        return swapChainFramebuffers;
    }


    VkCommandPool VulkanAPI::createCommandPool()
    {
        VkCommandPool commandPool;

        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_PhysicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        VkResult result = vkCreateCommandPool(m_Device, & poolInfo, nullptr, & commandPool);

        if (result != VK_SUCCESS) {
            PLP_ERROR("failed to create command pool!");
        }
        return commandPool;
    }

    std::vector<VkCommandBuffer> VulkanAPI::allocateCommandBuffers(VkCommandPool commandPool, uint32_t size,
        bool isSecondary)
    {
        std::vector<VkCommandBuffer> commandBuffers;

        commandBuffers.resize(size);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = (!isSecondary) ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        VkResult result = vkAllocateCommandBuffers(m_Device, & allocInfo, commandBuffers.data());

        if (result != VK_SUCCESS) {
            PLP_ERROR("failed to allocate command buffers!");
        }
        return commandBuffers;
    }

    void VulkanAPI::beginCommandBuffer(VkCommandBuffer& commandBuffer, VkCommandBufferUsageFlagBits flags,
        VkCommandBufferInheritanceInfo inheritanceInfo)
    {
        vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | flags;
        beginInfo.pInheritanceInfo = (flags == VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) ? & inheritanceInfo : nullptr;

        if (VK_SUCCESS != vkBeginCommandBuffer(commandBuffer, & beginInfo)) {
            throw std::runtime_error("failed to create descriptor pool");
        }
    }

    void VulkanAPI::setViewPort(VkCommandBuffer& commandBuffer)
    {
        VkViewport viewport;
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = static_cast<float>(m_SwapChainExtent.width);
        viewport.height = static_cast<float>(m_SwapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    }

    void VulkanAPI::setScissor(VkCommandBuffer& commandBuffer)
    {
        VkRect2D scissor = { { 0, 0 }, { static_cast<uint32_t>(m_SwapChainExtent.width),
            static_cast<uint32_t>(m_SwapChainExtent.height) } };

        vkCmdSetScissor(commandBuffer, 0, 1, & scissor);
    }

    void VulkanAPI::bindPipeline(VkCommandBuffer& commandBuffer, VkPipeline pipeline)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

   /* std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>>
    VulkanAPI::createSyncObjects(std::vector<VkImage> swapChainImages)
    {
        std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> semaphores{};

        std::vector<VkSemaphore> imageAvailableSemaphores{};
        std::vector<VkSemaphore> renderFinishedSemaphores{};

        imageAvailableSemaphores.resize(m_MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(m_MAX_FRAMES_IN_FLIGHT);
        m_InFlightFences.resize(m_MAX_FRAMES_IN_FLIGHT);
        m_ImagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (int i { 0 }; i < m_MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr,
                & imageAvailableSemaphores[static_cast<size_t>(i)]) != VK_SUCCESS
                || vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr,
                    & renderFinishedSemaphores[static_cast<size_t>(i)]) != VK_SUCCESS
                || vkCreateFence(m_Device, &fenceInfo, nullptr,
                    & m_InFlightFences[static_cast<size_t>(i)]) != VK_SUCCESS)
            {
                PLP_ERROR("failed to create semaphores!");
            }
        }
        semaphores.first = imageAvailableSemaphores;
        semaphores.second = renderFinishedSemaphores;

        return semaphores;
    }*/

    VkDescriptorPool VulkanAPI::createDescriptorPool(std::vector<VkDescriptorPoolSize> const & poolSizes,
        uint32_t maxSets, VkDescriptorPoolCreateFlags flags)
    {
        VkDescriptorPool descriptorPool{};

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = poolSizes.size();
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = maxSets;
        poolInfo.flags = flags;

        if (vkCreateDescriptorPool(m_Device, & poolInfo, nullptr, & descriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool");
        }
        return descriptorPool;
    }

    VkDescriptorSet VulkanAPI::createDescriptorSets(VkDescriptorPool const & descriptorPool,
        std::vector<VkDescriptorSetLayout> const  & descriptorSetLayouts, uint32_t count)
    {
        VkDescriptorSet descriptorSet{};
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = count;
        allocInfo.pSetLayouts = descriptorSetLayouts.data();

        VkResult result = vkAllocateDescriptorSets(m_Device, & allocInfo, & descriptorSet);

        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets");
        }
        return descriptorSet;
    }

    void VulkanAPI::updateDescriptorSets(std::vector<Buffer> & uniformBuffers, VkDescriptorSet & descriptorSet,
        std::vector<VkDescriptorImageInfo> & imageInfo, VkDescriptorType type)
    {
        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
        std::vector<VkDescriptorBufferInfo> bufferInfos;

        std::for_each(std::begin(uniformBuffers), std::end(uniformBuffers),
        [& bufferInfos](const Buffer & uniformBuffer)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffer.buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = VK_WHOLE_SIZE;
            bufferInfos.emplace_back(bufferInfo);
        });

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSet;
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = type;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = bufferInfos.data();

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSet;
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = imageInfo.size();
        descriptorWrites[1].pImageInfo = imageInfo.data();

        vkUpdateDescriptorSets(m_Device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    void VulkanAPI::updateDescriptorSet(
        Buffer & uniformBuffer,
        VkDescriptorSet & descriptorSet,
        std::vector<VkDescriptorImageInfo> & imageInfo,
        VkDescriptorType type)
    {
        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
        std::vector<VkDescriptorBufferInfo> bufferInfos;

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffer.buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = VK_WHOLE_SIZE;
        bufferInfos.emplace_back(bufferInfo);

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSet;
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = type;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = bufferInfos.data();

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSet;
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = imageInfo.size();
        descriptorWrites[1].pImageInfo = imageInfo.data();

        vkUpdateDescriptorSets(m_Device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    void VulkanAPI::updateDescriptorSet(
        Buffer & uniformBuffer,
        Buffer & storageBuffer,
        VkDescriptorSet & descriptorSet,
        std::vector<VkDescriptorImageInfo> & imageInfo)
    {
        std::array<VkWriteDescriptorSet, 3> descriptorWrites{};
        std::vector<VkDescriptorBufferInfo> bufferInfos;
        std::vector<VkDescriptorBufferInfo> storageBufferInfos;

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffer.buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = VK_WHOLE_SIZE;
        bufferInfos.emplace_back(bufferInfo);

        VkDescriptorBufferInfo storageBufferInfo{};
        storageBufferInfo.buffer = storageBuffer.buffer;
        storageBufferInfo.offset = 0;
        storageBufferInfo.range = VK_WHOLE_SIZE;
        storageBufferInfos.emplace_back(storageBufferInfo);

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSet;
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = bufferInfos.data();

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSet;
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = imageInfo.size();
        descriptorWrites[1].pImageInfo = imageInfo.data();

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = descriptorSet;
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[2].descriptorCount = storageBufferInfos.size();
        descriptorWrites[2].pBufferInfo = storageBufferInfos.data();

        vkUpdateDescriptorSets(m_Device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    void VulkanAPI::updateDescriptorSets(
      std::vector<Buffer>& uniformBuffers,
      std::vector<Buffer>& storageBuffers,
      VkDescriptorSet& descriptorSet,
      std::vector<VkDescriptorImageInfo>& imageInfo)
    {
      std::array<VkWriteDescriptorSet, 3> descriptorWrites{};
      std::vector<VkDescriptorBufferInfo> bufferInfos;
      std::vector<VkDescriptorBufferInfo> storageBufferInfos;

      std::for_each(std::begin(uniformBuffers), std::end(uniformBuffers),
        [&bufferInfos](const Buffer& buffer)
        {
          VkDescriptorBufferInfo bufferInfo{};
          bufferInfo.buffer = buffer.buffer;
          bufferInfo.offset = 0;
          bufferInfo.range = VK_WHOLE_SIZE;
          bufferInfos.emplace_back(bufferInfo);
        });

      std::for_each(std::begin(storageBuffers), std::end(storageBuffers),
        [&storageBufferInfos](const Buffer& buffer)
        {
          VkDescriptorBufferInfo bufferInfo{};
          bufferInfo.buffer = buffer.buffer;
          bufferInfo.offset = 0;
          bufferInfo.range = VK_WHOLE_SIZE;
          storageBufferInfos.emplace_back(bufferInfo);
        });

      descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[0].dstSet = descriptorSet;
      descriptorWrites[0].dstBinding = 0;
      descriptorWrites[0].dstArrayElement = 0;
      descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrites[0].descriptorCount = 1;
      descriptorWrites[0].pBufferInfo = bufferInfos.data();

      descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[1].dstSet = descriptorSet;
      descriptorWrites[1].dstBinding = 1;
      descriptorWrites[1].dstArrayElement = 0;
      descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      descriptorWrites[1].descriptorCount = imageInfo.size();
      descriptorWrites[1].pImageInfo = imageInfo.data();

      descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[2].dstSet = descriptorSet;
      descriptorWrites[2].dstBinding = 2;
      descriptorWrites[2].dstArrayElement = 0;
      descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      descriptorWrites[2].descriptorCount = storageBufferInfos.size();
      descriptorWrites[2].pBufferInfo = storageBufferInfos.data();

      vkUpdateDescriptorSets(m_Device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    void VulkanAPI::updateStorageDescriptorSets(std::vector<Buffer> & uniformBuffers, VkDescriptorSet & descriptorSet,
        VkDescriptorType type)
    {
        std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
        std::vector<VkDescriptorBufferInfo> bufferInfos;

        std::for_each(std::begin(uniformBuffers), std::end(uniformBuffers),
        [& bufferInfos](const Buffer & uniformBuffer)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffer.buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = VK_WHOLE_SIZE;
            bufferInfos.emplace_back(bufferInfo);
        });

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSet;
        descriptorWrites[0].dstBinding = 2;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = type;
        descriptorWrites[0].descriptorCount = bufferInfos.size();
        descriptorWrites[0].pBufferInfo = bufferInfos.data();

        vkUpdateDescriptorSets(m_Device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    void VulkanAPI::beginRenderPass(VkRenderPass renderPass, VkCommandBuffer& commandBuffer, VkFramebuffer framebuffer)
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = framebuffer;
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_SwapChainExtent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        //vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    }

    void VulkanAPI::endRenderPass(VkCommandBuffer& commandBuffer)
    {
        vkCmdEndRenderPass(commandBuffer);
    }

    void VulkanAPI::beginRendering(VkCommandBuffer& commandBuffer,
        VkImageView & colorImageView,
        VkImageView & depthImageView,
        VkAttachmentLoadOp const loadOp,
        VkAttachmentStoreOp const storeOp)
    {
        VkClearColorValue colorClear = {};
        colorClear.float32[0] = 1;
        colorClear.float32[1] = 1;
        colorClear.float32[2] = 1;
        colorClear.float32[3] = 1;

        VkClearDepthStencilValue depthStencil = { 1.f, 0 };

        VkRenderingAttachmentInfo colorAttachment{ };
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView = colorImageView;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = loadOp;
        colorAttachment.storeOp = storeOp;
        colorAttachment.clearValue.color = colorClear;
        
        VkRenderingAttachmentInfo depthAttachment{ };
        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachment.imageView = depthImageView;
        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depthAttachment.loadOp = loadOp;
        depthAttachment.storeOp = storeOp;
        depthAttachment.clearValue.depthStencil = depthStencil;

        VkRenderingInfo renderingInfo{ };
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea.extent = m_SwapChainExtent;
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
        renderingInfo.pDepthAttachment = &depthAttachment;
        //renderingInfo.pStencilAttachment;

        vkCmdBeginRenderingKHR(commandBuffer, & renderingInfo);
    }

    void VulkanAPI::endRendering(VkCommandBuffer& commandBuffer)
    {
        vkCmdEndRenderingKHR(commandBuffer);
    }

    void VulkanAPI::endCommandBuffer(VkCommandBuffer& commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);
    }

    VkResult VulkanAPI::queueSubmit(VkCommandBuffer& commandBuffer, int queueIndex)
    {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        VkResult result = VK_SUCCESS;
        {
            std::lock_guard<std::mutex> guard(m_MutexQueueSubmit);
            vkResetFences(m_Device, 1, &m_FenceSubmit);

            result = vkQueueSubmit(m_GraphicsQueues[queueIndex], 1, &submitInfo, m_FenceSubmit);
            vkWaitForFences(m_Device, 1, & m_FenceSubmit, VK_TRUE, UINT32_MAX);
            vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
        }
        return result;
    }

    void VulkanAPI::resetCommandPool(VkCommandPool& commandPool)
    {
        vkResetCommandPool(m_Device, commandPool, 0);
    }

    void VulkanAPI::draw(
      VkCommandBuffer& commandBuffer,
      VkDescriptorSet& descriptorSet,
      VulkanPipeline & pipeline,
      Data * data,
      bool drawIndexed,
      uint32_t index)
    {
        /*vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout,
            0, 1, & descriptorSet, 0, nullptr);*/

        //vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);

        VkBuffer vertexBuffers[] = { data->m_VertexBuffer.buffer };
        VkDeviceSize offsets[] = { 0 };

        if (data->m_VertexBuffer.size > 0) {
          vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        }
        
        if (drawIndexed) {
            vkCmdBindIndexBuffer(commandBuffer, data->m_IndicesBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(commandBuffer, data->m_Indices.size(), data->m_Ubos.size(), 0, 0, 0);
        } else {
            vkCmdDraw(commandBuffer, data->m_Vertices.size(), 1, 0, index);
        }
    }

    void VulkanAPI::addPipelineBarriers(VkCommandBuffer& commandBuffer, std::vector<VkImageMemoryBarrier> renderBarriers,
        VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags)
    {
        vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, 0, nullptr, 0, nullptr,
            static_cast<uint32_t>(renderBarriers.size()), renderBarriers.data());
    }

    VkBuffer VulkanAPI::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage)
    {
        VkBuffer buffer{};
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(m_Device, & bufferInfo, nullptr, & buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }
        return buffer;
    }

    void VulkanAPI::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
        VkBuffer & buffer, VkDeviceMemory & bufferMemory)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(m_Device, & bufferInfo, nullptr, & buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device, buffer, & memRequirements);

        uint32_t allocatedSize = ((memRequirements.size / memRequirements.alignment) + 1) * memRequirements.alignment;

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = allocatedSize;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_Device, &allocInfo, nullptr, & bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }
        VkResult result = vkBindBufferMemory(m_Device, buffer, bufferMemory, 0);

        if (result != VK_SUCCESS) {
            PLP_ERROR("Memory binding failed in createBuffer");
        }
    }

    Buffer VulkanAPI::createIndexBuffer(VkCommandPool const & commandPool, std::vector<uint32_t> const & indices)
    {
        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
      
        void* data;
        vkMapMemory(m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(m_Device, stagingBufferMemory);

        VkBuffer buffer = createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device, buffer, & memRequirements);
        uint32_t size = ((memRequirements.size / memRequirements.alignment) + 1) * memRequirements.alignment;

        auto memoryType = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        auto deviceMemory = m_DeviceMemoryPool->get(
            m_Device,
            size,
            memoryType,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            memRequirements.alignment,
            DeviceMemoryPool::DeviceBufferType::UNIFORM);

        auto offset = deviceMemory->getOffset();
        deviceMemory->bindBufferToMemory(buffer, size);

        copyBuffer(commandPool, stagingBuffer, buffer, bufferSize);

        Buffer meshBuffer;
        meshBuffer.buffer = std::move(buffer);
        meshBuffer.memory = deviceMemory;
        meshBuffer.offset = offset;
        meshBuffer.size = size;

        vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
        vkFreeMemory(m_Device, stagingBufferMemory, nullptr);

        return meshBuffer;
    }

    Buffer VulkanAPI::createVertexBuffer(VkCommandPool commandPool, std::vector<Vertex> vertices)
    {
        VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();
        VkBuffer stagingBuffer{};
        VkDeviceMemory stagingBufferMemory{};

        createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(m_Device, stagingBufferMemory);

        VkBuffer buffer = createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device, buffer, & memRequirements);

        auto memoryType = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        uint32_t size = ((memRequirements.size / memRequirements.alignment) + 1) * memRequirements.alignment;

        auto deviceMemory = m_DeviceMemoryPool->get(
            m_Device,
            size,
            memoryType,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            memRequirements.alignment,
            DeviceMemoryPool::DeviceBufferType::UNIFORM);

        auto offset = deviceMemory->getOffset();
        deviceMemory->bindBufferToMemory(buffer, size);
        
        copyBuffer(commandPool, stagingBuffer, buffer, bufferSize);

        Buffer meshBuffer;
        meshBuffer.buffer = std::move(buffer);
        meshBuffer.memory = deviceMemory;
        meshBuffer.offset = offset;
        meshBuffer.size = size;
        
        vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
        vkFreeMemory(m_Device, stagingBufferMemory, nullptr);

        return meshBuffer;
    }

    Buffer VulkanAPI::createVertex2DBuffer(VkCommandPool const & commandPool,
        std::vector<Vertex2D> const & vertices)
    {
        //std::pair<VkBuffer, VkDeviceMemory> vertexBuffer{};
        VkDeviceSize bufferSize = sizeof(Vertex2D) * vertices.size();
        VkBuffer stagingBuffer{};
        VkDeviceMemory stagingBufferMemory{};

        createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(m_Device, stagingBufferMemory);

        VkBuffer buffer = createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

        auto memoryType = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        uint32_t size = ((memRequirements.size / memRequirements.alignment) + 1) * memRequirements.alignment;
        
        auto deviceMemory = m_DeviceMemoryPool->get(
            m_Device,
            size,
            memoryType,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            memRequirements.alignment,
            DeviceMemoryPool::DeviceBufferType::UNIFORM);

        auto offset = deviceMemory->getOffset();
        deviceMemory->bindBufferToMemory(buffer, size);
        
        copyBuffer(commandPool, stagingBuffer, buffer, bufferSize);

        Buffer meshBuffer;
        meshBuffer.buffer = std::move(buffer);
        meshBuffer.memory = deviceMemory;
        meshBuffer.offset = offset;
        meshBuffer.size = size;

        vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
        vkFreeMemory(m_Device, stagingBufferMemory, nullptr);

        return meshBuffer;
    }

    Buffer VulkanAPI::createUniformBuffers(uint32_t uniformBuffersCount)
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject) * uniformBuffersCount;
        VkBuffer buffer = createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

        auto memoryType = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        uint32_t size = ((memRequirements.size / memRequirements.alignment) + 1) * memRequirements.alignment;
        
        auto deviceMemory = m_DeviceMemoryPool->get(
            m_Device,
            size,
            memoryType,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            memRequirements.alignment,
            DeviceMemoryPool::DeviceBufferType::UNIFORM);

        auto offset = deviceMemory->getOffset();
        deviceMemory->bindBufferToMemory(buffer, size);

        Buffer uniformBuffer;
        uniformBuffer.buffer = std::move(buffer);
        uniformBuffer.memory = deviceMemory;
        uniformBuffer.offset = offset;
        uniformBuffer.size = size;

        return uniformBuffer;
    }

    Buffer VulkanAPI::createStorageBuffers(size_t storageSize)
    {
        VkDeviceSize bufferSize = storageSize;
        VkBuffer buffer = createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device, buffer, & memRequirements);

        auto const flags { VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT};

        auto memoryType = findMemoryType(memRequirements.memoryTypeBits, flags);

        uint32_t size = ((memRequirements.size / memRequirements.alignment) + 1) * memRequirements.alignment;

        auto deviceMemory = m_DeviceMemoryPool->get(
          m_Device,
          size,
          memoryType,
          flags,
          memRequirements.alignment,
          DeviceMemoryPool::DeviceBufferType::STORAGE);

        auto offset = deviceMemory->getOffset();
        deviceMemory->bindBufferToMemory(buffer, size);

        Buffer uniformBuffer;
        uniformBuffer.buffer = std::move(buffer);
        uniformBuffer.memory = deviceMemory;
        uniformBuffer.offset = offset;
        uniformBuffer.size = size;

        return uniformBuffer;
    }

    Buffer VulkanAPI::createIndirectCommandsBuffer(std::vector<VkDrawIndexedIndirectCommand> const& drawCommands)
    {
      VkBuffer buffer{ createBuffer(
        sizeof(VkDrawIndexedIndirectCommand) * drawCommands.size(),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT) };
        
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device, buffer, & memRequirements);

        auto const memoryType{ findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) };
        auto const size{ ((memRequirements.size / memRequirements.alignment) + 1) * memRequirements.alignment };

        auto deviceMemory{ m_DeviceMemoryPool->get(
          m_Device,
          size,
          memoryType,
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
          memRequirements.alignment,
          DeviceMemoryPool::DeviceBufferType::STORAGE) };

        auto const offset {deviceMemory->getOffset()};
        deviceMemory->bindBufferToMemory(buffer, size);

        Buffer indirectBuffer{ std::move(buffer), deviceMemory, offset, size };

        {
          indirectBuffer.memory->lock();

          auto memory{ indirectBuffer.memory->getMemory() };
          void* data;
          vkMapMemory(m_Device, *memory, indirectBuffer.offset, indirectBuffer.size, 0, &data);
          memcpy(data, drawCommands.data(), indirectBuffer.size);
          vkUnmapMemory(m_Device, *memory);

          indirectBuffer.memory->unLock();
        }
        return indirectBuffer;
    }

    Buffer VulkanAPI::createCubeUniformBuffers(uint32_t uniformBuffersCount)
    {
        VkDeviceSize bufferSize = sizeof(CubeUniformBufferObject) * uniformBuffersCount;
        VkBuffer buffer = createBuffer(bufferSize, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

        uint32_t size = ((memRequirements.size / memRequirements.alignment) + 1) * memRequirements.alignment;
        

        auto memoryType = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        auto deviceMemory = m_DeviceMemoryPool->get(
            m_Device,
            size,
            memoryType,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            memRequirements.alignment,
            DeviceMemoryPool::DeviceBufferType::UNIFORM);

        auto offset = deviceMemory->getOffset();
        deviceMemory->bindBufferToMemory(buffer, size);

        Buffer uniformBuffer;
        uniformBuffer.buffer = std::move(buffer);
        uniformBuffer.memory = deviceMemory;
        uniformBuffer.offset = offset;
        uniformBuffer.size = size;

        return uniformBuffer;
    }

    void VulkanAPI::updateUniformBuffer(Buffer & buffer, std::vector<UniformBufferObject>* uniformBufferObjects)
    {
        {
            buffer.memory->lock();

            auto memory = buffer.memory->getMemory();
            void* data;
            vkMapMemory(m_Device, *memory, buffer.offset, buffer.size, 0, & data);
            memcpy(data, uniformBufferObjects->data(), buffer.size);
            vkUnmapMemory(m_Device, *memory);

            buffer.memory->unLock();
        }
    }

    void VulkanAPI::updateStorageBuffer(Buffer & buffer, ObjectBuffer objectBuffer)
    {
        {
            buffer.memory->lock();

            auto memory = buffer.memory->getMemory();
            void* data;
            vkMapMemory(m_Device, *memory, buffer.offset, buffer.size, 0, &data);
            memcpy(data, & objectBuffer, buffer.size);
            vkUnmapMemory(m_Device, *memory);

            buffer.memory->unLock();
        }
    }

    void VulkanAPI::copyBuffer(VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size,
        int queueIndex)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer{};
        vkAllocateCommandBuffers(m_Device, & allocInfo, & commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, & beginInfo);

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = size;

        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, & copyRegion);
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = & commandBuffer;

        {
            std::lock_guard<std::mutex> guard(m_MutexQueueSubmit);
            vkResetFences(m_Device, 1, & m_FenceBuffer);

            vkQueueSubmit(m_GraphicsQueues[queueIndex], 1, & submitInfo, m_FenceBuffer);
            vkWaitForFences(m_Device, 1, & m_FenceBuffer, VK_TRUE, UINT32_MAX);
            vkFreeCommandBuffers(m_Device, commandPool, 1, & commandBuffer);
        }
    }

    VkImageMemoryBarrier VulkanAPI::setupImageMemoryBarrier(VkImage image, VkAccessFlags srcAccessMask,
        VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels,
        VkImageAspectFlags aspectMask)
    {
        VkImageMemoryBarrier result = { };
        result.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        result.srcAccessMask = srcAccessMask;
        result.dstAccessMask = dstAccessMask;
        result.oldLayout = oldLayout;
        result.newLayout = newLayout;
        result.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        result.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        result.image = image;
        result.subresourceRange.aspectMask = aspectMask;
        //those constants don't work on android
        result.subresourceRange.levelCount = mipLevels;
        result.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

        result.pNext = nullptr;

        return result;
    }

    uint32_t VulkanAPI::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

        for (uint32_t i { 0 }; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                m_MaxMemoryHeap = memProperties.memoryHeaps[memProperties.memoryTypes[i].heapIndex].size;
                return i;
            }
        }
        throw std::runtime_error("failed to find suitable memory type!");
    }

    void VulkanAPI::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
        VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = numSamples;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult result{ VK_SUCCESS };
        result = vkCreateImage(m_Device, & imageInfo, nullptr, &image);

        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_Device, image, & memRequirements);
        auto memoryType = findMemoryType(memRequirements.memoryTypeBits, properties);
        uint32_t size = ((memRequirements.size / memRequirements.alignment) + 1) * memRequirements.alignment;
        

        auto deviceMemory = m_DeviceMemoryPool->get(
            m_Device,
            size,
            memoryType, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            memRequirements.alignment,
            DeviceMemoryPool::DeviceBufferType::STAGING);

        deviceMemory->bindImageToMemory(image, size);
    }

    void VulkanAPI::createDepthMapImage(VkImage & image)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = 2048;
        imageInfo.extent.height = 2048;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_D32_SFLOAT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT 
            | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        
        VkResult result = vkCreateImage(getDevice(), & imageInfo, nullptr, & image);

         if (result != VK_SUCCESS) {
            throw std::runtime_error("can't create image for shadow map");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_Device, image, & memRequirements);
        auto memoryType = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        uint32_t size = ((memRequirements.size / memRequirements.alignment) + 1) * memRequirements.alignment;

        auto deviceMemory = m_DeviceMemoryPool->get(
            m_Device,
            size,
            memoryType,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            memRequirements.alignment,
            DeviceMemoryPool::DeviceBufferType::STAGING);

        deviceMemory->bindImageToMemory(image, size);
    }

    VkImageView VulkanAPI::createDepthMapImageView(VkImage image)
    {
        VkImageView depthMapImageView{};

        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = VK_FORMAT_D32_SFLOAT;
        createInfo.subresourceRange = {};
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VkResult result{ VK_SUCCESS };

        result = vkCreateImageView(m_Device, & createInfo, nullptr, & depthMapImageView);

        if (result != VK_SUCCESS) {
            PLP_ERROR("failed to create depth map image view.");
        }
        return depthMapImageView;
    }

    VkSampler VulkanAPI::createDepthMapSampler()
    {
        VkSampler sampler{};

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeV = samplerInfo.addressModeU;
        samplerInfo.addressModeW = samplerInfo.addressModeU;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        if (vkCreateSampler(m_Device, & samplerInfo, nullptr, & sampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create depth map sampler!");
        }
        return sampler;
    }

    void VulkanAPI::createDepthMapFrameBuffer(VkRenderPass & renderPass, VkImageView & imageView,
        VkFramebuffer & frameBuffer)
    {
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &imageView;
        framebufferInfo.width = m_SwapChainExtent.width;
        framebufferInfo.height = m_SwapChainExtent.height;
        framebufferInfo.layers = 1;

        VkResult result = vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, & frameBuffer);

        if (result != VK_SUCCESS) {
            PLP_ERROR("failed to create framebuffer for depth map");
        }
    }

    void VulkanAPI::createSkyboxImage(uint32_t width, uint32_t height, VkSampleCountFlagBits numSamples,
        VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage & image)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 6;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = numSamples;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

        if (vkCreateImage(m_Device, &imageInfo, nullptr, & image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_Device, image, & memRequirements);
        auto memoryType = findMemoryType(memRequirements.memoryTypeBits, properties);
        uint32_t size = ((memRequirements.size / memRequirements.alignment) + 1) * memRequirements.alignment;
        

        auto deviceMemory = m_DeviceMemoryPool->get(
            m_Device,
            size,
            memoryType,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            memRequirements.alignment,
            DeviceMemoryPool::DeviceBufferType::STAGING);

        deviceMemory->bindImageToMemory(image, size);
    }

    void VulkanAPI::createTextureImage(VkCommandBuffer & commandBuffer, stbi_uc* pixels, uint32_t texWidth,
        uint32_t texHeight, uint32_t mipLevels, VkImage & textureImage, VkFormat format)
    {
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        VkBuffer buffer = createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device, buffer, & memRequirements);

        auto memoryType = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        uint32_t size = ((memRequirements.size / memRequirements.alignment) + 1) * memRequirements.alignment;

        auto deviceMemory = m_DeviceMemoryPool->get(
            m_Device,
            size,
            memoryType, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            memRequirements.alignment,
            DeviceMemoryPool::DeviceBufferType::STAGING);

        auto offset = deviceMemory->getOffset();
        deviceMemory->bindBufferToMemory(buffer, size);

        void* data;
        vkMapMemory(m_Device, *deviceMemory->getMemory(), offset, size, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(size));
        vkUnmapMemory(m_Device, *deviceMemory->getMemory());

        stbi_image_free(pixels);

        createImage(texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage);

        VkImageMemoryBarrier renderBarrier = setupImageMemoryBarrier(
            textureImage, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        addPipelineBarriers(commandBuffer, { renderBarrier }, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_DEPENDENCY_BY_REGION_BIT);

        copyBufferToImage(commandBuffer, buffer, textureImage, static_cast<uint32_t>(texWidth),
            static_cast<uint32_t>(texHeight), mipLevels);

        generateMipmaps(commandBuffer, format, textureImage, texWidth, texHeight, mipLevels);
        endCommandBuffer(commandBuffer);
        queueSubmit(commandBuffer);
        //m_DeviceMemoryPool->clear(deviceMemory);
    }

    void VulkanAPI::createSkyboxTextureImage(VkCommandBuffer & commandBuffer, std::vector<stbi_uc*> & skyboxPixels,
        uint32_t texWidth, uint32_t texHeight, uint32_t mipLevels, VkImage & textureImage, VkFormat format)
    {
        VkDeviceSize imageSize = texWidth * texHeight * 4 * 6;
        VkBuffer buffer = createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device, buffer, & memRequirements);

        auto memoryType = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        uint32_t size = ((memRequirements.size / memRequirements.alignment) + 1) * memRequirements.alignment;
        
        
        auto deviceMemory = m_DeviceMemoryPool->get(
            m_Device,
            size,
            memoryType, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            memRequirements.alignment,
            DeviceMemoryPool::DeviceBufferType::STAGING,
            true);

        auto offset = deviceMemory->getOffset();
        deviceMemory->bindBufferToMemory(buffer, size);

        stbi_uc* data;
        VkDeviceSize layerSize = imageSize / 6;
        vkMapMemory(m_Device, *deviceMemory->getMemory(), offset, size, 0, (void**)&data);

        for (uint32_t i { 0 }; i < skyboxPixels.size(); i++) {
            memcpy(data + layerSize * i, skyboxPixels[i], layerSize);
        }

        vkUnmapMemory(m_Device, *deviceMemory->getMemory());
        createSkyboxImage(texWidth, texHeight, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage);

        VkImageMemoryBarrier renderBarrier = setupImageMemoryBarrier(textureImage, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        renderBarrier.subresourceRange.layerCount = 6;
        renderBarrier.subresourceRange.baseMipLevel = 0;
        renderBarrier.subresourceRange.levelCount = mipLevels;

        addPipelineBarriers(commandBuffer, { renderBarrier }, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT);

        copyBufferToImageSkybox(commandBuffer, buffer, textureImage, texWidth, texHeight, skyboxPixels, mipLevels,
            imageSize / 6);

        generateMipmaps(commandBuffer, format, textureImage, texWidth, texHeight, mipLevels, 6);
        endCommandBuffer(commandBuffer);
        queueSubmit(commandBuffer);

        for (uint32_t i { 0 }; i < skyboxPixels.size(); i++) {
            stbi_image_free(skyboxPixels[i]);
        }
        //m_DeviceMemoryPool->clear(deviceMemory);
    }

    void VulkanAPI::generateMipmaps(VkCommandBuffer& commandBuffer, VkFormat imageFormat, VkImage image, uint32_t texWidth,
        uint32_t texHeight, uint32_t mipLevels, uint32_t layerCount) {

        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, imageFormat, & formatProperties);

        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
            throw std::runtime_error("texture image format does not support linear blitting!");
        }

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = image;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
        barrier.subresourceRange.levelCount = 1;

        auto mipWidth = static_cast<int32_t>(texWidth);
        auto mipHeight = static_cast<int32_t>(texHeight);

        for (uint32_t i = 1; i < mipLevels; i++) {

            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr, 0, nullptr, 1, & barrier);

            VkImageBlit blit{};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = layerCount;
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = layerCount;

            vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, & blit, VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr, 0, nullptr, 1, & barrier);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr, 0, nullptr, 1, & barrier);
    }

    void VulkanAPI::copyBufferToImage(VkCommandBuffer & commandBuffer, VkBuffer& buffer, VkImage & image,
        uint32_t width, uint32_t height,  uint32_t mipLevels)
    {
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { width, height, 1 };

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, & region);
    }

    void VulkanAPI::copyBufferToImageSkybox(VkCommandBuffer & commandBuffer, VkBuffer & buffer, VkImage & image,
        uint32_t width, uint32_t height, std::vector<stbi_uc*>skyboxPixels, uint32_t mipLevels, uint32_t layerSize)
    {
        std::vector<VkBufferImageCopy> bufferCopyRegions;

        for (uint32_t i { 0 }; i < skyboxPixels.size(); i++) {

            for (uint32_t mipLevel = 0; mipLevel < mipLevels; mipLevel++) {

                VkBufferImageCopy region{};
                region.bufferOffset = layerSize * i;
                region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                region.imageSubresource.mipLevel = mipLevel;
                region.imageSubresource.baseArrayLayer = i;
                region.imageSubresource.layerCount = 1;
                region.imageExtent.width =  width >> mipLevel;
                region.imageExtent.height = height >> mipLevel;
                region.imageExtent.depth = 1;

                bufferCopyRegions.emplace_back(region);
            }
        }
        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            static_cast<uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data());
    }

    VkImageView VulkanAPI::createDepthResources(VkCommandBuffer& commandBuffer)
    {
        VkImage depthImage{};
        //VkDeviceMemory depthImageMemory;
        VkImageView depthImageView{};
        VkFormat depthFormat = findDepthFormat();

        createImage(m_SwapChainExtent.width, m_SwapChainExtent.height, 1, VK_SAMPLE_COUNT_1_BIT, depthFormat,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            depthImage);

        depthImageView = createImageView(depthImage, depthFormat, 1);

        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (hasStencilComponent(depthFormat)) {
            aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        VkImageMemoryBarrier renderBarrier = setupImageMemoryBarrier(depthImage, 0,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, aspectMask);

        addPipelineBarriers(commandBuffer, { renderBarrier }, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_DEPENDENCY_BY_REGION_BIT);

        return depthImageView;
    }

    VkSampler VulkanAPI::createTextureSampler(uint32_t mipLevels)
    {
        VkSampler textureSampler{};

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = samplerInfo.addressModeU;
        samplerInfo.addressModeW = samplerInfo.addressModeU;
        samplerInfo.maxAnisotropy = m_DeviceProps.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(mipLevels);
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;

        if (vkCreateSampler(m_Device, & samplerInfo, nullptr, & textureSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
        return textureSampler;
    }

    VkSampler VulkanAPI::createSkyboxTextureSampler(uint32_t mipLevels)
    {
        VkSampler textureSampler{};

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_NEAREST;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeV = samplerInfo.addressModeU;
        samplerInfo.addressModeW = samplerInfo.addressModeU;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(mipLevels);
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        if (vkCreateSampler(m_Device, &samplerInfo, nullptr, & textureSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
        return textureSampler;
    }

    VkFormat VulkanAPI::findDepthFormat()
    {
        return findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    VkFormat VulkanAPI::findSupportedFormat(std::vector<VkFormat> const & candidates, VkImageTiling tiling,
        VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, & props);

            if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
        }
        throw std::runtime_error("failed to find supported format!");
    }

    bool VulkanAPI::hasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    VkSampleCountFlagBits VulkanAPI::getMaxUsableSampleCount()
    {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(m_PhysicalDevice, & physicalDeviceProperties);

        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts 
            & physicalDeviceProperties.limits.framebufferDepthSampleCounts;

        if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
        if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
        if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
        if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
        if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
        if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
        return VK_SAMPLE_COUNT_1_BIT;
    }

    //void VulkanAPI::destroyPipeline(VkPipeline pipeline)
    //{
    //    vkDestroyPipeline(m_Device, pipeline, nullptr);
    //}

    //void VulkanAPI::destroyPipelineData(VkPipelineLayout pipelineLayout, VkDescriptorPool descriptorPool,
    //    VkDescriptorSetLayout descriptorSetLayout)
    //{
    //    vkDestroyDescriptorSetLayout(m_Device, descriptorSetLayout, nullptr);
    //    vkDestroyPipelineLayout(m_Device, pipelineLayout, nullptr);
    //    vkDestroyDescriptorPool(m_Device, descriptorPool, nullptr);
    //}

    //void VulkanAPI::destroySemaphores(std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> semaphores)
    //{
    //    for (int i { 0 }; i < m_MAX_FRAMES_IN_FLIGHT; i++) {
    //        
    //        if (nullptr != semaphores.first[i]) vkDestroySemaphore(m_Device, semaphores.first[i], nullptr);
    //        if (nullptr != semaphores.second[i]) vkDestroySemaphore(m_Device, semaphores.second[i], nullptr);
    //    }
    //}

    //void VulkanAPI::destroyFences()
    //{
    //    for (size_t i { 0 }; i < m_InFlightFences.size(); ++i) {
    //        vkDestroyFence(m_Device, m_InFlightFences[i], nullptr);
    //    }
    //    for (size_t i { 0 }; i < m_ImagesInFlight.size(); ++i) {
    //        vkDestroyFence(m_Device, m_ImagesInFlight[i], nullptr);
    //    }
    //}

    //void VulkanAPI::destroyBuffer(VkBuffer buffer)
    //{
    //    if (VK_NULL_HANDLE == buffer) return;

    //    vkDestroyBuffer(m_Device, buffer, nullptr);
    //}

    //void VulkanAPI::destroyRenderPass(VkRenderPass* renderPass, VkCommandPool commandPool,
    //    std::vector<VkCommandBuffer> commandBuffers)
    //{
    //    vkFreeCommandBuffers(m_Device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
    //    vkDestroyRenderPass(m_Device, *renderPass, nullptr);
    //    vkDestroyCommandPool(m_Device, commandPool, nullptr);
    //}

    //void VulkanAPI::destroy()
    //{
    //    vkDestroyFence(m_Device, m_Fence, nullptr);
    //    vkDestroyDevice(m_Device, nullptr);

    //    if (m_EnableValidationLayers) {
    //        DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessengerCallback, nullptr);
    //    }

    //    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    //    vkDestroyInstance(m_Instance, nullptr);

    //    PLP_TRACE("VK instance destroyed and cleaned");
    //}

    //void VulkanAPI::destroySwapchain(VkDevice device, VkSwapchainKHR swapChain,
    //    std::vector<VkFramebuffer> swapChainFramebuffers, std::vector<VkImageView> swapChainImageViews)
    //{
    //    for (uint32_t i { 0 }; i < swapChainFramebuffers.size(); ++i) {
    //        vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
    //    }
    //    for (uint32_t i { 0 }; i < swapChainImageViews.size(); ++i) {
    //        vkDestroyImageView(device, swapChainImageViews[i], nullptr);
    //    }
    //    vkDestroySwapchainKHR(device, swapChain, nullptr);
    //}

    VulkanAPI::~VulkanAPI()
    {
        PLP_TRACE("VulkanAPI deleted.");
    }

    void VulkanAPI::startMarker(VkCommandBuffer buffer, const std::string& name, float r, float g, float b, float a)
    {
#ifdef PLP_DEBUG_BUILD
        VkDebugUtilsLabelEXT label;
        label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        label.pLabelName = name.c_str();
        label.pNext = VK_NULL_HANDLE;
        label.color[0] = a;
        label.color[1] = r;
        label.color[2] = g;
        label.color[3] = b;

        vkCmdBeginDebugUtilsLabelEXT(buffer, &label);
#endif
    }

    void VulkanAPI::endMarker(VkCommandBuffer buffer)
    {
#ifdef PLP_DEBUG_BUILD
        vkCmdEndDebugUtilsLabelEXT(buffer);
#endif
    }

    void VulkanAPI::waitIdle()
    {
        vkQueueWaitIdle(m_PresentQueues[0]);
        vkDeviceWaitIdle(m_Device);
    }

    void VulkanAPI::setResolution(unsigned int width, unsigned int height)
    {
        m_Width = width;
        m_Height = height;
    }

    void VulkanAPI::transitionImageLayout(
      VkCommandBuffer& commandBuffer,
      VkImage image,
      VkImageLayout oldLayout,
      VkImageLayout newLayout,
      VkImageAspectFlags aspectFlags)
    {
      VkImageMemoryBarrier barrier{};
      barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      barrier.oldLayout = oldLayout;
      barrier.newLayout = newLayout;
      barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      barrier.image = image;
      barrier.subresourceRange.aspectMask = aspectFlags;
      barrier.subresourceRange.baseMipLevel = 0;
      barrier.subresourceRange.levelCount = 1;
      barrier.subresourceRange.baseArrayLayer = 0;
      barrier.subresourceRange.layerCount = 1;

      VkPipelineStageFlags sourceStage;
      VkPipelineStageFlags destinationStage;

      if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      } else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = 0;

        sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
      } else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = 0;

        sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
      } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = 0;

        sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
      } else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = 0;

        sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
      } else {
        throw std::invalid_argument("unsupported layout transition");
      }

      vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
      );
    }

    void VulkanAPI::submit(
      VkQueue& queue,
      VkSubmitInfo& submitInfo,
      VkPresentInfoKHR& presentInfo,
      VkFence& fence)
    {
      {
        std::lock_guard<std::mutex> guard(m_MutexQueueSubmit);
        VkResult submitResult = vkQueueSubmit(queue, 1, &submitInfo, fence);

        if (submitResult != VK_SUCCESS) {
          PLP_ERROR("Error on queue submit: {}", submitResult);
          throw std::runtime_error("Error on queueSubmit");
        }

        VkResult presentResult = vkQueuePresentKHR(queue, &presentInfo);

        if (presentResult != VK_SUCCESS) {
          PLP_ERROR("Error on queue present: {}", presentResult);
        }

        vkQueueWaitIdle(queue);
      }
    }
}
