#include "rebulkpch.h"
#include "VulkanRenderer.h"

#define VOLK_IMPLEMENTATION
#include <volk.h>

namespace Rbk {

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pCallback);
        }
        else {

            std::cerr << "Debug utils extension not present";

            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {

        spdlog::set_pattern("%^[%T] %n: %v%$");

        switch (messageSeverity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        {
            Rbk::Log::GetLogger()->critical("{} : {}", messageType, pCallbackData->pMessage);
            break;
        }
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        {
            Rbk::Log::GetLogger()->warn("{} : {}", messageType, pCallbackData->pMessage);
            break;
        }
        default:
        {
            //Rbk::Log::GetLogger()->trace("{} : {}", messageType, pCallbackData->pMessage);
        }
        }

        return VK_FALSE;
    }

    VulkanRenderer::VulkanRenderer(std::shared_ptr<Window> window) : m_Window(window)
    {
#ifdef RBK_DEBUG
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

        LoadRequiredExtensions();
        EnumerateExtensions();
        CreateInstance();
        SetupDebugMessenger();
        CreateSurface();
        PickPhysicalDevice();
        CreateLogicalDevice();
        InitDetails();
        CreateFence();

        m_DeviceMemoryPool = std::make_shared<DeviceMemoryPool>(
            m_DeviceProps.limits.maxMemoryAllocationCount,
            m_DeviceMaintenance3Properties.maxMemoryAllocationSize
        );
    }

    std::string VulkanRenderer::GetAPIVersion()
    {
        if (m_apiVersion.empty()) {
            uint32_t instanceVersion = VK_API_VERSION_1_3;
            auto FN_vkEnumerateInstanceVersion = PFN_vkEnumerateInstanceVersion(vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));

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

    void VulkanRenderer::CreateInstance()
    {
        std::string message;

        if (!IsValidationLayersEnabled() && !CheckValidationLayerSupport()) {
            Rbk::Log::GetLogger()->warn("Validations layers not available !");
        }

        VkApplicationInfo appInfo{};

        appInfo.apiVersion = VK_API_VERSION_1_3;
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Rebulkan";
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
            debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugCreateInfo.pfnUserCallback = DebugCallback;
            debugCreateInfo.pUserData = nullptr;

            createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
            createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
            Rbk::Log::GetLogger()->trace("Validations enabled");
        } else {
            Rbk::Log::GetLogger()->trace("Validations disabled");
            createInfo.enabledLayerCount = 0;
        }

        VkResult result = VK_SUCCESS;

        result = vkCreateInstance(&createInfo, nullptr, &m_Instance);

        if (VK_SUCCESS != result) {
            Rbk::Log::GetLogger()->critical("Can't create VK instance : {}", result);
            throw std::runtime_error("Can't create VK instance.");
        }

        m_InstanceCreated = true;

        volkLoadInstance(m_Instance);
    }

    void VulkanRenderer::InitDetails()
    {
        m_SwapChainSupport = QuerySwapChainSupport(m_PhysicalDevice);
        m_SurfaceFormat = ChooseSwapSurfaceFormat(m_SwapChainSupport.formats);
        m_PresentMode = ChooseSwapPresentMode(m_SwapChainSupport.presentModes);
        VkExtent2D extent = ChooseSwapExtent(m_SwapChainSupport.capabilities);
        m_SwapChainImageFormat = m_SurfaceFormat.format;
        m_SwapChainExtent = extent;
    }

    void VulkanRenderer::EnumerateExtensions()
    {
        vkEnumerateInstanceExtensionProperties(nullptr, &m_ExtensionCount, nullptr);

        std::vector<VkExtensionProperties> extensions(m_ExtensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &m_ExtensionCount, extensions.data());

        m_Extensions = extensions;
    }

    bool VulkanRenderer::CheckValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const std::string layerName : m_ValidationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
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

    void VulkanRenderer::LoadRequiredExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const  char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        
        if (m_EnableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            //extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME); deprecated
            //extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME); deprecated
        }

        //for_each(extensions.begin(), extensions.end(), [](std::string text) {
        //    Rbk::Log::GetLogger()->trace("required extension needed : {}", text);
        //});

        m_RequiredExtensions = extensions;
    }

    void VulkanRenderer::SetupDebugMessenger()
    {
        if (!m_EnableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;
        createInfo.pUserData = nullptr;

        if (CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessengerCallback) != VK_SUCCESS) {
            Rbk::Log::GetLogger()->warn("Can't create debug messenger.");
        }
    }

    void VulkanRenderer::PickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            Rbk::Log::GetLogger()->critical("failed to find GPUs with Vulkan support!");
            exit(-1);
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

        for (const auto& device : devices) {
            if (IsDeviceSuitable(device)) {
                m_PhysicalDevice = device;

                VkPhysicalDeviceProperties deviceProperties;
                VkPhysicalDeviceFeatures deviceFeatures;
                VkPhysicalDeviceMaintenance3Properties deviceMaintenance3Properties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES };
                VkPhysicalDeviceProperties2 deviceProperties2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
                deviceProperties2.pNext = &deviceMaintenance3Properties;

                vkGetPhysicalDeviceProperties(device, &deviceProperties);
                vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
                vkGetPhysicalDeviceProperties2(device, &deviceProperties2);

                m_DeviceProps = deviceProperties;
                m_DeviceFeatures = deviceFeatures;
                m_DeviceProperties2 = deviceProperties2;
                m_DeviceMaintenance3Properties = deviceMaintenance3Properties;

                m_MsaaSamples = GetMaxUsableSampleCount();

                break;
            }
        }

        if (m_PhysicalDevice == VK_NULL_HANDLE) {
            Rbk::Log::GetLogger()->critical("failed to find a suitable GPU");
            exit(-1);
        }
    }

    bool VulkanRenderer::IsDeviceSuitable(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices = FindQueueFamilies(device);
        bool extensionsSupported = CheckDeviceExtensionSupport(device);

        bool swapChainAdequate = false;

        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    bool VulkanRenderer::CheckDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices VulkanRenderer::FindQueueFamilies(VkPhysicalDevice device)
    {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        uint32_t i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                m_QueueFamilyIndices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);

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

    void VulkanRenderer::CreateLogicalDevice()
    {
        QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        float queuePriority = 1.0f;

        for (uint32_t queueFamily : uniqueQueueFamilies) {

            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

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

        VkPhysicalDeviceVulkan13Features vkFeatures13 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
        vkFeatures13.dynamicRendering = VK_TRUE;
        vkFeatures13.synchronization2 = VK_TRUE;
        vkFeatures13.pipelineCreationCacheControl = VK_TRUE;
        vkFeatures13.pNext = &descriptorIndexing;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(m_DeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();
        createInfo.pNext = &vkFeatures13;

        if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS) {
            Rbk::Log::GetLogger()->critical("failed to create logical device!");
            return;
        }

        vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &m_PresentQueue);

        volkLoadDevice(m_Device);
    }

    void VulkanRenderer::CreateSurface()
    {
        VkResult result = glfwCreateWindowSurface(m_Instance, m_Window->Get(), nullptr, &m_Surface);

        if (result != VK_SUCCESS) {
            Rbk::Log::GetLogger()->critical("failed to create window surface!");
            throw std::runtime_error("failed to create window surface!");
        }
    }

    const SwapChainSupportDetails VulkanRenderer::QuerySwapChainSupport(VkPhysicalDevice device) const
    {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    VkSurfaceFormatKHR VulkanRenderer::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR VulkanRenderer::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VulkanRenderer::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        }

        int width, height;
        glfwGetFramebufferSize(m_Window->Get(), &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }

    uint32_t VulkanRenderer::GetImageCount() const
    {
        uint32_t imageCount = m_SwapChainSupport.capabilities.minImageCount + 1;

        if (m_SwapChainSupport.capabilities.maxImageCount > 0 && imageCount > m_SwapChainSupport.capabilities.maxImageCount) {
            imageCount = m_SwapChainSupport.capabilities.maxImageCount;
        }

        return imageCount;
    }

    VkSwapchainKHR VulkanRenderer::CreateSwapChain(std::vector<VkImage>& swapChainImages, const VkSwapchainKHR& oldSwapChain)
    {
        VkSwapchainKHR swapChain = VK_NULL_HANDLE;

        uint32_t imageCount = GetImageCount();

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_Surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = m_SurfaceFormat.format;
        createInfo.imageColorSpace = m_SurfaceFormat.colorSpace;
        createInfo.imageExtent = m_SwapChainExtent;
        createInfo.imageArrayLayers = 1;//1 unless stereoscopic app
        //use of VK_IMAGE_USAGE_TRANSFER_DST_BIT if post process is needed
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        createInfo.preTransform = m_SwapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = m_PresentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = oldSwapChain;

        QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);
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

        result = vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &swapChain);

        if (result != VK_SUCCESS) {
            Rbk::Log::GetLogger()->critical("Swap chain failed " + std::to_string(result));
        }

        vkGetSwapchainImagesKHR(m_Device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_Device, swapChain, &imageCount, swapChainImages.data());

        return swapChain;
    }

    bool VulkanRenderer::SouldResizeSwapChain(VkSwapchainKHR swapChain)
    {
        VkExtent2D currentExtent = GetSwapChainExtent();

        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_PhysicalDevice);
        //VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
        //VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

        uint32_t newWidth = extent.width;
        uint32_t newHeight = extent.height;

        return (currentExtent.width == newWidth && currentExtent.height == newHeight) ? false : true;
    }

    VkImageView VulkanRenderer::CreateImageView(VkImage image, VkFormat format, uint32_t mipLevels, VkImageAspectFlags aspectFlags)
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

        VkResult result;

        result = vkCreateImageView(m_Device, &createInfo, nullptr, &swapChainImageView);

        if (result != VK_SUCCESS) {
            Rbk::Log::GetLogger()->critical("failed to create image views!");
        }

        return swapChainImageView;
    }

    VkImageView VulkanRenderer::CreateSkyboxImageView(VkImage image, VkFormat format, uint32_t mipLevels, VkImageAspectFlags aspectFlags)
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

        VkResult result;

        result = vkCreateImageView(m_Device, &createInfo, nullptr, &swapChainImageView);

        if (result != VK_SUCCESS) {
            Rbk::Log::GetLogger()->critical("failed to create image views!");
        }

        return swapChainImageView;
    }

    VkDescriptorSetLayout VulkanRenderer::CreateDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& pBindings, const VkDescriptorSetLayoutCreateFlagBits& flags)
    {
        VkDescriptorSetLayout descriptorSetLayout;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(pBindings.size());
        layoutInfo.pBindings = pBindings.data();
        //layoutInfo.flags = flags;

        if (vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        return descriptorSetLayout;
    }

    VkPipelineLayout VulkanRenderer::CreatePipelineLayout(const std::vector<VkDescriptorSet>& descriptorSets, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, const std::vector<VkPushConstantRange>& pushConstants)
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

        VkResult result = vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &graphicsPipelineLayout);

        if (result != VK_SUCCESS) {
            Rbk::Log::GetLogger()->critical("failed to create pipeline layout!");
        }

        return graphicsPipelineLayout;
    }

    VkPipeline VulkanRenderer::CreateGraphicsPipeline(
        std::shared_ptr<VkRenderPass> renderPass,
        VkPipelineLayout pipelineLayout,
        std::string_view name,
        std::vector<VkPipelineShaderStageCreateInfo>shadersCreateInfos,
        VkPipelineVertexInputStateCreateInfo vertexInputInfo,
        VkCullModeFlagBits cullMode,
        bool dynamicRendering,
        bool depthTestEnable,
        bool depthWriteEnable,
        bool stencilTestEnable,
        int polygoneMode
    )
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
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = static_cast<VkPolygonMode>(polygoneMode);
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = cullMode;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f;
        rasterizer.depthBiasClamp = 0.0f;
        rasterizer.depthBiasSlopeFactor = 0.0f;

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
        depthStencil.depthTestEnable = (depthTestEnable) ? VK_TRUE : VK_FALSE;
        depthStencil.depthWriteEnable = (depthWriteEnable) ? VK_TRUE : VK_FALSE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f;
        depthStencil.maxDepthBounds = 1.0f;
        depthStencil.stencilTestEnable = (stencilTestEnable) ? VK_TRUE : VK_FALSE;
        depthStencil.front = {};
        depthStencil.back = {};

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 1.0f;
        colorBlending.blendConstants[1] = 1.0f;
        colorBlending.blendConstants[2] = 1.0f;
        colorBlending.blendConstants[3] = 1.0f;

        VkDynamicState dynamicStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = dynamicStates;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = static_cast<uint32_t>(shadersCreateInfos.size());
        pipelineInfo.pStages = shadersCreateInfos.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = (dynamicRendering) ? VK_NULL_HANDLE : *renderPass.get();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;
        pipelineInfo.pDynamicState = &dynamicState;

        VkFormat format = GetSwapChainImageFormat();
        VkFormat depthFormat = FindDepthFormat();

        VkPipelineRenderingCreateInfoKHR renderingCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR };
        renderingCreateInfo.colorAttachmentCount = 1;
        renderingCreateInfo.pColorAttachmentFormats = &format;
        renderingCreateInfo.depthAttachmentFormat = depthFormat; //(VK_FORMAT_D32_SFLOAT) 

        pipelineInfo.pNext = &renderingCreateInfo;

        VkPipeline graphicsPipeline = nullptr;

        //@todo move to a FileManager
        //@todo option to enable / disable pipeline cache
        std::string cacheFileName = "cache/pipeline_cache_data_" + std::to_string(GetDeviceProperties().vendorID) + "_" + std::to_string(GetDeviceProperties().deviceID) + "_" + name.data();

        bool badCache = false;
        size_t cacheFileSize = 0;
        void* cacheFileData = nullptr;
        std::string pReadFile = "";

        if (std::filesystem::exists(cacheFileName)) {
            std::ifstream fStream(cacheFileName, std::ios::in | std::ios::binary);
            cacheFileSize = std::filesystem::file_size(cacheFileName);
            pReadFile.resize(cacheFileSize);
            fStream.read(pReadFile.data(), cacheFileSize);
            fStream.close();
        } else {
            Rbk::Log::GetLogger()->debug("Pipeline cache miss!");
            badCache = true;
        }

        if (!pReadFile.empty()) {
            cacheFileData = (char*)malloc(sizeof(char) * cacheFileSize);
            cacheFileData = pReadFile.data();

            if (cacheFileData == nullptr) {
                Rbk::Log::GetLogger()->critical("Cannot allocate memory to pipeline cache");
            }

            Rbk::Log::GetLogger()->debug("Pipeline cache HIT from {}", cacheFileName);
        } 

        if (cacheFileData != nullptr) {
            uint32_t headerLength = 0;
            uint32_t cacheHeaderVersion = 0;
            uint32_t vendorID = 0;
            uint32_t deviceID = 0;
            uint8_t pipelineCacheUUID[VK_UUID_SIZE] = {};

            memcpy(&headerLength, static_cast<uint8_t*>(cacheFileData) + 0, 4);
            memcpy(&cacheHeaderVersion, static_cast<uint8_t*>(cacheFileData) + 4, 4);
            memcpy(&vendorID, static_cast<uint8_t*>(cacheFileData) + 8, 4);
            memcpy(&deviceID, static_cast<uint8_t*>(cacheFileData) + 12, 4);
            memcpy(pipelineCacheUUID, static_cast<uint8_t*>(cacheFileData) + 16, VK_UUID_SIZE);

            if (headerLength <= 0) {
                badCache = true;
                Rbk::Log::GetLogger()->critical("Bad header length in {} - {}", cacheFileName, headerLength);
            }
            if (cacheHeaderVersion != VK_PIPELINE_CACHE_HEADER_VERSION_ONE) {
                badCache = true;
                Rbk::Log::GetLogger()->critical("Unsupported cache header version in {} got {}", cacheFileName, cacheHeaderVersion);
            }
            if (vendorID != GetDeviceProperties().vendorID) {
                badCache = true;
                Rbk::Log::GetLogger()->critical("Vendor ID mismatch in {} got {} expect {}", cacheFileName, vendorID, GetDeviceProperties().vendorID);
            }
            if (deviceID != GetDeviceProperties().deviceID) {
                badCache = true;
                Rbk::Log::GetLogger()->critical("Device ID mismatch in {} got {} expect {}", cacheFileName, deviceID, GetDeviceProperties().deviceID);
            }
            if (memcmp(pipelineCacheUUID, GetDeviceProperties().pipelineCacheUUID, sizeof(pipelineCacheUUID)) != 0) {
                badCache = true;
                Rbk::Log::GetLogger()->critical("UUID mismatch in {} got {} expect {}", cacheFileName, pipelineCacheUUID, GetDeviceProperties().pipelineCacheUUID);
            }
            if (badCache) {
                free(cacheFileData);
                cacheFileSize = 0;
                cacheFileData = nullptr;

                Rbk::Log::GetLogger()->debug("Deleting cache entry {} to repopulate.", cacheFileName);

                if (remove(cacheFileName.c_str()) != 0) {
                    Rbk::Log::GetLogger()->critical("Reading error");
                }
            }
        }

        VkPipelineCache pipelineCache;
        VkPipelineCacheCreateInfo pCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO  };
        pCreateInfo.initialDataSize = cacheFileSize;
        pCreateInfo.pInitialData = &cacheFileData;

        VkResult result = vkCreatePipelineCache(m_Device, &pCreateInfo, nullptr, &pipelineCache);

        if (result != VK_SUCCESS) {
            Rbk::Log::GetLogger()->critical("failed to get graphics pipeline cache size!");
        }

        result = vkCreateGraphicsPipelines(m_Device, pipelineCache, 1, &pipelineInfo, nullptr, &graphicsPipeline);

        if (result != VK_SUCCESS) {
            Rbk::Log::GetLogger()->critical("failed to create graphics pipeline cache!");
        }
        
        if (result == VK_SUCCESS && badCache) {
            size_t pDataSize = 0;
            void* data = nullptr;

            //first call to get cache size with nullptr
            result = vkGetPipelineCacheData(m_Device, pipelineCache, &pDataSize, nullptr);

            if (result != VK_SUCCESS) {
                Rbk::Log::GetLogger()->critical("failed to get graphics pipeline cache size!");
            }

            data = (char*)malloc(sizeof(char) * pDataSize);

            if (!data) {
                Rbk::Log::GetLogger()->critical("failed to resize cache buffer!");
            } else {
                result = vkGetPipelineCacheData(m_Device, pipelineCache, &pDataSize, data);

                //@todo move to a FileManager
                if (result == VK_SUCCESS) {
                    std::ofstream ostrm(cacheFileName, std::ios::binary);
                    ostrm.write(static_cast<const char*>(data), pDataSize);
                    ostrm.close();
                    Rbk::Log::GetLogger()->debug("cacheData written to {}", cacheFileName);
                }
            }
        }
        return graphicsPipeline;
    }

    VkShaderModule VulkanRenderer::CreateShaderModule(const std::vector<char>& code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        VkResult result;

        result = vkCreateShaderModule(m_Device, &createInfo, nullptr, &shaderModule);

        if (result != VK_SUCCESS) {
            Rbk::Log::GetLogger()->critical("failed to create shader module!");
        }

        return shaderModule;
    }

    std::shared_ptr<VkRenderPass> VulkanRenderer::CreateRenderPass(const VkSampleCountFlagBits& msaaSamples)
    {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_PhysicalDevice);
        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);

        std::shared_ptr<VkRenderPass> renderPass = std::make_shared<VkRenderPass>();

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = surfaceFormat.format;
        colorAttachment.samples = msaaSamples;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription colorAttachmentResolve{};
        colorAttachmentResolve.format = surfaceFormat.format;
        colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentResolveRef{};
        colorAttachmentResolveRef.attachment = 2;
        colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = FindDepthFormat();
        depthAttachment.samples = msaaSamples;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
        subpass.pResolveAttachments = &colorAttachmentResolveRef;

        std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());;
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        VkResult result = vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, renderPass.get());

        if (result != VK_SUCCESS) {
            Rbk::Log::GetLogger()->critical("failed to create render pass!");
        }

        return renderPass;
    }

    std::vector<VkFramebuffer> VulkanRenderer::CreateFramebuffers(std::shared_ptr<VkRenderPass> renderPass, std::vector<VkImageView> swapChainImageViews, std::vector<VkImageView> depthImageView, std::vector<VkImageView> colorImageView)
    {
        std::vector<VkFramebuffer> swapChainFramebuffers;

        swapChainFramebuffers.resize(swapChainImageViews.size());

        for (size_t i = 0; i < swapChainImageViews.size(); i++) {

            std::array<VkImageView, 3> attachments = {
                *colorImageView.data(),
                *depthImageView.data(),
                swapChainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = *renderPass.get();
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = m_SwapChainExtent.width;
            framebufferInfo.height = m_SwapChainExtent.height;
            framebufferInfo.layers = 1;

            VkResult result = vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]);

            if (result != VK_SUCCESS) {
                Rbk::Log::GetLogger()->critical("failed to create framebuffer!");
            }            
        }

        return swapChainFramebuffers;
    }


    VkCommandPool VulkanRenderer::CreateCommandPool()
    {
        VkCommandPool commandPool;

        QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_PhysicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        VkResult result = vkCreateCommandPool(m_Device, &poolInfo, nullptr, &commandPool);

        if (result != VK_SUCCESS) {
            Rbk::Log::GetLogger()->critical("failed to create command pool!");
        }

        return commandPool;
    }

    std::vector<VkCommandBuffer> VulkanRenderer::AllocateCommandBuffers(VkCommandPool commandPool, uint32_t size, bool isSecondary)
    {
        std::vector<VkCommandBuffer> commandBuffers;

        commandBuffers.resize(size);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        //allocInfo.level = (!isSecondary) ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;

        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        VkResult result = vkAllocateCommandBuffers(m_Device, &allocInfo, commandBuffers.data());

        if (result != VK_SUCCESS) {
            Rbk::Log::GetLogger()->critical("failed to allocate command buffers!");
        }

        return commandBuffers;
    }

    void VulkanRenderer::BeginCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlagBits flags, VkCommandBufferInheritanceInfo inheritanceInfo)
    {
        vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = flags;
        beginInfo.pInheritanceInfo = (flags == VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) ? &inheritanceInfo : nullptr;

        if (VK_SUCCESS != vkBeginCommandBuffer(commandBuffer, &beginInfo)) {
            throw std::runtime_error("failed to create descriptor pool");
        }
    }

    void VulkanRenderer::SetViewPort(VkCommandBuffer commandBuffer)
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

    void VulkanRenderer::SetScissor(VkCommandBuffer commandBuffer)
    {
        VkRect2D scissor = { { 0, 0 }, { static_cast<uint32_t>(m_SwapChainExtent.width), static_cast<uint32_t>(m_SwapChainExtent.height) } };

        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void VulkanRenderer::BindPipeline(const VkCommandBuffer& commandBuffer, const VkPipeline& pipeline)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

    std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> VulkanRenderer::CreateSyncObjects(std::vector<VkImage> swapChainImages)
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
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (int i = 0; i < m_MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[static_cast<size_t>(i)]) != VK_SUCCESS ||
                vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[static_cast<size_t>(i)]) != VK_SUCCESS ||
                vkCreateFence(m_Device, &fenceInfo, nullptr, &m_InFlightFences[static_cast<size_t>(i)]) != VK_SUCCESS) {
                Rbk::Log::GetLogger()->critical("failed to create semaphores!");
            }
        }

        semaphores.first = imageAvailableSemaphores;
        semaphores.second = renderFinishedSemaphores;

        return semaphores;
    }

    VkDescriptorPool VulkanRenderer::CreateDescriptorPool(const std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets)
    {
        VkDescriptorPool descriptorPool;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = poolSizes.size();
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = maxSets;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

        if (vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool");
        }

        return descriptorPool;
    }

    VkDescriptorSet VulkanRenderer::CreateDescriptorSets(const VkDescriptorPool& descriptorPool, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, uint32_t count)
    {
        VkDescriptorSet descriptorSet;

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = count;
        allocInfo.pSetLayouts = descriptorSetLayouts.data();

        VkResult result = vkAllocateDescriptorSets(m_Device, &allocInfo, &descriptorSet);

        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets");
        }

        return descriptorSet;
    }

    void VulkanRenderer::UpdateDescriptorSets(const std::vector<Buffer>& uniformBuffers, const VkDescriptorSet& descriptorSet, const std::vector<VkDescriptorImageInfo>& imageInfo)
    {
        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
        std::vector<VkDescriptorBufferInfo> bufferInfos;

        std::for_each(std::begin(uniformBuffers), std::end(uniformBuffers),
            [&bufferInfos](const Buffer& uniformBuffer)
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

        vkUpdateDescriptorSets(m_Device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    void VulkanRenderer::BeginRenderPass(std::shared_ptr<VkRenderPass> renderPass, VkCommandBuffer commandBuffer, VkFramebuffer swapChainFramebuffer)
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = *renderPass.get();
        renderPassInfo.framebuffer = swapChainFramebuffer;
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_SwapChainExtent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.f, 1.f, 0.f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    }

    void VulkanRenderer::EndRenderPass(VkCommandBuffer commandBuffer)
    {
        vkCmdEndRenderPass(commandBuffer);
    }

    void VulkanRenderer::BeginRendering(const VkCommandBuffer& commandBuffer, const VkImageView& colorImageView, const VkImageView& depthImageView)
    {
        VkClearColorValue colorClear = { 21.f / 255.f, 10.f / 255.f, 26.f / 255.f, 1.f };
        VkClearDepthStencilValue depthStencil = { 1.f, 0 };

        VkRenderingAttachmentInfo colorAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO  };
        colorAttachment.imageView = colorImageView;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue.color = colorClear;
        
        VkRenderingAttachmentInfo depthAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        depthAttachment.imageView = depthImageView;
        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.clearValue.depthStencil = depthStencil;

        VkRenderingInfo renderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
        renderingInfo.renderArea.extent = m_SwapChainExtent;
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
        renderingInfo.pDepthAttachment = &depthAttachment;
        //renderingInfo.pStencilAttachment;

        vkCmdBeginRenderingKHR(commandBuffer, &renderingInfo);
    }

    void VulkanRenderer::EndRendering(VkCommandBuffer commandBuffer)
    {
        vkCmdEndRenderingKHR(commandBuffer);
    }

    void VulkanRenderer::EndCommandBuffer(VkCommandBuffer commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);
    }

    void VulkanRenderer::QueueSubmit(VkCommandBuffer commandBuffer)
    {
        m_MutexQueueSubmit.lock();

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_GraphicsQueue);

        m_MutexQueueSubmit.unlock();
    }

    void VulkanRenderer::QueueSubmit(uint32_t imageIndex, VkCommandBuffer commandBuffer, std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>>& semaphores)
    {
        m_MutexQueueSubmit.lock();

        std::vector<VkSemaphore>& imageAvailableSemaphores = semaphores.first;
        std::vector<VkSemaphore>& renderFinishedSemaphores = semaphores.second;

        if (m_ImagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(m_Device, 1, &m_ImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }

        m_ImagesInFlight[imageIndex] = m_ImagesInFlight[m_CurrentFrame];

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[m_CurrentFrame] };

        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[m_CurrentFrame] };

        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame]);

        VkResult result = vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]);

        m_MutexQueueSubmit.unlock();

        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }
    }

    uint32_t VulkanRenderer::QueuePresent(uint32_t imageIndex, VkSwapchainKHR swapChain, std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>>& semaphores)
    {
        std::vector<VkSemaphore>& renderFinishedSemaphores = semaphores.second;

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[m_CurrentFrame] };

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        VkResult result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);

        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        m_CurrentFrame = (m_CurrentFrame + 1) % static_cast<uint32_t>(m_MAX_FRAMES_IN_FLIGHT);

        return m_CurrentFrame;
    }

    void VulkanRenderer::WaitIdle()
    {
        vkDeviceWaitIdle(m_Device);
    }

    uint32_t VulkanRenderer::AcquireNextImageKHR(VkSwapchainKHR swapChain, std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>>& semaphores)
    {
        std::vector<VkSemaphore>& imageAvailableSemaphores = semaphores.first;

        vkWaitForFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex = 0;
        VkResult result = vkAcquireNextImageKHR(m_Device, swapChain, UINT64_MAX, imageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        return imageIndex;
    }

    void VulkanRenderer::ResetCommandPool(VkCommandPool commandPool)
    {
        vkResetCommandPool(m_Device, commandPool, 0);
    }

    void VulkanRenderer::Draw(VkCommandBuffer commandBuffer, Mesh* mesh, Data data, uint32_t frameIndex, bool drawIndexed)
    {
        VkBuffer vertexBuffers[] = { data.m_VertexBuffer.buffer };
        VkDeviceSize offsets[] = { 0 };

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            mesh->m_PipelineLayout,
            0,
            1,
            &mesh->m_DescriptorSets[frameIndex],
            0,
            nullptr
        );

        if (drawIndexed) {
            vkCmdBindIndexBuffer(commandBuffer, data.m_IndicesBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(data.m_Indices.size()), static_cast<uint32_t>(data.m_Ubos.size()), 0, 0, 0);
        } else {
            vkCmdDraw(commandBuffer, static_cast<uint32_t>(data.m_Vertices.size()), 1, 0, 0);
        }
    }

    void VulkanRenderer::AddPipelineBarriers(VkCommandBuffer commandBuffer, std::vector<VkImageMemoryBarrier> renderBarriers, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags)
    {
        vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, 0, 0, 0, 0, static_cast<uint32_t>(renderBarriers.size()), renderBarriers.data());
    }

    VkBuffer VulkanRenderer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage)
    {
        VkBuffer buffer;
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(m_Device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        return buffer;
    }

    void VulkanRenderer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(m_Device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(m_Device, buffer, bufferMemory, 0);
    }

    Buffer VulkanRenderer::CreateIndexBuffer(const VkCommandPool& commandPool, const std::vector<uint32_t>& indices)
    {
        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
      
        void* data;
        vkMapMemory(m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(m_Device, stagingBufferMemory);

        VkBuffer buffer = CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);
        uint32_t size = ((memRequirements.size / memRequirements.alignment) + 1) * memRequirements.alignment;

        auto memoryType = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        auto deviceMemory = m_DeviceMemoryPool->Get(m_Device, size, memoryType, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        auto offset = deviceMemory->GetOffset();
        deviceMemory->BindBufferToMemory(buffer, size);

        CopyBuffer(commandPool, stagingBuffer, buffer, bufferSize);

        Buffer meshBuffer;
        meshBuffer.buffer = std::move(buffer);
        meshBuffer.memory = deviceMemory->GetMemory();
        meshBuffer.offset = offset;
        meshBuffer.size = size;

        vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
        vkFreeMemory(m_Device, stagingBufferMemory, nullptr);

        return meshBuffer;
    }

    Buffer VulkanRenderer::CreateVertexBuffer(VkCommandPool commandPool, std::vector<Rbk::Vertex> vertices)
    {
        VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        CreateBuffer(
            bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory
        );

        void* data;
        vkMapMemory(m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(m_Device, stagingBufferMemory);

        VkBuffer buffer = CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

        auto memoryType = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        uint32_t size = ((memRequirements.size / memRequirements.alignment) + 1) * memRequirements.alignment;
        auto deviceMemory = m_DeviceMemoryPool->Get(m_Device, size, memoryType, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        auto offset = deviceMemory->GetOffset();
        deviceMemory->BindBufferToMemory(buffer, size);
        
        CopyBuffer(commandPool, stagingBuffer, buffer, bufferSize);

        Buffer meshBuffer;
        meshBuffer.buffer = std::move(buffer);
        meshBuffer.memory = deviceMemory->GetMemory();
        meshBuffer.offset = offset;
        meshBuffer.size = size;
        
        vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
        vkFreeMemory(m_Device, stagingBufferMemory, nullptr);

        return meshBuffer;
    }

    Buffer VulkanRenderer::CreateVertex2DBuffer(const VkCommandPool& commandPool, const std::vector<Rbk::Vertex2D>& vertices)
    {
        std::pair<VkBuffer, VkDeviceMemory> vertexBuffer{};
        VkDeviceSize bufferSize = sizeof(Vertex2D) * vertices.size();
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        CreateBuffer(
            bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory
        );

        void* data;
        vkMapMemory(m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(m_Device, stagingBufferMemory);

        VkBuffer buffer = CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

        auto memoryType = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        uint32_t size = ((memRequirements.size / memRequirements.alignment) + 1) * memRequirements.alignment;
        auto deviceMemory = m_DeviceMemoryPool->Get(m_Device, size, memoryType, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        auto offset = deviceMemory->GetOffset();
        deviceMemory->BindBufferToMemory(buffer, size);
        
        CopyBuffer(commandPool, stagingBuffer, buffer, bufferSize);

        Buffer meshBuffer;
        meshBuffer.buffer = std::move(buffer);
        meshBuffer.memory = deviceMemory->GetMemory();
        meshBuffer.offset = offset;
        meshBuffer.size = size;

        vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
        vkFreeMemory(m_Device, stagingBufferMemory, nullptr);

        return meshBuffer;
    }

    Buffer VulkanRenderer::CreateUniformBuffers(uint32_t uniformBuffersCount)
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject) * (uniformBuffersCount + 1);
        VkBuffer buffer = CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

        auto memoryType = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        uint32_t size = ((memRequirements.size / memRequirements.alignment) + 1) * memRequirements.alignment;
        auto deviceMemory = m_DeviceMemoryPool->Get(m_Device, size, memoryType, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        auto offset = deviceMemory->GetOffset();
        deviceMemory->BindBufferToMemory(buffer, size);

        Buffer uniformBuffer;
        uniformBuffer.buffer = std::move(buffer);
        uniformBuffer.memory = deviceMemory->GetMemory();
        uniformBuffer.offset = offset;
        uniformBuffer.size = size;

        return uniformBuffer;
    }

    Buffer VulkanRenderer::CreateCubeUniformBuffers(uint32_t uniformBuffersCount)
    {
        VkDeviceSize bufferSize = sizeof(CubeUniformBufferObject) * uniformBuffersCount;
        VkBuffer buffer = CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

        uint32_t size = ((memRequirements.size / memRequirements.alignment) + 1) * memRequirements.alignment;
        auto memoryType = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        auto deviceMemory = m_DeviceMemoryPool->Get(m_Device, size, memoryType, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        auto offset = deviceMemory->GetOffset();
        deviceMemory->BindBufferToMemory(buffer, size);

        Buffer uniformBuffer;
        uniformBuffer.buffer = std::move(buffer);
        uniformBuffer.memory = deviceMemory->GetMemory();
        uniformBuffer.offset = offset;
        uniformBuffer.size = size;

        return uniformBuffer;
    }

    void VulkanRenderer::UpdateUniformBuffer(Buffer buffer, std::vector<UniformBufferObject> uniformBufferObjects, uint32_t uniformBuffersCount)
    {
        void* data;
        vkMapMemory(m_Device, *buffer.memory, buffer.offset, buffer.size, 0, &data);
        memcpy(data, uniformBufferObjects.data(), buffer.size);
        vkUnmapMemory(m_Device, *buffer.memory);
    }

    void VulkanRenderer::CopyBuffer(VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        m_MutexQueueSubmit.lock();

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_GraphicsQueue);
        vkFreeCommandBuffers(m_Device, commandPool, 1, &commandBuffer);

        m_MutexQueueSubmit.unlock();
    }

    VkImageMemoryBarrier VulkanRenderer::SetupImageMemoryBarrier(VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, VkImageAspectFlags aspectMask)
    {
        VkImageMemoryBarrier result = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
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

    uint32_t VulkanRenderer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                m_MaxMemoryHeap = memProperties.memoryHeaps[memProperties.memoryTypes[i].heapIndex].size;
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    void VulkanRenderer::CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
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

        if (vkCreateImage(m_Device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_Device, image, &memRequirements);
        auto memoryType = FindMemoryType(memRequirements.memoryTypeBits, properties);
        uint32_t size = ((memRequirements.size / memRequirements.alignment) + 1) * memRequirements.alignment;
        auto deviceMemory = m_DeviceMemoryPool->Get(m_Device, size, memoryType, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        deviceMemory->BindImageToMemory(image, size);
    }

    void VulkanRenderer::CreateSkyboxImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 6;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = numSamples;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

        if (vkCreateImage(m_Device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_Device, image, &memRequirements);
        auto memoryType = FindMemoryType(memRequirements.memoryTypeBits, properties);
        uint32_t size = ((memRequirements.size / memRequirements.alignment) + 1) * memRequirements.alignment;
        auto deviceMemory = m_DeviceMemoryPool->Get(m_Device, size, memoryType, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        deviceMemory->BindImageToMemory(image, size);
    }

    void VulkanRenderer::CreateTextureImage(VkCommandBuffer commandBuffer, stbi_uc* pixels, uint32_t texWidth, uint32_t texHeight, uint32_t mipLevels, VkImage& textureImage, VkDeviceMemory& textureImageMemory, VkFormat format)
    {
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        VkBuffer buffer = CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

        auto memoryType = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        uint32_t size = ((memRequirements.size / memRequirements.alignment) + 1) * memRequirements.alignment;
        auto deviceMemory = m_DeviceMemoryPool->Get(m_Device, size, memoryType, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        auto offset = deviceMemory->GetOffset();
        deviceMemory->BindBufferToMemory(buffer, size);

        void* data;
        vkMapMemory(m_Device, *deviceMemory->GetMemory(), offset, size, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(size));
        vkUnmapMemory(m_Device, *deviceMemory->GetMemory());

        stbi_image_free(pixels);
        CreateImage(texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

        VkImageMemoryBarrier renderBarrier = SetupImageMemoryBarrier(
            textureImage, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        );

        AddPipelineBarriers(commandBuffer, { renderBarrier }, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT);
        CopyBufferToImage(commandBuffer, buffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        GenerateMipmaps(commandBuffer, format, textureImage, texWidth, texHeight, mipLevels);
        EndCommandBuffer(commandBuffer);
        QueueSubmit(commandBuffer);
    }

    void VulkanRenderer::CreateSkyboxTextureImage(VkCommandBuffer commandBuffer, std::vector<stbi_uc*>skyboxPixels, uint32_t texWidth, uint32_t texHeight, uint32_t mipLevels, VkImage& textureImage, VkDeviceMemory& textureImageMemory, VkFormat format)
    {
        VkDeviceSize imageSize = texWidth * texHeight * 4 * 6;
        VkBuffer buffer = CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

        auto memoryType = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        uint32_t size = ((memRequirements.size / memRequirements.alignment) + 1) * memRequirements.alignment;
        auto deviceMemory = m_DeviceMemoryPool->Get(m_Device, size, memoryType, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        auto offset = deviceMemory->GetOffset();
        deviceMemory->BindBufferToMemory(buffer, size);

        stbi_uc* data;
        VkDeviceSize layerSize = size / 6;
        vkMapMemory(m_Device, *deviceMemory->GetMemory(), offset, size, 0, (void**)&data);

        for (uint32_t i = 0; i < skyboxPixels.size(); i++) {
            memcpy(data + layerSize * i, skyboxPixels[i], layerSize);
        }

        vkUnmapMemory(m_Device, *deviceMemory->GetMemory());
        CreateSkyboxImage(texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
        VkImageMemoryBarrier renderBarrier = SetupImageMemoryBarrier(
            textureImage, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        );

        renderBarrier.subresourceRange.layerCount = 6;
        renderBarrier.subresourceRange.baseMipLevel = 0;
        renderBarrier.subresourceRange.levelCount = mipLevels;

        AddPipelineBarriers(commandBuffer, { renderBarrier }, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT);
        CopyBufferToImageSkybox(commandBuffer, buffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), skyboxPixels, mipLevels, layerSize);
        GenerateMipmaps(commandBuffer, format, textureImage, texWidth, texHeight, mipLevels, 6);
        EndCommandBuffer(commandBuffer);
        QueueSubmit(commandBuffer);

        for (uint32_t i = 0; i < skyboxPixels.size(); i++) {
            stbi_image_free(skyboxPixels[i]);
        }
    }

    void VulkanRenderer::GenerateMipmaps(VkCommandBuffer commandBuffer, VkFormat imageFormat, VkImage image, uint32_t texWidth, uint32_t texHeight, uint32_t mipLevels, uint32_t layerCount) {

        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, imageFormat, &formatProperties);

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

        int32_t mipWidth = static_cast<int32_t>(texWidth);
        int32_t mipHeight = static_cast<int32_t>(texHeight);

        for (uint32_t i = 1; i < mipLevels; i++) {

            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );

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

            vkCmdBlitImage(
                commandBuffer,
                image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit,
                VK_FILTER_LINEAR
            );

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }

    void VulkanRenderer::CopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
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

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    }

    void VulkanRenderer::CopyBufferToImageSkybox(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, std::vector<stbi_uc*>skyboxPixels, uint32_t mipLevels, uint64_t offset)
    {
        std::vector<VkBufferImageCopy> bufferCopyRegions;

        for (uint32_t i = 0; i < skyboxPixels.size(); i++) {

            for (uint32_t mipLevel = 0; mipLevel < mipLevels; mipLevel++) {

                VkBufferImageCopy region{};
                region.bufferOffset = offset * i;
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

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data());
    }

    VkImageView VulkanRenderer::CreateDepthResources(VkCommandBuffer commandBuffer)
    {
        VkImage depthImage;
        VkDeviceMemory depthImageMemory;
        VkImageView depthImageView;
        VkFormat depthFormat = FindDepthFormat();

        CreateImage(m_SwapChainExtent.width, m_SwapChainExtent.height, 1, VK_SAMPLE_COUNT_1_BIT, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
        depthImageView = CreateImageView(depthImage, depthFormat, 1);

        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (HasStencilComponent(depthFormat)) {
            aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }

        VkImageMemoryBarrier renderBarrier = SetupImageMemoryBarrier(
            depthImage, 0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, aspectMask
        );

        AddPipelineBarriers(commandBuffer, { renderBarrier }, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_DEPENDENCY_BY_REGION_BIT);

        return depthImageView;
    }

    VkSampler VulkanRenderer::CreateTextureSampler(uint32_t mipLevels)
    {
        VkSampler textureSampler;

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.maxAnisotropy = m_DeviceProps.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(mipLevels);
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

        if (vkCreateSampler(m_Device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }

        return textureSampler;
    }

    VkSampler VulkanRenderer::CreateSkyboxTextureSampler(uint32_t mipLevels)
    {
        VkSampler textureSampler;

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_NEAREST;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(mipLevels);
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        if (vkCreateSampler(m_Device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }

        return textureSampler;
    }

    VkFormat VulkanRenderer::FindDepthFormat()
    {
        return FindSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    VkFormat VulkanRenderer::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
        }
        throw std::runtime_error("failed to find supported format!");
    }

    bool VulkanRenderer::HasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    VkSampleCountFlagBits VulkanRenderer::GetMaxUsableSampleCount()
    {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(m_PhysicalDevice, &physicalDeviceProperties);

        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
        if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
        if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
        if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
        if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
        if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

        return VK_SAMPLE_COUNT_1_BIT;
    }

    void VulkanRenderer::CreateFence()
    {
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        vkCreateFence(m_Device, &fenceInfo, nullptr, &m_Fence);
    }

    void VulkanRenderer::WaitForFence()
    {
        vkWaitForFences(m_Device, 1, &m_Fence, true, 9999999999);
        vkResetFences(m_Device, 1, &m_Fence);
    }

    void VulkanRenderer::DestroyPipeline(VkPipeline pipeline)
    {
        vkDestroyPipeline(m_Device, pipeline, nullptr);
    }

    void VulkanRenderer::DestroyPipelineData(VkPipelineLayout pipelineLayout, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout)
    {
        vkDestroyDescriptorSetLayout(m_Device, descriptorSetLayout, nullptr);
        vkDestroyPipelineLayout(m_Device, pipelineLayout, nullptr);
        vkDestroyDescriptorPool(m_Device, descriptorPool, nullptr);
    }

    void VulkanRenderer::DestroySemaphores(std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> semaphores)
    {
        for (int i = 0; i < m_MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(m_Device, semaphores.first[i], nullptr);
            vkDestroySemaphore(m_Device, semaphores.second[i], nullptr);
            vkDestroyFence(m_Device, m_InFlightFences[i], nullptr);
            vkDestroyFence(m_Device, m_ImagesInFlight[i], nullptr);
        }
    }

    void VulkanRenderer::DestroyDeviceMemory(VkDeviceMemory deviceMemory)
    {
        vkFreeMemory(m_Device, deviceMemory, nullptr);
    }

    void VulkanRenderer::DestroyBuffer(VkBuffer buffer)
    {
        vkDestroyBuffer(m_Device, buffer, nullptr);
    }

    void VulkanRenderer::DestroyRenderPass(std::shared_ptr<VkRenderPass> renderPass, VkCommandPool commandPool, std::vector<VkCommandBuffer> commandBuffers)
    {
        vkFreeCommandBuffers(m_Device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        vkDestroyRenderPass(m_Device, *renderPass.get(), nullptr);
        vkDestroyCommandPool(m_Device, commandPool, nullptr);
    }

    void VulkanRenderer::Destroy()
    {
        vkDestroyFence(m_Device, m_Fence, nullptr);
        vkDestroyDevice(m_Device, nullptr);

        if (m_EnableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessengerCallback, nullptr);
        }

        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        vkDestroyInstance(m_Instance, nullptr);

        Rbk::Log::GetLogger()->trace("VK instance destroyed and cleaned");
    }

    void VulkanRenderer::DestroySwapchain(VkDevice device, VkSwapchainKHR swapChain, std::vector<VkFramebuffer> swapChainFramebuffers, std::vector<VkImageView> swapChainImageViews)
    {
        for (uint32_t i = 0; i < swapChainFramebuffers.size(); ++i)
            vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);

        for (uint32_t i = 0; i < swapChainImageViews.size(); ++i)
            vkDestroyImageView(device, swapChainImageViews[i], nullptr);

        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }

    VulkanRenderer::~VulkanRenderer()
    {
        std::cout << "VulkanRenderer deleted." << std::endl;
    }
}
