#include "VulkanAPI.hpp"

#include <iostream>
#include <filesystem>
#include <set>
#include <volk.h>

//@todo this class needs a huge clean up
namespace Poulpe {

  VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT const * create_info,
      VkAllocationCallbacks const * allocattor, VkDebugUtilsMessengerEXT* callback)
  {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (func != nullptr) {
      return func(instance, create_info, allocattor, callback);
    } else {
      std::cerr << "Debug utils extension not present";
      return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
  }

  //static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
  //    VkAllocationCallbacks const * allocattor)
  //{
  //    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
  //        "vkDestroyDebugUtilsMessengerEXT");
  //    if (func != nullptr) {
  //        func(instance, debugMessenger, allocattor);
  //    }
  //}

  static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    VkDebugUtilsMessengerCallbackDataEXT const * data,
    void* pUserData)
  {
    spdlog::set_pattern("%^[%T] %n: %v%$");

    switch (severity)
    {
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      {
          PLP_FATAL("{} : {}", type, data->pMessage);
          break;
      }
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      {
          PLP_WARN("{} : {}", type, data->pMessage);
          break;
      }
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
      {
          PLP_TRACE("{} : {}", type, data->pMessage);
      }
    }
    return VK_FALSE;
  }

  VulkanAPI::VulkanAPI(Window* window)
    : _window(window)
  {
#ifdef PLP_DEBUG_BUILD
    _enable_validation_layers = true;
#else
    _enable_validation_layers = false;
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

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    //VkFence fence;

    vkCreateFence(_device, & fence_info, nullptr, & _fence_acquire_image);
    vkCreateFence(_device, & fence_info, nullptr, & _fence_submit);
    vkCreateFence(_device, & fence_info, nullptr, & _fence_buffer);
  }

  void VulkanAPI::initMemoryPool()
  {
      VkPhysicalDeviceMemoryProperties mem_props;
      vkGetPhysicalDeviceMemoryProperties(_physical_device, & mem_props);

      _device_memory_pool = std::make_unique<DeviceMemoryPool>(
        _device_props2, _device_maintenance3_props, mem_props);
  }

  std::string VulkanAPI::getAPIVersion()
  {
    if (_api_version.empty()) {
      uint32_t version = VK_API_VERSION_1_3;

      //@todo fix unused ?
      auto tmp = PFN_vkEnumerateInstanceVersion(vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));
      (void)tmp;

      if (vkEnumerateInstanceVersion) {
          vkEnumerateInstanceVersion(&version);
      }

      uint32_t major = VK_VERSION_MAJOR(version);
      uint32_t minor = VK_VERSION_MINOR(version);
      uint32_t patch = VK_VERSION_PATCH(version);

      _api_version = std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
    }
    return _api_version;
  }

  void VulkanAPI::createInstance()
  {
    std::string message;

    if (!isValidationLayersEnabled() && !checkValidationLayerSupport()) {
        PLP_WARN("Validations layers not available !");
    }

    VkApplicationInfo app_info{};

    app_info.apiVersion = VK_API_VERSION_1_3;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "PoulpeEngine";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    create_info.enabledExtensionCount = static_cast<uint32_t>(_required_extensions.size());
    create_info.ppEnabledExtensionNames = _required_extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};

    if (_enable_validation_layers) {
      debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
          | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT 
          | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      debug_create_info.pfnUserCallback = DebugCallback;
      debug_create_info.pUserData = nullptr;

      create_info.enabledLayerCount = static_cast<uint32_t>(_validation_layers.size());
      create_info.ppEnabledLayerNames = _validation_layers.data();
      create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
      PLP_TRACE("Validations enabled");
    } else {
      PLP_TRACE("Validations disabled");
      create_info.enabledLayerCount = 0;
    }

    VkResult result = VK_SUCCESS;

    result = vkCreateInstance(& create_info, nullptr, &_instance);

    if (VK_SUCCESS != result) {
      PLP_FATAL("Can't create VK instance : {}", result);
      throw std::runtime_error("Can't create VK instance.");
    }

    _instance_created = true;

    volkLoadInstance(_instance);
  }

  void VulkanAPI::initDetails()
  {
    _swapchain_support = querySwapChainSupport(_physical_device);
    _surface_format = chooseSwapSurfaceFormat(_swapchain_support.formats);
    _present_mode = chooseSwapPresentMode(_swapchain_support.presentModes);
    _swapchain_extent = chooseSwapExtent(_swapchain_support.capabilities);
    _swapchain_iImage_format = _surface_format.format;
  }

  void VulkanAPI::enumerateExtensions()
  {
    vkEnumerateInstanceExtensionProperties(nullptr, & _extension_count, nullptr);
    std::vector<VkExtensionProperties> extensions(_extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, & _extension_count, extensions.data());
    _extensions = extensions;
  }

  bool VulkanAPI::checkValidationLayerSupport()
  {
    uint32_t layer_count{ 0 };
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    for (std::string const layer_name : _validation_layers) {
      bool founded = false;

      for (auto const & layer_props : available_layers) {
        if (strcmp(layer_name.c_str(), layer_props.layerName) == 0) {
          founded = true;
          break;
        }
      }
      if (!founded) {
        return false;
      }
    }
    _layers_available = available_layers;

    return false;
  }

  void VulkanAPI::loadRequiredExtensions()
  {
    uint32_t glfw_ext_count{ 0 };
    char const ** glfw_ext;
    glfw_ext = glfwGetRequiredInstanceExtensions(& glfw_ext_count);

    std::vector<char const*> extensions(glfw_ext, glfw_ext + glfw_ext_count);
        
    if (_enable_validation_layers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        //extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME); deprecated
        //extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME); deprecated
    }
    _required_extensions = extensions;
  }

  void VulkanAPI::setupDebugMessenger()
  {
    if (!_enable_validation_layers) return;

    VkDebugUtilsMessengerCreateInfoEXT create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT 
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = DebugCallback;
    create_info.pUserData = nullptr;

    if (CreateDebugUtilsMessengerEXT(_instance, & create_info, nullptr, &_debug_msg_callback) != VK_SUCCESS) {
      PLP_ERROR("Can't create debug messenger.");
    }
  }

  void VulkanAPI::pickPhysicalDevice()
  {
    uint32_t device_count{ 0 };
    vkEnumeratePhysicalDevices(_instance, & device_count, nullptr);

    if (device_count == 0) {
      PLP_FATAL("failed to find GPUs with Vulkan support!");
      exit(-1);
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(_instance, & device_count, devices.data());

    for (auto &device : devices) {
      if (isDeviceSuitable(device)) {
        _physical_device = device;

        VkPhysicalDeviceProperties device_props;
        VkPhysicalDeviceFeatures devices_features;
        VkPhysicalDeviceMaintenance3Properties device_maintenance3_props{};
        device_maintenance3_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES;
            
        VkPhysicalDeviceProperties2 device_props2{};
        device_props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        device_props2.pNext = &device_maintenance3_props;

        vkGetPhysicalDeviceProperties(device, &device_props);
        vkGetPhysicalDeviceFeatures(device, &devices_features);
        vkGetPhysicalDeviceProperties2(device, &device_props2);

        _device_props = device_props;
        _device_features = devices_features;
        _device_props2 = device_props2;
        _device_maintenance3_props = device_maintenance3_props;

        _sample_count = getMaxUsableSampleCount();

        break;
      }
    }
    if (_physical_device == VK_NULL_HANDLE) {
      PLP_FATAL("failed to find a suitable GPU");
      exit(-1);
    }
  }

  bool VulkanAPI::isDeviceSuitable(VkPhysicalDevice& device)
  {
    QueueFamilyIndices indices = findQueueFamilies(device);
    bool ext_supported{ checkDeviceExtensionSupport(device) };
    bool swapChainAdequate{ false };

    if (ext_supported) {
      SwapChainSupportDetails swapchain_support{ querySwapChainSupport(device) };
      swapChainAdequate = !swapchain_support.formats.empty() && !swapchain_support.presentModes.empty();
    }

    return indices.isComplete() && ext_supported && swapChainAdequate;
  }

  bool VulkanAPI::checkDeviceExtensionSupport(VkPhysicalDevice& device)
  {
    uint32_t ext_count{ 0 };
    vkEnumerateDeviceExtensionProperties(device, nullptr, & ext_count, nullptr);

    std::vector<VkExtensionProperties> available_ext(ext_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, & ext_count, available_ext.data());

    std::set<std::string> required_ext(_device_extensions.begin(), _device_extensions.end());

    for (auto const & extension : available_ext) {
        required_ext.erase(extension.extensionName);
    }
    return required_ext.empty();
  }

  QueueFamilyIndices VulkanAPI::findQueueFamilies(VkPhysicalDevice& device)
  {
    uint32_t queue_family_count{ 0 };
    vkGetPhysicalDeviceQueueFamilyProperties(device, & queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, & queue_family_count, queue_families.data());

    uint32_t i{ 0 };
    for (auto const & queueFamily : queue_families) {
      if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
         _queue_family_indices.graphicsFamily = i;
      }

      VkBool32 present_support{ false };
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, & present_support);

      if (present_support) {
        _queue_family_indices.presentFamily = i;
      }
      if (_queue_family_indices.isComplete()) {
        break;
      }
      i++;
    }
    return _queue_family_indices;
  }

  void VulkanAPI::createLogicalDevice()
  {
    QueueFamilyIndices indices = findQueueFamilies(_physical_device);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    float queue_priority{ 1.0f };
    _graphics_queues.resize(_queue_count);
    _present_queues.resize(_queue_count);

    for (uint32_t queue_family : unique_queue_families) {
      VkDeviceQueueCreateInfo queueCreateInfo{};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queue_family;
      queueCreateInfo.queueCount = _queue_count;
      queueCreateInfo.pQueuePriorities = &queue_priority;
      queue_create_infos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures device_features{};
    device_features.fillModeNonSolid =  VK_TRUE;
    device_features.samplerAnisotropy = VK_TRUE;
    device_features.sampleRateShading = VK_TRUE;
    device_features.imageCubeArray = VK_TRUE;
    device_features.geometryShader = VK_TRUE;
    device_features.depthBiasClamp = VK_TRUE;
    device_features.depthClamp = VK_TRUE;

    VkPhysicalDeviceDescriptorIndexingFeatures descriptor_indexing{};
    descriptor_indexing.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    descriptor_indexing.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    descriptor_indexing.runtimeDescriptorArray = VK_TRUE;
    descriptor_indexing.descriptorBindingVariableDescriptorCount = VK_TRUE;
    descriptor_indexing.descriptorBindingPartiallyBound = VK_TRUE;

    VkPhysicalDeviceVulkan13Features vulkan_features13 = {};
    vulkan_features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    vulkan_features13.dynamicRendering = VK_TRUE;
    vulkan_features13.synchronization2 = VK_TRUE;
    vulkan_features13.pipelineCreationCacheControl = VK_TRUE;
    vulkan_features13.pNext = &descriptor_indexing;

    VkPhysicalDeviceShaderDrawParametersFeatures shader_draw_parameters_features = {};
    shader_draw_parameters_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
    shader_draw_parameters_features.pNext = &vulkan_features13;
    shader_draw_parameters_features.shaderDrawParameters = VK_TRUE;

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT ext_dynamic_state{};
    ext_dynamic_state.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT;
    ext_dynamic_state.extendedDynamicState3DepthClampEnable = VK_TRUE;
    ext_dynamic_state.extendedDynamicState3RasterizationSamples = VK_TRUE;
    ext_dynamic_state.pNext = &shader_draw_parameters_features;

    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    create_info.pQueueCreateInfos = queue_create_infos.data();
    create_info.pEnabledFeatures = &device_features;
    create_info.enabledExtensionCount = static_cast<uint32_t>(_device_extensions.size());
    create_info.ppEnabledExtensionNames = _device_extensions.data();
    create_info.pNext = &ext_dynamic_state;

    if (vkCreateDevice(_physical_device, & create_info, nullptr, &_device) != VK_SUCCESS) {
      PLP_FATAL("failed to create logical device!");
      return;
    }

    for (uint32_t i { 0 }; i < _queue_count; i++) {
      _graphics_queues[i] = VK_NULL_HANDLE;
      _present_queues[i] = VK_NULL_HANDLE;
      vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, & _graphics_queues[i]);
      vkGetDeviceQueue(_device, indices.presentFamily.value(), 0, & _present_queues[i]);
    }
    volkLoadDevice(_device);
  }

  void VulkanAPI::createSurface()
  {
    VkResult result = glfwCreateWindowSurface(_instance, _window->get(), nullptr, & _surface);

    if (result != VK_SUCCESS) {
      PLP_FATAL("failed to create window surface!");
      throw std::runtime_error("failed to create window surface!");
    }
  }

  const SwapChainSupportDetails VulkanAPI::querySwapChainSupport(VkPhysicalDevice device) const
  {
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, & details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, & format_count, nullptr);

    if (format_count != 0) {
      details.formats.resize(format_count);
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, & format_count, details.formats.data());
    }
    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, & present_mode_count, nullptr);

    if (present_mode_count != 0) {
      details.presentModes.resize(present_mode_count);
      vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &present_mode_count, details.presentModes.data());
    }
    return details;
  }

  VkSurfaceFormatKHR VulkanAPI::chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> const & available_formats)
  {
    for (auto const & available_format : available_formats) {
      if (available_format.format == VK_FORMAT_B8G8R8A8_UNORM 
        && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        return available_format;
      }
    }
    return available_formats[0];
  }

  VkPresentModeKHR VulkanAPI::chooseSwapPresentMode(std::vector<VkPresentModeKHR> const & available_present_modes)
  {
    return VK_PRESENT_MODE_FIFO_KHR;
  }

  VkExtent2D VulkanAPI::chooseSwapExtent( VkSurfaceCapabilitiesKHR const & capabilities)
  {
    if (capabilities.currentExtent.width != UINT32_MAX) {
      return capabilities.currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(_window->get(), &width, &height);

    VkExtent2D actual_extent = {
      static_cast<uint32_t>(width),
      static_cast<uint32_t>(height)
    };

    actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actual_extent;
  }

  uint32_t VulkanAPI::getImageCount() const
  {
    uint32_t image_count = _swapchain_support.capabilities.minImageCount;

    if (_swapchain_support.capabilities.maxImageCount > 0 
      && image_count > _swapchain_support.capabilities.maxImageCount) {
      image_count = _swapchain_support.capabilities.maxImageCount;
    }
    return image_count;
  }

  VkSwapchainKHR VulkanAPI::createSwapChain(
    std::vector<VkImage>& swapchain_images,
    VkSwapchainKHR const& old_swapchain)
  {
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;

    uint32_t image_count = getImageCount();

    VkImageSubresourceLayers src_img_subrc_layers;
    src_img_subrc_layers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    src_img_subrc_layers.mipLevel = 1;
    src_img_subrc_layers.baseArrayLayer = 1;
    src_img_subrc_layers.layerCount = 1;

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = _surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = _surface_format.format;
    create_info.imageColorSpace = _surface_format.colorSpace;
    create_info.imageExtent = _swapchain_extent;
    create_info.imageArrayLayers = 1;//1 unless stereoscopic app
    //use of VK_IMAGE_USAGE_TRANSFER_DST_BIT if post process is needed
    create_info.imageUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT 
        | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    create_info.preTransform = _swapchain_support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = _present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = old_swapchain;

    QueueFamilyIndices indices = findQueueFamilies(_physical_device);
    uint32_t queue_family_indices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) {
      create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      create_info.queueFamilyIndexCount = 2;
      create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
      create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      create_info.queueFamilyIndexCount = 0;
      create_info.pQueueFamilyIndices = nullptr;
    }
    VkResult result;

    result = vkCreateSwapchainKHR(_device, & create_info, nullptr, & swapchain);

    if (result != VK_SUCCESS) {
        PLP_FATAL("Swap chain failed " + std::to_string(result));
    }
    vkGetSwapchainImagesKHR(_device, swapchain, &image_count, nullptr);
    swapchain_images.resize(image_count);
    vkGetSwapchainImagesKHR(_device, swapchain, &image_count, swapchain_images.data());

    return swapchain;
  }

  bool VulkanAPI::souldResizeSwapChain()
  {
    VkExtent2D current_extent = getSwapChainExtent();

    SwapChainSupportDetails swapchain_support = querySwapChainSupport(_physical_device);
    //VkSurfaceFormatKHR surface_format = ChooseSwapSurfaceFormat(swapchain_support.formats);
    //VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapchain_support.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapchain_support.capabilities);

    uint32_t width = extent.width;
    uint32_t height = extent.height;

    return (current_extent.width == width && current_extent.height == height) ? false : true;
  }

  VkImageView VulkanAPI::createImageView(
    VkImage& image,
    VkFormat const format,
    uint32_t const mip_lvl,
    VkImageAspectFlags const aspect_flags)
  {
    VkImageView swapchain_image_view{};

    VkImageViewCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = image;
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = format;
    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.subresourceRange.aspectMask = aspect_flags;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = mip_lvl;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;

    VkResult result{ VK_SUCCESS };

    result = vkCreateImageView(_device, &create_info, nullptr, &swapchain_image_view);

    if (result != VK_SUCCESS) {
      PLP_ERROR("failed to create image views!");
    }
    return swapchain_image_view;
  }

  VkImageView VulkanAPI::createSkyboxImageView(
    VkImage& image,
    VkFormat const format,
    uint32_t const mip_lvl,
    VkImageAspectFlags const aspect_flags)
  {
    VkImageView swapchain_image_view{};

    VkImageViewCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = image;
    create_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    create_info.format = format;
    create_info.components.r = VK_COMPONENT_SWIZZLE_R;
    create_info.components.g = VK_COMPONENT_SWIZZLE_G;
    create_info.components.b = VK_COMPONENT_SWIZZLE_B;
    create_info.components.a = VK_COMPONENT_SWIZZLE_A;
    create_info.subresourceRange = { aspect_flags, 0, 1, 0, 1 };
    create_info.subresourceRange.levelCount = mip_lvl;
    create_info.subresourceRange.layerCount = 6;

    VkResult result{ VK_SUCCESS };

    result = vkCreateImageView(_device, &create_info, nullptr, &swapchain_image_view);

    if (result != VK_SUCCESS) {
      PLP_ERROR("failed to create image views!");
    }
    return swapchain_image_view;
  }

  VkDescriptorSetLayout VulkanAPI::createDescriptorSetLayout(
    std::vector<VkDescriptorSetLayoutBinding> const& bindings)
  {
    VkDescriptorSetLayout descset_layout{};

    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
    layout_info.pBindings = bindings.data();
    layout_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    //VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
    if (vkCreateDescriptorSetLayout(_device, &layout_info, nullptr, &descset_layout) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create descriptor set layout!");
    }
    return descset_layout;
  }

  VkPipelineLayout VulkanAPI::createPipelineLayout(
    std::vector<VkDescriptorSetLayout> const& descset_layouts,
      std::vector<VkPushConstantRange> const& push_const)
  {
    VkPipelineLayout graphics_pipeline_layout{ VK_NULL_HANDLE };

    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    if (0 != push_const.size()) {
      pipeline_layout_info.pushConstantRangeCount = static_cast<uint32_t>(push_const.size());
      pipeline_layout_info.pPushConstantRanges = push_const.data();
    }
    pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descset_layouts.size());
    pipeline_layout_info.pSetLayouts = descset_layouts.data();

    VkResult result = vkCreatePipelineLayout(_device, &pipeline_layout_info, nullptr, &graphics_pipeline_layout);

    if (result != VK_SUCCESS) {
        PLP_ERROR("failed to create pipeline layout!");
    }
    return graphics_pipeline_layout;
  }

  //@todo refactor this...
  VkPipeline VulkanAPI::createGraphicsPipeline(
    VkPipelineLayout& pipeline_layout,
    std::string_view name,
    std::vector<VkPipelineShaderStageCreateInfo>& shaders_create_info,
    VkPipelineVertexInputStateCreateInfo& vertex_input_info,
    VkCullModeFlagBits const cull_mode,
    bool const has_depth_test,
    bool const has_depth_write,
    bool const has_stencil_test,
    int const polygone_mode,
    bool const has_color_attachment,
    bool const has_dynamic_depth_bias)
  {
    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(_swapchain_extent.width);
    viewport.height = static_cast<float>(_swapchain_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = _swapchain_extent;

    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = & viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = & scissor;

    //@todo extension not working ?
    VkPipelineRasterizationDepthClipStateCreateInfoEXT depth_clip_state{};
    depth_clip_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT;
    depth_clip_state.depthClipEnable = VK_TRUE;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_TRUE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = static_cast<VkPolygonMode>(polygone_mode);
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = cull_mode;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;
    rasterizer.pNext = &depth_clip_state;

    VkPipelineMultisampleStateCreateInfo multi_sampling{};
    multi_sampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multi_sampling.sampleShadingEnable = VK_FALSE;
    multi_sampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;//_sample_count;
    multi_sampling.minSampleShading = 0.2f;
    multi_sampling.pSampleMask = nullptr;
    multi_sampling.alphaToCoverageEnable = VK_FALSE;
    multi_sampling.alphaToOneEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = (has_depth_test == true) ? VK_TRUE : VK_FALSE;
    depth_stencil.depthWriteEnable = (has_depth_write == true) ? VK_TRUE : VK_FALSE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.minDepthBounds = 0.0f;
    depth_stencil.maxDepthBounds = 1.0f;
    depth_stencil.stencilTestEnable = (has_stencil_test == true) ? VK_TRUE : VK_FALSE;
    depth_stencil.front = {};
    depth_stencil.back = {};

    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
        | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_TRUE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = & color_blend_attachment;
    color_blending.blendConstants[0] = 1.0f;
    color_blending.blendConstants[1] = 1.0f;
    color_blending.blendConstants[2] = 1.0f;
    color_blending.blendConstants[3] = 1.0f;

    std::vector<VkDynamicState> dynamic_states{
        VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    if (has_dynamic_depth_bias) dynamic_states.emplace_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
        
    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = dynamic_states.size();
    dynamic_state.pDynamicStates = dynamic_states.data();

    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = static_cast<uint32_t>(shaders_create_info.size());
    pipeline_info.pStages = shaders_create_info.data();
    pipeline_info.pVertexInputState = & vertex_input_info;
    pipeline_info.pInputAssemblyState = & input_assembly;
    pipeline_info.pViewportState = & viewport_state;
    pipeline_info.pRasterizationState = & rasterizer;
    pipeline_info.pMultisampleState = & multi_sampling;
    if (has_depth_test) pipeline_info.pDepthStencilState = & depth_stencil;
    pipeline_info.pColorBlendState = & color_blending;
    pipeline_info.layout = pipeline_layout;
    pipeline_info.renderPass = VK_NULL_HANDLE;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex = -1;
    pipeline_info.pDynamicState = & dynamic_state;

    VkFormat format = getSwapChainImageFormat();
    VkFormat depth_format = findDepthFormat();

    VkPipelineRenderingCreateInfoKHR rendering_create_info = { };
    rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    rendering_create_info.colorAttachmentCount = has_color_attachment ? 1 : 0;
    if (has_color_attachment) rendering_create_info.pColorAttachmentFormats = & format;
    if (has_depth_test) rendering_create_info.depthAttachmentFormat = VK_FORMAT_D24_UNORM_S8_UINT; //(VK_FORMAT_D24_UNORM_S8_UINT) 

    pipeline_info.pNext = & rendering_create_info;

    VkPipeline graphics_pipeline = nullptr;

    {
      //std::lock_guard<std::mutex> guard(_MutexGraphicsPipeline);

      //@todo move to a FileManager
      //@todo option to enable / disable pipeline cache
      std::string cache_filename = "./cache/pipeline_cache_data_" + std::to_string(_device_props.vendorID) +
          "_" + std::to_string(_device_props.deviceID) + "_" + name.data() + ".bin";

      bool bad_cache = false;
      std::streamsize cache_file_size = 0;
      void* cache_file_data = nullptr;
      std::string p_read_file = "";

      if (std::filesystem::exists(cache_filename)) {
        std::ifstream fStream(cache_filename, std::ios::in | std::ios::binary);
        cache_file_size = std::filesystem::file_size(cache_filename);
        p_read_file.resize(cache_file_size);
        fStream.read(p_read_file.data(), cache_file_size);
        fStream.close();
      } else {
        //PLP_TRACE("Pipeline cache miss!");
        bad_cache = true;
      }

      if (!p_read_file.empty()) {
        cache_file_data = (char*)malloc(sizeof(char) * cache_file_size);
        cache_file_data = p_read_file.data();

        if (cache_file_data == nullptr) {
          PLP_WARN("Cannot allocate memory to pipeline cache");
        }
        //PLP_TRACE("Pipeline cache HIT from {}", cache_filename);
      }

      if (cache_file_data != nullptr) {
        uint32_t header_length{ 0 };
        uint32_t cache_header_version{ 0 };
        uint32_t vendor_ID{ 0 };
        uint32_t device_ID{ 0 };
        uint8_t pipeline_cache_UUID[VK_UUID_SIZE] = {};

        memcpy(& header_length, static_cast<uint8_t*>(cache_file_data) + 0, 4);
        memcpy(& cache_header_version, static_cast<uint8_t*>(cache_file_data) + 4, 4);
        memcpy(& vendor_ID, static_cast<uint8_t*>(cache_file_data) + 8, 4);
        memcpy(& device_ID, static_cast<uint8_t*>(cache_file_data) + 12, 4);
        memcpy(& pipeline_cache_UUID, static_cast<uint8_t*>(cache_file_data) + 16, VK_UUID_SIZE);

        if (header_length <= 0) {
          bad_cache = true;
          PLP_ERROR("Bad header length in {} - {}", cache_filename, header_length);
        }
        if (cache_header_version != VK_PIPELINE_CACHE_HEADER_VERSION_ONE) {
          bad_cache = true;
          PLP_ERROR("Unsupported cache header version in {} got {}", cache_filename, cache_header_version);
        }
        if (vendor_ID != static_cast<uint32_t>(_device_props.vendorID)) {
          bad_cache = true;
          PLP_ERROR("Vendor ID mismatch in {} got {} expect {}", cache_filename, vendor_ID, _device_props.vendorID);
        }
        if (device_ID != static_cast<uint32_t>(_device_props.deviceID)) {
          bad_cache = true;
          PLP_ERROR("Device ID mismatch in {} got {} expect {}", cache_filename, device_ID, _device_props.deviceID);
        }
        if (memcmp(pipeline_cache_UUID,  _device_props.pipelineCacheUUID, sizeof(pipeline_cache_UUID)) != 0) {
          PLP_ERROR("UUID mismatch in {} got {} expect {}", cache_filename, *pipeline_cache_UUID, *_device_props.pipelineCacheUUID);
        }

        if (bad_cache) {
          free(cache_file_data);
          cache_file_size = 0;
          cache_file_data = nullptr;

          PLP_TRACE("Deleting cache entry {} to repopulate.", cache_filename);

          if (remove(cache_filename.c_str()) != 0) {
            PLP_ERROR("Reading error");
          }
        }
      }

      VkPipelineCache pipeline_cache;
      VkPipelineCacheCreateInfo p_create_info = { };
      p_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
      p_create_info.initialDataSize = cache_file_size;
      p_create_info.pInitialData = &cache_file_data;

      VkResult result = vkCreatePipelineCache(_device, & p_create_info, nullptr, & pipeline_cache);

      if (result != VK_SUCCESS) {
        PLP_ERROR("failed to get graphics pipeline cache size!");
      }
      result = vkCreateGraphicsPipelines(_device, pipeline_cache, 1, & pipeline_info, nullptr, & graphics_pipeline);

      if (result != VK_SUCCESS) {
        PLP_ERROR("failed to create graphics pipeline cache!");
      }
      if (result == VK_SUCCESS && bad_cache) {
        size_t p_data_size = 0;
        void* data = nullptr;

        //first call to get cache size with nullptr
        result = vkGetPipelineCacheData(_device, pipeline_cache, & p_data_size, nullptr);

        if (result != VK_SUCCESS) {
          PLP_ERROR("failed to get graphics pipeline cache size!");
        }
        data = (char*)malloc(sizeof(char) * p_data_size);

        if (!data) {
          PLP_ERROR("failed to resize cache buffer!");
        } else {
          result = vkGetPipelineCacheData(_device, pipeline_cache, & p_data_size, data);

          //@todo move to a FileManager
          if (result == VK_SUCCESS) {
            std::ofstream ostrm(cache_filename, std::ios::binary);
            ostrm.write(static_cast<const char*>(data), p_data_size);
            ostrm.close();
            PLP_TRACE("cacheData written to {}", cache_filename);
          }
        }
      }
    }
    return graphics_pipeline;
  }

  VkShaderModule VulkanAPI::createShaderModule(std::vector<char> const& code)
  {
    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shader_module{};
    VkResult result{ VK_SUCCESS };

    result = vkCreateShaderModule(_device, &create_info, nullptr, &shader_module);

    if (result != VK_SUCCESS) {
        PLP_ERROR("failed to create shader module!");
    }
    return shader_module;
  }

  VkRenderPass* VulkanAPI::createRenderPass(VkSampleCountFlagBits const& msaaSamples)
  {
    SwapChainSupportDetails swapchain_support = querySwapChainSupport(_physical_device);
    VkSurfaceFormatKHR surface_format = chooseSwapSurfaceFormat(swapchain_support.formats);

    VkRenderPass* rdr_pass = new VkRenderPass();

    VkAttachmentDescription color_attachment{};
    color_attachment.format = surface_format.format;
    color_attachment.samples = msaaSamples;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription color_attachment_resolve{};
    color_attachment_resolve.format = surface_format.format;
    color_attachment_resolve.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment_resolve.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment_resolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment_resolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment_resolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment_resolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment_resolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_resolve_ref{};
    color_attachment_resolve_ref.attachment = 2;
    color_attachment_resolve_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription depth_attachment{};
    depth_attachment.format = findDepthFormat();
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = & color_attachment_ref;
    subpass.pDepthStencilAttachment = & depth_attachment_ref;
    subpass.pResolveAttachments = & color_attachment_resolve_ref;

    std::array<VkAttachmentDescription, 3> attachments = { color_attachment, depth_attachment, color_attachment_resolve };

    VkRenderPassCreateInfo rdr_pass_info{};
    rdr_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rdr_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    rdr_pass_info.pAttachments = attachments.data();
    rdr_pass_info.subpassCount = 1;
    rdr_pass_info.pSubpasses = & subpass;
    rdr_pass_info.dependencyCount = 1;
    rdr_pass_info.pDependencies = & dependency;

    VkResult result = vkCreateRenderPass(_device, & rdr_pass_info, nullptr, rdr_pass);

    if (result != VK_SUCCESS) {
      PLP_FATAL("failed to create render pass!");
    }
    return rdr_pass;
  }

  std::vector<VkFramebuffer> VulkanAPI::createFramebuffers(
    VkRenderPass& rdr_pass,
    std::vector<VkImageView>& swapchain_image_views,
    std::vector<VkImageView>& depth_imageview,
    std::vector<VkImageView>& colorImageView)
  {
    std::vector<VkFramebuffer> swapchain_framebuffers;

    swapchain_framebuffers.resize(swapchain_image_views.size());

    for (size_t i { 0 }; i < swapchain_image_views.size(); i++) {

      std::array<VkImageView, 3> attachments {
          *colorImageView.data(),
          *depth_imageview.data(),
          swapchain_image_views[i]
      };

      VkFramebufferCreateInfo frame_buffer_info{};
      frame_buffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      frame_buffer_info.renderPass = rdr_pass;
      frame_buffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
      frame_buffer_info.pAttachments = attachments.data();
      frame_buffer_info.width = _swapchain_extent.width;
      frame_buffer_info.height = _swapchain_extent.height;
      frame_buffer_info.layers = 1;

      VkResult result = vkCreateFramebuffer(_device, & frame_buffer_info, nullptr, & swapchain_framebuffers[i]);

      if (result != VK_SUCCESS) {
        PLP_ERROR("failed to create framebuffer!");
      }
    }
    return swapchain_framebuffers;
  }

  VkCommandPool VulkanAPI::createCommandPool()
  {
    VkCommandPool cmd_pool;

    QueueFamilyIndices queue_family_indices = findQueueFamilies(_physical_device);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queue_family_indices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkResult result = vkCreateCommandPool(_device, & poolInfo, nullptr, & cmd_pool);

    if (result != VK_SUCCESS) {
      PLP_ERROR("failed to create command pool!");
    }
    return cmd_pool;
  }

  std::vector<VkCommandBuffer> VulkanAPI::allocateCommandBuffers(
    VkCommandPool& cmd_pool,
    uint32_t const size,
    bool const is_secondary)
  {
    std::vector<VkCommandBuffer> cmd_buffers;
    cmd_buffers.resize(size);

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = cmd_pool;
    alloc_info.level = (!is_secondary) ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    alloc_info.commandBufferCount = static_cast<uint32_t>(cmd_buffers.size());

    VkResult result = vkAllocateCommandBuffers(_device, & alloc_info, cmd_buffers.data());

    if (result != VK_SUCCESS) {
      PLP_ERROR("failed to allocate command buffers!");
    }
    return cmd_buffers;
  }

  void VulkanAPI::beginCommandBuffer(
    VkCommandBuffer& cmd_buffer,
    VkCommandBufferUsageFlagBits const flags,
    VkCommandBufferInheritanceInfo const inheritance_info)
  {
    vkResetCommandBuffer(cmd_buffer, 0);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | flags;
    begin_info.pInheritanceInfo = (flags == VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) ? &inheritance_info : nullptr;

    if (VK_SUCCESS != vkBeginCommandBuffer(cmd_buffer, & begin_info)) {
      throw std::runtime_error("failed to create descriptor pool");
    }
  }

  void VulkanAPI::setViewPort(VkCommandBuffer& cmd_buffer)
  {
    VkViewport viewport;
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = static_cast<float>(_swapchain_extent.width);
    viewport.height = static_cast<float>(_swapchain_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);
  }

  void VulkanAPI::setScissor(VkCommandBuffer& cmd_buffer)
  {
    VkRect2D scissor = { { 0, 0 }, { static_cast<uint32_t>(_swapchain_extent.width),
        static_cast<uint32_t>(_swapchain_extent.height) } };

    vkCmdSetScissor(cmd_buffer, 0, 1, & scissor);
  }

  void VulkanAPI::bindPipeline(VkCommandBuffer& cmd_buffer, VkPipeline& pipeline)
  {
    vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  }

  VkDescriptorPool VulkanAPI::createDescriptorPool(
    std::vector<VkDescriptorPoolSize> const& pool_sizes,
    uint32_t const max_sets,
    VkDescriptorPoolCreateFlags const flags)
  {
    VkDescriptorPool descriptor_pool{};

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = pool_sizes.size();
    pool_info.pPoolSizes = pool_sizes.data();
    pool_info.maxSets = max_sets;
    pool_info.flags = flags;

    if (vkCreateDescriptorPool(_device, &pool_info, nullptr, & descriptor_pool) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create descriptor pool");
    }
    return descriptor_pool;
  }

  VkDescriptorSet VulkanAPI::createDescriptorSets(
    VkDescriptorPool const& descriptor_pool,
    std::vector<VkDescriptorSetLayout> const& descset_layout,
    uint32_t const count)
  {
    VkDescriptorSet descset{};
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = descriptor_pool;
    alloc_info.descriptorSetCount = count;
    alloc_info.pSetLayouts = descset_layout.data();
    
    VkResult result = vkAllocateDescriptorSets(_device, & alloc_info, & descset);

    if (result != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate descriptor sets");
    }
    return descset;
  }

  void VulkanAPI::updateDescriptorSets(
    std::vector<Buffer>& uniform_buffers,
    VkDescriptorSet& descset,
    std::vector<VkDescriptorImageInfo>& image_info,
    VkDescriptorType const type)
  {
    std::array<VkWriteDescriptorSet, 2> desc_writes{};
    std::vector<VkDescriptorBufferInfo> buffer_infos;

    std::for_each(std::begin(uniform_buffers), std::end(uniform_buffers),
    [& buffer_infos](const Buffer & uniformBuffer)
    {
      VkDescriptorBufferInfo buffer_info{};
      buffer_info.buffer = uniformBuffer.buffer;
      buffer_info.offset = 0;
      buffer_info.range = VK_WHOLE_SIZE;
      buffer_infos.emplace_back(buffer_info);
    });

    desc_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_writes[0].dstSet = descset;
    desc_writes[0].dstBinding = 0;
    desc_writes[0].dstArrayElement = 0;
    desc_writes[0].descriptorType = type;
    desc_writes[0].descriptorCount = 1;
    desc_writes[0].pBufferInfo = buffer_infos.data();

    desc_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_writes[1].dstSet = descset;
    desc_writes[1].dstBinding = 1;
    desc_writes[1].dstArrayElement = 0;
    desc_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    desc_writes[1].descriptorCount = image_info.size();
    desc_writes[1].pImageInfo = image_info.data();

    vkUpdateDescriptorSets(_device, static_cast<uint32_t>(desc_writes.size()), desc_writes.data(), 0, nullptr);
  }

  void VulkanAPI::updateDescriptorSet(
    Buffer& uniform_buffer,
    VkDescriptorSet& descset,
    std::vector<VkDescriptorImageInfo>& image_info,
    VkDescriptorType const type)
  {
    std::array<VkWriteDescriptorSet, 2> desc_writes{};
    std::vector<VkDescriptorBufferInfo> buffer_infos;

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = uniform_buffer.buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = VK_WHOLE_SIZE;
    buffer_infos.emplace_back(bufferInfo);

    desc_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_writes[0].dstSet = descset;
    desc_writes[0].dstBinding = 0;
    desc_writes[0].dstArrayElement = 0;
    desc_writes[0].descriptorType = type;
    desc_writes[0].descriptorCount = 1;
    desc_writes[0].pBufferInfo = buffer_infos.data();

    desc_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_writes[1].dstSet = descset;
    desc_writes[1].dstBinding = 1;
    desc_writes[1].dstArrayElement = 0;
    desc_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    desc_writes[1].descriptorCount = image_info.size();
    desc_writes[1].pImageInfo = image_info.data();

    vkUpdateDescriptorSets(_device, static_cast<uint32_t>(desc_writes.size()), desc_writes.data(), 0, nullptr);
  }

  void VulkanAPI::updateDescriptorSet(
    Buffer& uniform_buffer,
    Buffer& storage_buffer,
    VkDescriptorSet& descset,
    std::vector<VkDescriptorImageInfo>& image_info)
  {
    std::array<VkWriteDescriptorSet, 3> desc_writes{};
    std::vector<VkDescriptorBufferInfo> buffer_infos;
    std::vector<VkDescriptorBufferInfo> storage_buffer_infos;

    VkDescriptorBufferInfo buffer_info{};
    buffer_info.buffer = uniform_buffer.buffer;
    buffer_info.offset = 0;
    buffer_info.range = VK_WHOLE_SIZE;
    buffer_infos.emplace_back(buffer_info);

    VkDescriptorBufferInfo storage_buffer_info{};
    storage_buffer_info.buffer = storage_buffer.buffer;
    storage_buffer_info.offset = 0;
    storage_buffer_info.range = VK_WHOLE_SIZE;
    storage_buffer_infos.emplace_back(storage_buffer_info);

    desc_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_writes[0].dstSet = descset;
    desc_writes[0].dstBinding = 0;
    desc_writes[0].dstArrayElement = 0;
    desc_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    desc_writes[0].descriptorCount = 1;
    desc_writes[0].pBufferInfo = buffer_infos.data();

    desc_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_writes[1].dstSet = descset;
    desc_writes[1].dstBinding = 1;
    desc_writes[1].dstArrayElement = 0;
    desc_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    desc_writes[1].descriptorCount = image_info.size();
    desc_writes[1].pImageInfo = image_info.data();

    desc_writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_writes[2].dstSet = descset;
    desc_writes[2].dstBinding = 2;
    desc_writes[2].dstArrayElement = 0;
    desc_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    desc_writes[2].descriptorCount = storage_buffer_infos.size();
    desc_writes[2].pBufferInfo = storage_buffer_infos.data();

    vkUpdateDescriptorSets(_device, static_cast<uint32_t>(desc_writes.size()), desc_writes.data(), 0, nullptr);
  }

  void VulkanAPI::updateDescriptorSets(
    std::vector<Buffer>& uniform_buffers,
    std::vector<Buffer>& storage_buffers,
    VkDescriptorSet& descset,
    std::vector<VkDescriptorImageInfo>& image_info)
  {
    std::array<VkWriteDescriptorSet, 3> desc_writes{};
    std::vector<VkDescriptorBufferInfo> buffer_infos;
    std::vector<VkDescriptorBufferInfo> storage_buffer_infos;

    std::for_each(std::begin(uniform_buffers), std::end(uniform_buffers),
    [&buffer_infos](const Buffer& buffer)
    {
      VkDescriptorBufferInfo buffer_info{};
      buffer_info.buffer = buffer.buffer;
      buffer_info.offset = 0;
      buffer_info.range = VK_WHOLE_SIZE;
      buffer_infos.emplace_back(buffer_info);
    });

    std::for_each(std::begin(storage_buffers), std::end(storage_buffers),
    [&storage_buffer_infos](const Buffer& buffer)
    {
      VkDescriptorBufferInfo buffer_info{};
      buffer_info.buffer = buffer.buffer;
      buffer_info.offset = 0;
      buffer_info.range = VK_WHOLE_SIZE;
      storage_buffer_infos.emplace_back(buffer_info);
    });

    desc_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_writes[0].dstSet = descset;
    desc_writes[0].dstBinding = 0;
    desc_writes[0].dstArrayElement = 0;
    desc_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    desc_writes[0].descriptorCount = 1;
    desc_writes[0].pBufferInfo = buffer_infos.data();

    desc_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_writes[1].dstSet = descset;
    desc_writes[1].dstBinding = 1;
    desc_writes[1].dstArrayElement = 0;
    desc_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    desc_writes[1].descriptorCount = image_info.size();
    desc_writes[1].pImageInfo = image_info.data();

    desc_writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_writes[2].dstSet = descset;
    desc_writes[2].dstBinding = 2;
    desc_writes[2].dstArrayElement = 0;
    desc_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    desc_writes[2].descriptorCount = storage_buffer_infos.size();
    desc_writes[2].pBufferInfo = storage_buffer_infos.data();

    vkUpdateDescriptorSets(_device, static_cast<uint32_t>(desc_writes.size()), desc_writes.data(), 0, nullptr);
  }

  void VulkanAPI::updateStorageDescriptorSets(
    std::vector<Buffer> &uniform_buffers,
    VkDescriptorSet& desc_set,
    VkDescriptorType const type)
  {
    std::array<VkWriteDescriptorSet, 1> desc_writes{};
    std::vector<VkDescriptorBufferInfo> buffer_infos;

    std::for_each(std::begin(uniform_buffers), std::end(uniform_buffers),
    [& buffer_infos](const Buffer & uniformBuffer)
    {
      VkDescriptorBufferInfo buffer_info{};
      buffer_info.buffer = uniformBuffer.buffer;
      buffer_info.offset = 0;
      buffer_info.range = VK_WHOLE_SIZE;
      buffer_infos.emplace_back(buffer_info);
    });

    desc_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_writes[0].dstSet = desc_set;
    desc_writes[0].dstBinding = 2;
    desc_writes[0].dstArrayElement = 0;
    desc_writes[0].descriptorType = type;
    desc_writes[0].descriptorCount = buffer_infos.size();
    desc_writes[0].pBufferInfo = buffer_infos.data();

    vkUpdateDescriptorSets(_device, static_cast<uint32_t>(desc_writes.size()), desc_writes.data(), 0, nullptr);
  }

  void VulkanAPI::beginRenderPass(
    VkRenderPass& rdr_pass,
    VkCommandBuffer& cmd_buffer,
    VkFramebuffer& frame_buffer)
  {
    VkRenderPassBeginInfo rdr_pass_info{};
    rdr_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rdr_pass_info.renderPass = rdr_pass;
    rdr_pass_info.framebuffer = frame_buffer;
    rdr_pass_info.renderArea.offset = { 0, 0 };
    rdr_pass_info.renderArea.extent = _swapchain_extent;

    std::array<VkClearValue, 2> clear_value{};
    clear_value[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    clear_value[1].depthStencil = { 1.0f, 0 };

    rdr_pass_info.clearValueCount = static_cast<uint32_t>(clear_value.size());
    rdr_pass_info.pClearValues = clear_value.data();

    vkCmdBeginRenderPass(cmd_buffer, &rdr_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    //vkCmdBeginRenderPass(cmd_buffer, &rdr_pass_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
  }

  void VulkanAPI::endRenderPass(VkCommandBuffer& cmd_buffer)
  {
    vkCmdEndRenderPass(cmd_buffer);
  }

  void VulkanAPI::beginRendering(
    VkCommandBuffer& cmd_buffer,
    VkImageView& color_imageview,
    VkImageView& depth_imageview,
    VkAttachmentLoadOp const load_op,
    VkAttachmentStoreOp const store_op)
  {
    VkRenderingInfo rendering_info{ };
    rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    rendering_info.renderArea.extent = _swapchain_extent;
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;

    VkClearColorValue color_clear = {};

    color_clear.float32[0] = 0.0f;
    color_clear.float32[1] = 0.0f;
    color_clear.float32[2] = 0.0f;
    color_clear.float32[3] = 1.0f;

    VkRenderingAttachmentInfo color_attachment{ };
    color_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    color_attachment.imageView = color_imageview;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.storeOp = store_op;
    color_attachment.clearValue.color = color_clear;

    VkClearDepthStencilValue depth_stencil { 1.f, 0 };

    VkRenderingAttachmentInfo depth_attachment{ };
    depth_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depth_attachment.imageView = depth_imageview;
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.clearValue.depthStencil = depth_stencil;

    rendering_info.pColorAttachments = &color_attachment;
    rendering_info.pDepthAttachment = &depth_attachment;
    //rendering_info.pStencilAttachment;

    vkCmdBeginRendering(cmd_buffer, &rendering_info);
  }

  void VulkanAPI::endRendering(VkCommandBuffer& cmd_buffer)
  {
    vkCmdEndRendering(cmd_buffer);
  }

  void VulkanAPI::endCommandBuffer(VkCommandBuffer& cmd_buffer)
  {
    vkEndCommandBuffer(cmd_buffer);
  }

  VkResult VulkanAPI::queueSubmit(VkCommandBuffer& cmd_buffer, int const queueIndex)
  {
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd_buffer;

    VkResult result = VK_SUCCESS;
    {
      std::lock_guard<std::mutex> guard(_mutex_queue_submit);
      vkResetFences(_device, 1, &_fence_submit);

      result = vkQueueSubmit(_graphics_queues[queueIndex], 1, &submit_info, _fence_submit);
      vkWaitForFences(_device, 1, & _fence_submit, VK_TRUE, UINT32_MAX);
      vkResetCommandBuffer(cmd_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
      //vkQueueWaitIdle(_graphics_queues[queue_index]);
    }
    return result;
  }

  void VulkanAPI::resetCommandPool(VkCommandPool& cmd_pool)
  {
    vkResetCommandPool(_device, cmd_pool, 0);
  }

  void VulkanAPI::draw(
    VkCommandBuffer& cmd_buffer,
    VkDescriptorSet& descset,
    VulkanPipeline & pipeline,
    Data * data,
    bool const is_indexed,
    uint32_t const index)
  {
    VkBuffer vertex_buffers[] = { data->_vertex_buffer.buffer };
    VkDeviceSize offsets[] = { 0 };

    if (data->_vertex_buffer.size > 0) {
      vkCmdBindVertexBuffers(cmd_buffer, 0, 1, vertex_buffers, offsets);
    }
        
    if (is_indexed) {
      vkCmdBindIndexBuffer(cmd_buffer, data->_indices_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
      vkCmdDrawIndexed(cmd_buffer, data->_indices.size(), data->_ubos.size(), 0, 0, 0);
    } else {
      vkCmdDraw(cmd_buffer, data->_vertices.size(), 1, 0, index);
    }
  }

  void VulkanAPI::addPipelineBarriers(
    VkCommandBuffer& cmd_buffer,
    std::vector<VkImageMemoryBarrier>& rdr_barriers,
    VkPipelineStageFlags const src_stage_mask,
    VkPipelineStageFlags const dst_stage_mask,
    VkDependencyFlags const depency_flags)
  {
    vkCmdPipelineBarrier(
      cmd_buffer, src_stage_mask, dst_stage_mask, depency_flags, 0, nullptr, 0, nullptr,
      static_cast<uint32_t>(rdr_barriers.size()), rdr_barriers.data());
  }

  VkBuffer VulkanAPI::createBuffer(VkDeviceSize const size, VkBufferUsageFlags const usage)
  {
    VkBuffer buffer{};
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(_device, & buffer_info, nullptr, & buffer) != VK_SUCCESS) {
      throw std::runtime_error("failed to create buffer!");
    }
    return buffer;
  }

  void VulkanAPI::createBuffer(
    VkDeviceSize const size,
    VkBufferUsageFlags const usage,
    VkMemoryPropertyFlags const properties,
    VkBuffer& buffer,
    VkDeviceMemory& device_memory)
  {
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(_device, &buffer_info, nullptr, &buffer) != VK_SUCCESS) {
      throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(_device, buffer, & mem_requirements);

    uint32_t allocated_size = ((mem_requirements.size / mem_requirements.alignment) + 1) * mem_requirements.alignment;

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = allocated_size;
    alloc_info.memoryTypeIndex = findMemoryType(mem_requirements.memoryTypeBits, properties);

    if (vkAllocateMemory(_device, &alloc_info, nullptr, & device_memory) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate buffer memory!");
    }
    VkResult result = vkBindBufferMemory(_device, buffer, device_memory, 0);

    if (result != VK_SUCCESS) {
      PLP_ERROR("Memory binding failed in createBuffer");
    }
  }

  Buffer VulkanAPI::createIndexBuffer(
    VkCommandPool& cmd_pool,
    std::vector<uint32_t> const& indices)
  {
    VkDeviceSize buffer_size = sizeof(indices[0]) * indices.size();

    VkBuffer staging_buffer;
    VkDeviceMemory staging_device_memory;
    createBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
        | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_device_memory);
      
    void* data;
    vkMapMemory(_device, staging_device_memory, 0, buffer_size, 0, &data);
    memcpy(data, indices.data(), static_cast<size_t>(buffer_size));
    vkUnmapMemory(_device, staging_device_memory);

    VkBuffer buffer = createBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(_device, buffer, & mem_requirements);
    uint32_t size = ((mem_requirements.size / mem_requirements.alignment) + 1) * mem_requirements.alignment;

    auto memory_type = findMemoryType(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    auto device_memory = _device_memory_pool->get(
      _device,
      size,
      memory_type,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      mem_requirements.alignment,
      DeviceMemoryPool::DeviceBufferType::UNIFORM);

    auto offset = device_memory->getOffset();
    device_memory->bindBufferToMemory(buffer, size);

    copyBuffer(cmd_pool, staging_buffer, buffer, buffer_size);

    Buffer mesh_buffer;
    mesh_buffer.buffer = std::move(buffer);
    mesh_buffer.memory = device_memory;
    mesh_buffer.offset = offset;
    mesh_buffer.size = size;

    vkDestroyBuffer(_device, staging_buffer, nullptr);
    vkFreeMemory(_device, staging_device_memory, nullptr);

    return mesh_buffer;
  }

  Buffer VulkanAPI::createVertexBuffer(VkCommandPool& commandPool, std::vector<Vertex> const& vertices)
  {
    VkDeviceSize buffer_size = sizeof(Vertex) * vertices.size();
    VkBuffer staging_buffer{};
    VkDeviceMemory staging_device_memory{};

    createBuffer(buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_device_memory);

    void* data;
    vkMapMemory(_device, staging_device_memory, 0, buffer_size, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(buffer_size));
    vkUnmapMemory(_device, staging_device_memory);

    VkBuffer buffer = createBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(_device, buffer, & mem_requirements);

    auto memory_type = findMemoryType(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    uint32_t size = ((mem_requirements.size / mem_requirements.alignment) + 1) * mem_requirements.alignment;

    auto device_memory = _device_memory_pool->get(
      _device,
      size,
      memory_type,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      mem_requirements.alignment,
      DeviceMemoryPool::DeviceBufferType::UNIFORM);

    auto offset = device_memory->getOffset();
    device_memory->bindBufferToMemory(buffer, size);

    copyBuffer(commandPool, staging_buffer, buffer, buffer_size);

    Buffer mesh_buffer;
    mesh_buffer.buffer = std::move(buffer);
    mesh_buffer.memory = device_memory;
    mesh_buffer.offset = offset;
    mesh_buffer.size = size;

    vkDestroyBuffer(_device, staging_buffer, nullptr);
    vkFreeMemory(_device, staging_device_memory, nullptr);

    return mesh_buffer;
  }

  Buffer VulkanAPI::createVertex2DBuffer(
    VkCommandPool& cmd_pool,
    std::vector<Vertex2D> const& vertices)
  {
    VkDeviceSize buffer_size = sizeof(Vertex2D) * vertices.size();
    VkBuffer staging_buffer{};
    VkDeviceMemory staging_device_memory{};

    createBuffer(buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_device_memory);

    void* data;
    vkMapMemory(_device, staging_device_memory, 0, buffer_size, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(buffer_size));
    vkUnmapMemory(_device, staging_device_memory);

    VkBuffer buffer = createBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(_device, buffer, &mem_requirements);

    auto memory_type = findMemoryType(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    uint32_t size = ((mem_requirements.size / mem_requirements.alignment) + 1) * mem_requirements.alignment;

    auto device_memory = _device_memory_pool->get(
      _device,
      size,
      memory_type,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      mem_requirements.alignment,
      DeviceMemoryPool::DeviceBufferType::UNIFORM);

    auto offset = device_memory->getOffset();
    device_memory->bindBufferToMemory(buffer, size);

    copyBuffer(cmd_pool, staging_buffer, buffer, buffer_size);

    Buffer mesh_buffer;
    mesh_buffer.buffer = std::move(buffer);
    mesh_buffer.memory = device_memory;
    mesh_buffer.offset = offset;
    mesh_buffer.size = size;

    vkDestroyBuffer(_device, staging_buffer, nullptr);
    vkFreeMemory(_device, staging_device_memory, nullptr);

    return mesh_buffer;
  }

  Buffer VulkanAPI::createUniformBuffers(
    uint32_t const uniform_buffers_count,
    VkCommandPool& cmd_pool)
  {
    VkDeviceSize buffer_size = sizeof(UniformBufferObject) * uniform_buffers_count;
    VkBuffer buffer = createBuffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(_device, buffer, &mem_requirements);

    auto memoryType = findMemoryType(
      mem_requirements.memoryTypeBits,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    uint32_t size = ((mem_requirements.size / mem_requirements.alignment) + 1) * mem_requirements.alignment;

    auto device_memory = _device_memory_pool->get(
      _device,
      size,
      memoryType,
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      mem_requirements.alignment,
      DeviceMemoryPool::DeviceBufferType::UNIFORM);

    auto offset = device_memory->getOffset();
    device_memory->bindBufferToMemory(buffer, size);

    Buffer uniform_buffer;
    uniform_buffer.buffer = std::move(buffer);
    uniform_buffer.memory = device_memory;
    uniform_buffer.offset = offset;
    uniform_buffer.size = size;

    return uniform_buffer;
  }

  Buffer VulkanAPI::createStorageBuffers(
    ObjectBuffer const& storage_buffer,
    VkCommandPool& command_pool)
  {
    VkDeviceSize buffer_size { sizeof(storage_buffer) };

    VkBuffer staging_buffer{};
    VkDeviceMemory staging_device_memory{};

    createBuffer(buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_device_memory);

    void* data;
    vkMapMemory(_device, staging_device_memory, 0, buffer_size, 0, &data);
    memcpy(data, &storage_buffer, static_cast<size_t>(buffer_size));
    vkUnmapMemory(_device, staging_device_memory);

    VkBuffer buffer = createBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(_device, buffer, & mem_requirements);

    auto memoryType = findMemoryType(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    uint32_t size = ((mem_requirements.size / mem_requirements.alignment) + 1) * mem_requirements.alignment;

    auto const flags { VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

    auto device_memory = _device_memory_pool->get(
      _device,
      size,
      memoryType,
      flags,
      mem_requirements.alignment,
      DeviceMemoryPool::DeviceBufferType::STORAGE);

    auto offset = device_memory->getOffset();
    device_memory->bindBufferToMemory(buffer, size);

    copyBuffer(command_pool, staging_buffer, buffer, buffer_size);

    Buffer uniform_buffer;
    uniform_buffer.buffer = std::move(buffer);
    uniform_buffer.memory = device_memory;
    uniform_buffer.offset = offset;
    uniform_buffer.size = size;

    vkDestroyBuffer(_device, staging_buffer, nullptr);
    vkFreeMemory(_device, staging_device_memory, nullptr);

    return uniform_buffer;
  }

  Buffer VulkanAPI::createIndirectCommandsBuffer(std::vector<VkDrawIndexedIndirectCommand> const& draw_cmds)
  {
    VkBuffer buffer{ createBuffer(
      sizeof(VkDrawIndexedIndirectCommand) * draw_cmds.size(),
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT) };
        
    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(_device, buffer, & mem_requirements);

    auto const memory_type{ findMemoryType(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) };
    auto const size{ ((mem_requirements.size / mem_requirements.alignment) + 1) * mem_requirements.alignment };

    auto device_memory{ _device_memory_pool->get(
      _device,
      size,
      memory_type,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
      mem_requirements.alignment,
      DeviceMemoryPool::DeviceBufferType::STORAGE) };

    auto const offset {device_memory->getOffset()};
    device_memory->bindBufferToMemory(buffer, size);

    Buffer indirect_buffer{ std::move(buffer), device_memory, offset, size };

    {
      indirect_buffer.memory->lock();

      auto memory{ indirect_buffer.memory->getMemory() };
      void* data;
      vkMapMemory(_device, *memory, indirect_buffer.offset, indirect_buffer.size, 0, &data);
      memcpy(data, draw_cmds.data(), indirect_buffer.size);
      vkUnmapMemory(_device, *memory);

      indirect_buffer.memory->unLock();
    }
    return indirect_buffer;
  }

  Buffer VulkanAPI::createCubeUniformBuffers(uint32_t const uniform_buffers_count)
  {
    VkDeviceSize buffer_size = sizeof(CubeUniformBufferObject) * uniform_buffers_count;
    VkBuffer buffer = createBuffer(buffer_size, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(_device, buffer, &mem_requirements);

    uint32_t size = ((mem_requirements.size / mem_requirements.alignment) + 1) * mem_requirements.alignment;

    auto memory_type = findMemoryType(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
      | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    auto device_memory = _device_memory_pool->get(
      _device,
      size,
      memory_type,
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      mem_requirements.alignment,
      DeviceMemoryPool::DeviceBufferType::UNIFORM);

    auto offset = device_memory->getOffset();
    device_memory->bindBufferToMemory(buffer, size);

    Buffer uniform_buffer;
    uniform_buffer.buffer = std::move(buffer);
    uniform_buffer.memory = device_memory;
    uniform_buffer.offset = offset;
    uniform_buffer.size = size;

    return uniform_buffer;
  }

  void VulkanAPI::updateUniformBuffer(Buffer& buffer, std::vector<UniformBufferObject>* uniform_buffer_objects)
  {
    {
      buffer.memory->lock();

      auto memory = buffer.memory->getMemory();
      void* data;
      vkMapMemory(_device, *memory, buffer.offset, buffer.size, 0, & data);
      memcpy(data, uniform_buffer_objects->data(), buffer.size);
      vkUnmapMemory(_device, *memory);

      buffer.memory->unLock();
    }
  }

  void VulkanAPI::updateStorageBuffer(Buffer& buffer, ObjectBuffer& object_buffer)
  {
    {
      buffer.memory->lock();

      auto memory = buffer.memory->getMemory();
      void* data;
      vkMapMemory(_device, *memory, buffer.offset, buffer.size, 0, &data);
      memcpy(data, & object_buffer, buffer.size);
      vkUnmapMemory(_device, *memory);

      buffer.memory->unLock();
    }
  }

  void VulkanAPI::copyBuffer(
    VkCommandPool& cmd_pool,
    VkBuffer& src_buffer,
    VkBuffer& dst_buffer,
    VkDeviceSize const size,
    int const queue_index)
  {
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = cmd_pool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer cmd_buffer{};
    vkAllocateCommandBuffers(_device, & alloc_info, & cmd_buffer);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmd_buffer, &begin_info);

    VkBufferCopy copy_region{};
    copy_region.srcOffset = 0; // Optional
    copy_region.dstOffset = 0; // Optional
    copy_region.size = size;

    vkCmdCopyBuffer(cmd_buffer, src_buffer, dst_buffer, 1, & copy_region);
    vkEndCommandBuffer(cmd_buffer);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = & cmd_buffer;

    {
      std::lock_guard<std::mutex> guard(_mutex_queue_submit);
      vkResetFences(_device, 1, & _fence_buffer);

      vkQueueSubmit(_graphics_queues[queue_index], 1, & submit_info, _fence_buffer);
      vkWaitForFences(_device, 1, & _fence_buffer, VK_TRUE, UINT32_MAX);
      vkFreeCommandBuffers(_device, cmd_pool, 1, & cmd_buffer);
      //vkQueueWaitIdle(_graphics_queues[queue_index]);
    }
  }

  VkImageMemoryBarrier VulkanAPI::setupImageMemoryBarrier(
    VkImage& image,
    VkAccessFlags const src_access_mask,
    VkAccessFlags const dst_access_mask,
    VkImageLayout const old_layout,
    VkImageLayout const new_layout,
    uint32_t const mip_lvl,
    VkImageAspectFlags const aspect_mask)
  {
    VkImageMemoryBarrier result = { };
    result.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    result.srcAccessMask = src_access_mask;
    result.dstAccessMask = dst_access_mask;
    result.oldLayout = old_layout;
    result.newLayout = new_layout;
    result.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    result.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    result.image = image;
    result.subresourceRange.aspectMask = aspect_mask;
    //those constants don't work on android
    result.subresourceRange.levelCount = mip_lvl;
    result.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
    result.pNext = nullptr;

    return result;
  }

  uint32_t VulkanAPI::findMemoryType(uint32_t const type_filter, VkMemoryPropertyFlags const properties)
  {
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(_physical_device, &mem_properties);

    for (uint32_t i { 0 }; i < mem_properties.memoryTypeCount; i++) {
      if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
        _max_memory_heap = mem_properties.memoryHeaps[mem_properties.memoryTypes[i].heapIndex].size;
        return i;
      }
    }
    throw std::runtime_error("failed to find suitable memory type!");
  }

  void VulkanAPI::createImage(
    uint32_t const width,
    uint32_t const height,
    uint32_t const mip_lvl,
    VkSampleCountFlagBits const num_samples,
    VkFormat const format,
    VkImageTiling const tiling,
    VkImageUsageFlags const usage,
    VkMemoryPropertyFlags const properties,
    VkImage& image)
  {
    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = mip_lvl;
    image_info.arrayLayers = 1;
    image_info.format = format;
    image_info.tiling = tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage;
    image_info.samples = num_samples;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result{ VK_SUCCESS };
    result = vkCreateImage(_device, & image_info, nullptr, &image);

    if (result != VK_SUCCESS) {
      throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(_device, image, & mem_requirements);
    auto memory_type = findMemoryType(mem_requirements.memoryTypeBits, properties);
    uint32_t size = ((mem_requirements.size / mem_requirements.alignment) + 1) * mem_requirements.alignment;

    auto device_memory = _device_memory_pool->get(
      _device,
      size,
      memory_type,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      mem_requirements.alignment,
      DeviceMemoryPool::DeviceBufferType::STAGING);

    device_memory->bindImageToMemory(image, size);
  }

  void VulkanAPI::createDepthMapImage(VkImage& image)
  {
    uint32_t const width{ getSwapChainExtent().width * 8 };
    uint32_t const height{ getSwapChainExtent().height * 8 };

    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = VK_FORMAT_D24_UNORM_S8_UINT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT 
        | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        
    VkResult result = vkCreateImage(getDevice(), & image_info, nullptr, & image);

    if (result != VK_SUCCESS) {
      throw std::runtime_error("can't create image for shadow map");
    }

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(_device, image, & mem_requirements);
    auto memoryType = findMemoryType(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    uint32_t size = ((mem_requirements.size / mem_requirements.alignment) + 1) * mem_requirements.alignment;

    auto device_memory = _device_memory_pool->get(
      _device,
      size,
      memoryType,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      mem_requirements.alignment,
      DeviceMemoryPool::DeviceBufferType::STAGING);

    device_memory->bindImageToMemory(image, size);
  }

  VkImageView VulkanAPI::createDepthMapImageView(VkImage& image)
  {
    VkImageView depth_map_imageview{};

    VkImageViewCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = image;
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = VK_FORMAT_D24_UNORM_S8_UINT;
    create_info.subresourceRange = {};
    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;

    VkResult result{ VK_SUCCESS };

    result = vkCreateImageView(_device, &create_info, nullptr, &depth_map_imageview);

    if (result != VK_SUCCESS) {
      PLP_ERROR("failed to create depth map image view.");
    }
    return depth_map_imageview;
  }

  VkSampler VulkanAPI::createDepthMapSampler()
  {
    VkSampler sampler{};

    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_NEAREST;
    sampler_info.minFilter = VK_FILTER_NEAREST;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeV = sampler_info.addressModeU;
    sampler_info.addressModeW = sampler_info.addressModeU;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.maxAnisotropy = 1.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 1.0f;
    sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    if (vkCreateSampler(_device, &sampler_info, nullptr, &sampler) != VK_SUCCESS) {
      throw std::runtime_error("failed to create depth map sampler!");
    }
    return sampler;
  }

  void VulkanAPI::createDepthMapFrameBuffer(
    VkRenderPass& rdr_pass,
    VkImageView& imageview,
    VkFramebuffer& frame_buffer)
  {
    VkFramebufferCreateInfo frame_buffer_info{};
    frame_buffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frame_buffer_info.renderPass = rdr_pass;
    frame_buffer_info.attachmentCount = 1;
    frame_buffer_info.pAttachments = &imageview;
    frame_buffer_info.width = _swapchain_extent.width;
    frame_buffer_info.height = _swapchain_extent.height;
    frame_buffer_info.layers = 1;

    VkResult result = vkCreateFramebuffer(_device, &frame_buffer_info, nullptr, & frame_buffer);

    if (result != VK_SUCCESS) {
      PLP_ERROR("failed to create framebuffer for depth map");
    }
  }

  void VulkanAPI::createSkyboxImage(
    uint32_t const width,
    uint32_t const height,
    VkSampleCountFlagBits const num_samples,
    VkFormat const format,
    VkImageTiling const tiling,
    VkImageUsageFlags const usage,
    VkMemoryPropertyFlags const properties,
    VkImage& image)
  {
    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 6;
    image_info.format = format;
    image_info.tiling = tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage;
    image_info.samples = num_samples;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    if (vkCreateImage(_device, &image_info, nullptr, & image) != VK_SUCCESS) {
      throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(_device, image, & mem_requirements);
    auto memoryType = findMemoryType(mem_requirements.memoryTypeBits, properties);
    uint32_t size = ((mem_requirements.size / mem_requirements.alignment) + 1) * mem_requirements.alignment;

    auto device_memory = _device_memory_pool->get(
      _device,
      size,
      memoryType,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      mem_requirements.alignment,
      DeviceMemoryPool::DeviceBufferType::STAGING);

    device_memory->bindImageToMemory(image, size);
  }

  void VulkanAPI::createTextureImage(
    VkCommandBuffer& cmd_buffer,
    stbi_uc* pixels,
    uint32_t const tex_width,
    uint32_t const tex_height,
    uint32_t const mip_lvl,
    VkImage& texture_image,
    VkFormat const format)
  {
    VkDeviceSize image_size = tex_width * tex_height * 4;
    VkBuffer buffer = createBuffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(_device, buffer, & mem_requirements);

    auto memory_type = findMemoryType(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
      | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    uint32_t size = ((mem_requirements.size / mem_requirements.alignment) + 1) * mem_requirements.alignment;

    auto device_memory = _device_memory_pool->get(
      _device,
      size,
      memory_type,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      mem_requirements.alignment,
      DeviceMemoryPool::DeviceBufferType::STAGING);

    auto offset = device_memory->getOffset();
    device_memory->bindBufferToMemory(buffer, size);

    void* data;
    vkMapMemory(_device, *device_memory->getMemory(), offset, size, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(size));
    vkUnmapMemory(_device, *device_memory->getMemory());

    stbi_image_free(pixels);

    createImage(tex_width, tex_height, mip_lvl, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture_image);

    VkImageMemoryBarrier rdr_barrier = setupImageMemoryBarrier(
      texture_image, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    std::vector<VkImageMemoryBarrier> barrier{ rdr_barrier };

    addPipelineBarriers(cmd_buffer, barrier, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
      VK_DEPENDENCY_BY_REGION_BIT);

    copyBufferToImage(cmd_buffer, buffer, texture_image, static_cast<uint32_t>(tex_width),
      static_cast<uint32_t>(tex_height), mip_lvl);

    generateMipmaps(cmd_buffer, format, texture_image, tex_width, tex_height, mip_lvl);
    endCommandBuffer(cmd_buffer);
    queueSubmit(cmd_buffer);
    //_device_memory_pool->clear(device_memory);
  }

  void VulkanAPI::createSkyboxTextureImage(
    VkCommandBuffer& cmd_buffer,
    std::vector<stbi_uc*>& pixels,
    uint32_t const tex_width,
    uint32_t const tex_height,
    uint32_t const mip_lvl,
    VkImage& texture_image,
    VkFormat const format)
  {
    VkDeviceSize image_size = tex_width * tex_height * 4 * 6;
    VkBuffer buffer = createBuffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(_device, buffer, & mem_requirements);

    auto memory_type = findMemoryType(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    uint32_t size = ((mem_requirements.size / mem_requirements.alignment) + 1) * mem_requirements.alignment;

    auto device_memory = _device_memory_pool->get(
      _device,
      size,
      memory_type, 
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      mem_requirements.alignment,
      DeviceMemoryPool::DeviceBufferType::STAGING,
      true);

    auto offset = device_memory->getOffset();
    device_memory->bindBufferToMemory(buffer, size);

    stbi_uc* data;
    VkDeviceSize layerSize = image_size / 6;
    vkMapMemory(_device, *device_memory->getMemory(), offset, size, 0, (void**)&data);

    for (uint32_t i { 0 }; i < pixels.size(); i++) {
      memcpy(data + layerSize * i, pixels[i], layerSize);
    }

    vkUnmapMemory(_device, *device_memory->getMemory());
    createSkyboxImage(tex_width, tex_height, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture_image);

    VkImageMemoryBarrier rdr_barrier = setupImageMemoryBarrier(texture_image, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    rdr_barrier.subresourceRange.layerCount = 6;
    rdr_barrier.subresourceRange.baseMipLevel = 0;
    rdr_barrier.subresourceRange.levelCount = mip_lvl;

    std::vector<VkImageMemoryBarrier> barrier{ rdr_barrier };
    addPipelineBarriers(cmd_buffer, barrier, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT);

    copyBufferToImageSkybox(cmd_buffer, buffer, texture_image, tex_width, tex_height, pixels, mip_lvl,
        image_size / 6);

    generateMipmaps(cmd_buffer, format, texture_image, tex_width, tex_height, mip_lvl, 6);
    endCommandBuffer(cmd_buffer);
    queueSubmit(cmd_buffer);

    for (uint32_t i { 0 }; i < pixels.size(); i++) {
      stbi_image_free(pixels[i]);
    }
    //_device_memory_pool->clear(device_memory);
  }

  void VulkanAPI::generateMipmaps(
    VkCommandBuffer& cmd_buffer,
    VkFormat const image_format,
    VkImage& image,
    uint32_t const tex_width,
    uint32_t const tex_height,
    uint32_t const mip_lvl,
    uint32_t const layer_count) {

    VkFormatProperties format_props;
    vkGetPhysicalDeviceFormatProperties(_physical_device, image_format, & format_props);

    if (!(format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
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

    auto mip_width { static_cast<int32_t>(tex_width)};
    auto mip_height{ static_cast<int32_t>(tex_height) };

    for (uint32_t i{ 1 }; i < mip_lvl; i++) {

      barrier.subresourceRange.baseMipLevel = i - 1;
      barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

      vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
        0, nullptr, 0, nullptr, 1, & barrier);

      VkImageBlit blit{};
      blit.srcOffsets[0] = { 0, 0, 0 };
      blit.srcOffsets[1] = { mip_width, mip_height, 1 };
      blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      blit.srcSubresource.mipLevel = i - 1;
      blit.srcSubresource.baseArrayLayer = 0;
      blit.srcSubresource.layerCount = layer_count;
      blit.dstOffsets[0] = { 0, 0, 0 };
      blit.dstOffsets[1] = { mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1 };
      blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      blit.dstSubresource.mipLevel = i;
      blit.dstSubresource.baseArrayLayer = 0;
      blit.dstSubresource.layerCount = layer_count;

      vkCmdBlitImage(cmd_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, & blit, VK_FILTER_LINEAR);

      barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
      barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

      vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, nullptr, 0, nullptr, 1, & barrier);

      if (mip_width > 1) mip_width /= 2;
      if (mip_height > 1) mip_height /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mip_lvl - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
      0, nullptr, 0, nullptr, 1, & barrier);
  }

  void VulkanAPI::copyBufferToImage(
    VkCommandBuffer& cmd_buffer,
    VkBuffer& buffer,
    VkImage & image,
    uint32_t const width,
    uint32_t const height,
    uint32_t const mip_lvl)
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

    vkCmdCopyBufferToImage(cmd_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, & region);
  }

  void VulkanAPI::copyBufferToImageSkybox(
    VkCommandBuffer& cmd_buffer,
    VkBuffer& buffer,
    VkImage & image,
    uint32_t const width,
    uint32_t const height,
    std::vector<stbi_uc*>& pixels,
    uint32_t const mip_lvl,
    uint32_t const layer_size)
  {
    std::vector<VkBufferImageCopy> buffer_copy_regions;

    for (uint32_t i { 0 }; i < pixels.size(); i++) {
      for (uint32_t mipLevel = 0; mipLevel < mip_lvl; mipLevel++) {
        VkBufferImageCopy region{};
        region.bufferOffset = layer_size * i;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = mipLevel;
        region.imageSubresource.baseArrayLayer = i;
        region.imageSubresource.layerCount = 1;
        region.imageExtent.width =  width >> mipLevel;
        region.imageExtent.height = height >> mipLevel;
        region.imageExtent.depth = 1;

        buffer_copy_regions.emplace_back(region);
      }
    }
    vkCmdCopyBufferToImage(cmd_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      static_cast<uint32_t>(buffer_copy_regions.size()), buffer_copy_regions.data());
  }

  VkImageView VulkanAPI::createDepthResources(VkCommandBuffer& cmd_buffer)
  {
    VkImage depth_image{};
    VkImageView depth_imageview{};
    VkFormat depth_format = findDepthFormat();

    createImage(_swapchain_extent.width, _swapchain_extent.height, 1, VK_SAMPLE_COUNT_1_BIT, depth_format,
      VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      depth_image);

    depth_imageview = createImageView(depth_image, depth_format, 1);

    VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

    if (hasStencilComponent(depth_format)) {
      aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    VkImageMemoryBarrier rdr_barrier = setupImageMemoryBarrier(depth_image, 0,
      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, aspect_mask);

    std::vector<VkImageMemoryBarrier> barrier{ rdr_barrier };

    addPipelineBarriers(cmd_buffer, barrier, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_DEPENDENCY_BY_REGION_BIT);

    return depth_imageview;
  }

  VkSampler VulkanAPI::createTextureSampler(uint32_t const mip_lvl)
  {
    VkSampler texture_sampler{};

    VkSamplerCreateInfo sample_info{};
    sample_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sample_info.magFilter = VK_FILTER_LINEAR;
    sample_info.minFilter = VK_FILTER_LINEAR;
    sample_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sample_info.addressModeV = sample_info.addressModeU;
    sample_info.addressModeW = sample_info.addressModeU;
    sample_info.maxAnisotropy = _device_props.limits.maxSamplerAnisotropy;
    sample_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sample_info.unnormalizedCoordinates = VK_FALSE;
    sample_info.compareEnable = VK_FALSE;
    sample_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sample_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sample_info.minLod = 0.0f;
    sample_info.maxLod = static_cast<float>(mip_lvl);
    sample_info.mipLodBias = 0.0f;
    sample_info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;

    if (vkCreateSampler(_device, & sample_info, nullptr, & texture_sampler) != VK_SUCCESS) {
      throw std::runtime_error("failed to create texture sampler!");
    }
    return texture_sampler;
  }

  VkSampler VulkanAPI::createSkyboxTextureSampler(uint32_t const mip_lvl)
  {
    VkSampler texture_sampler{};

    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_NEAREST;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    sampler_info.addressModeV = sampler_info.addressModeU;
    sampler_info.addressModeW = sampler_info.addressModeU;
    sampler_info.anisotropyEnable = VK_FALSE;
    sampler_info.maxAnisotropy = 1.0f;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_NEVER;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = static_cast<float>(mip_lvl);
    sampler_info.mipLodBias = 0.0f;
    sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    if (vkCreateSampler(_device, &sampler_info, nullptr, & texture_sampler) != VK_SUCCESS) {
      throw std::runtime_error("failed to create texture sampler!");
    }
    return texture_sampler;
  }

  VkFormat VulkanAPI::findDepthFormat()
  {
    return findSupportedFormat({ VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
      VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
  }

  VkFormat VulkanAPI::findSupportedFormat(
    std::vector<VkFormat>const& candidates,
    VkImageTiling const tiling,
    VkFormatFeatureFlags const features)
  {
    for (VkFormat format : candidates) {
      VkFormatProperties props;
      vkGetPhysicalDeviceFormatProperties(_physical_device, format, & props);

      if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
        return format;
      } else if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
        return format;
      }
    }
    throw std::runtime_error("failed to find supported format!");
  }

  bool VulkanAPI::hasStencilComponent(VkFormat const format)
  {
    return format == VK_FORMAT_D24_UNORM_S8_UINT || VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
  }

  VkSampleCountFlagBits VulkanAPI::getMaxUsableSampleCount()
  {
    VkPhysicalDeviceProperties physical_device_props;
    vkGetPhysicalDeviceProperties(_physical_device, & physical_device_props);

    VkSampleCountFlags counts = physical_device_props.limits.framebufferColorSampleCounts
      & physical_device_props.limits.framebufferDepthSampleCounts;

    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
    return VK_SAMPLE_COUNT_1_BIT;
  }

  VulkanAPI::~VulkanAPI()
  {
      PLP_TRACE("VulkanAPI deleted.");
  }

  void VulkanAPI::startMarker(
    VkCommandBuffer& buffer,
    std::string const& name,
    float const r,
    float const g,
    float const b,
    float const a)
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

  void VulkanAPI::endMarker(VkCommandBuffer& buffer)
  {
#ifdef PLP_DEBUG_BUILD
    vkCmdEndDebugUtilsLabelEXT(buffer);
#endif
  }

  void VulkanAPI::waitIdle()
  {
    vkQueueWaitIdle(_present_queues[0]);
    vkDeviceWaitIdle(_device);
  }

  void VulkanAPI::setResolution(unsigned int const width, unsigned int const height)
  {
    _width = width;
    _height = height;
  }

  void VulkanAPI::transitionImageLayout(
    VkCommandBuffer& cmd_buffer,
    VkImage& image,
    VkImageLayout const old_layout,
    VkImageLayout const new_layout,
    VkImageAspectFlags const aspect_flags)
  {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspect_flags;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

      source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    
    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

      source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    
    } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

      source_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      destination_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
   
    } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

      source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      destination_stage = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
      barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      barrier.dstAccessMask = 0;

      source_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      destination_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    
    } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

      source_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
      destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    
    } else if (old_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
      barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

      source_stage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
      destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    } else {
      throw std::invalid_argument("unsupported layout transition");
    }
    
    vkCmdPipelineBarrier(
      cmd_buffer,
      source_stage, destination_stage,
      0,
      0, nullptr,
      0, nullptr,
      1, &barrier
    );
  }

  void VulkanAPI::submit(
    VkQueue& queue,
    std::vector<VkSubmitInfo> const& submit_infos,
    VkPresentInfoKHR const& present_info,
    VkFence& fence)
  {
    {
      //SCOPED_TIMER();

      std::lock_guard<std::mutex> guard(_mutex_queue_submit);
      VkResult result = vkQueueSubmit(queue, submit_infos.size(), submit_infos.data(), fence);

      if (result != VK_SUCCESS) {
        PLP_ERROR("Error on queue submit: {}", result);
        throw std::runtime_error("Error on queueSubmit");
      }

      result = vkQueuePresentKHR(queue, &present_info);

      if (result != VK_SUCCESS) {
        PLP_ERROR("Error on queue present: {}", result);
      }

      //vkQueueWaitIdle(queue);
      //vkDeviceWaitIdle(_device);
    }
  }
}
