#include "VulkanRenderer.h"
#include <chrono>

namespace Rebulk {

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
			Rebulk::Log::GetLogger()->critical(pCallbackData->pMessage);
			break;
		}
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		{
			Rebulk::Log::GetLogger()->warn(pCallbackData->pMessage);
			break;
		}
		default:
		{
			Rebulk::Log::GetLogger()->trace(pCallbackData->pMessage);
		}
		}

		return VK_FALSE;
	}

	static std::vector<char> ReadFile(const std::string& filename) 
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	VulkanRenderer::VulkanRenderer(GLFWwindow* window) : m_Window(window)
	{
#ifdef NDEBUG
		m_EnableValidationLayers = false;
#else
		m_EnableValidationLayers = true;
#endif
		
		EnumerateExtensions();
		LoadRequiredExtensions();
		CreateInstance();
		SetupDebugMessenger();
		CreateSurface();
		PickPhysicalDevice();
		CreateLogicalDevice();
	}

	void VulkanRenderer::Init()
	{
		//EnumerateExtensions();
		//LoadRequiredExtensions();
		//CreateInstance();
		//SetupDebugMessenger();
		//CreateSurface();
		//PickPhysicalDevice();
		//CreateLogicalDevice();
		//m_SwapChain = CreateSwapChain();
		//m_SwapChainImageViews = CreateImageViews();
		//m_RenderPass = CreateRenderPass();
		//m_CommandPool = CreateCommandPool();
		//VkCommandBuffer commandBuffer = CreateCommandBuffer(m_CommandPool);

		//std::pair<VkPipeline, VkPipelineLayout>pipeline = CreateGraphicsPipeline(m_RenderPass, commandBuffer);
		//m_GraphicsPipeline = pipeline.first;
		//m_PipelineLayout = pipeline.second;

		//m_SwapChainFramebuffers = CreateFramebuffers(m_RenderPass, m_SwapChainImageViews);
		//m_DescriptorPool = CreateDescriptorPool();
		//m_CommandBuffers = CreateCommandBuffers(m_RenderPass, m_CommandPool, pipeline, m_SwapChainFramebuffers);
		//std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> semaphores = CreateSyncObjects();
		//m_ImageAvailableSemaphores = semaphores.first;
		//m_RenderFinishedSemaphores = semaphores.second;
	}

	bool VulkanRenderer::DrawFrame(VkSwapchainKHR swapChain, std::vector<VkCommandBuffer> commandBuffers, std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> semaphores)
	{
		std::vector<VkSemaphore> imageAvailableSemaphores = semaphores.first;
		std::vector<VkSemaphore> renderFinishedSemaphores = semaphores.second;

		vkWaitForFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(m_Device, swapChain, UINT64_MAX, imageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			//out of date
			return true;
		} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

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
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[m_CurrentFrame] };

		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame]);

		if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FramebufferResized) {
			m_FramebufferResized = false;
			//out of date
			return true;
		} else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		m_CurrentFrame = (m_CurrentFrame + 1) % m_MAX_FRAMES_IN_FLIGHT;

		//not out of date
		return false;
	}

	void VulkanRenderer::CreateInstance()
	{
		std::string message;

		if (!IsValidationLayersEnabled() && !CheckValidationLayerSupport()) {
			m_Messages.emplace_back("Validations layers not available !");
			Notify();
			return;
		}

		VkApplicationInfo appInfo{};

		appInfo.apiVersion = VK_API_VERSION_1_2;
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Rebulkan";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(m_RequiredExtensions.size());;
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
			message = std::string("Validations enabled");
		}
		else {
			message = std::string("Validations disabled");
			createInfo.enabledLayerCount = 0;
		}

		m_Messages.emplace_back(message);
		Notify();

		VkResult result = VK_SUCCESS;

		result = vkCreateInstance(&createInfo, nullptr, &m_Instance);

		if (VK_SUCCESS != result) {
			std::string message = std::string("Can't create VK instance : " + std::to_string(result));
			Rebulk::Log::GetLogger()->critical("{}", message.c_str());
			m_Messages.emplace_back(message);
			Notify();
			return;
		}

		m_InstanceCreated = true;

		message = std::string("VK instance created successfully");
		Rebulk::Log::GetLogger()->trace(message);
		m_Messages.emplace_back(message);

		Notify();
	}

	void VulkanRenderer::EnumerateExtensions()
	{
		vkEnumerateInstanceExtensionProperties(nullptr, &m_ExtensionCount, nullptr);

		std::vector<VkExtensionProperties> extensions(m_ExtensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &m_ExtensionCount, extensions.data());

		Rebulk::Log::GetLogger()->trace("{} available extensions:\n", m_ExtensionCount);

		for (const auto& extension : extensions) {
			std::string message = std::string("extension available : ") + extension.extensionName;
			Rebulk::Log::GetLogger()->trace("\t {} \n", message.c_str());
			m_Messages.emplace_back(message);
		}

		m_Extensions = extensions;
		Notify();
	}

	bool VulkanRenderer::CheckValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : m_ValidationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
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

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (m_EnableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}

		for_each(extensions.begin(), extensions.end(), [this](const char* text) { 
			std::string message = std::string("required extension loaded : ") + text;
			m_Messages.emplace_back(message);
		});
		Notify();

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
			m_Messages.emplace_back("Can't create debug messenger.");

		} else {
			m_Messages.emplace_back("Debug messenger created");
		}

		Notify();
	}

	void VulkanRenderer::PickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			m_Messages.emplace_back("failed to find GPUs with Vulkan support!");
			return;
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

		for (const auto& device : devices) {
			if (IsDeviceSuitable(device)) {
				m_PhysicalDevice = device;

				VkPhysicalDeviceProperties deviceProperties;
				VkPhysicalDeviceFeatures deviceFeatures;

				vkGetPhysicalDeviceProperties(device, &deviceProperties);
				vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

				m_DeviceProps = deviceProperties;
				m_DeviceFeatures = deviceFeatures;

				break;
			}
		}

		if (m_PhysicalDevice == VK_NULL_HANDLE) {
			m_Messages.emplace_back("failed to find a suitable GPU");
			return;
		}

		Notify();
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

		int i = 0;
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

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(m_DeviceExtensions.size());
		createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

		if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS) {
			m_Messages.emplace_back("failed to create logical device!");
			Notify();
			return;
		} 
		
		vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &m_PresentQueue);

		m_Messages.emplace_back("successfully create logical device!");
		
		Notify();
	}

	void VulkanRenderer::CreateSurface()
	{
		VkResult result = glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &m_Surface);

		if (result != VK_SUCCESS) {
			m_Messages.emplace_back("failed to create window surface!");
			Notify();
			return;
		}

		m_Messages.emplace_back("successfully create window surface!");
		Notify();
	}

	SwapChainSupportDetails VulkanRenderer::QuerySwapChainSupport(VkPhysicalDevice device) 
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
		glfwGetFramebufferSize(m_Window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}

	VkSwapchainKHR VulkanRenderer::CreateSwapChain()
	{
		VkSwapchainKHR swapChain = VK_NULL_HANDLE;

		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_PhysicalDevice);

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_Surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;//1 unless stereoscopic app
		//use of VK_IMAGE_USAGE_TRANSFER_DST_BIT if post process is needed
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

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
			m_Messages.emplace_back("Swap chain failed " + std::to_string(result));
		} else {
			m_Messages.emplace_back("Swap chain created successfully");
		}

		vkGetSwapchainImagesKHR(m_Device, swapChain, &imageCount, nullptr);
		m_SwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_Device, swapChain, &imageCount, m_SwapChainImages.data());
		m_SwapChainImageFormat = surfaceFormat.format;
		m_SwapChainExtent = extent;

		Notify();

		return swapChain;
	}

	std::vector<VkImageView> VulkanRenderer::CreateImageViews()
	{
		std::vector<VkImageView> swapChainImageViews = {};

		swapChainImageViews.resize(m_SwapChainImages.size());

		for (size_t i = 0; i < m_SwapChainImages.size(); i++) {
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_SwapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_SwapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			VkResult result;

			result = vkCreateImageView(m_Device, &createInfo, nullptr, &swapChainImageViews[i]);
			
			if (result != VK_SUCCESS) {
				m_Messages.emplace_back("failed to create image views!");
			} else {
				m_Messages.emplace_back("created image views sucessfully");
			}

			Notify();
		}

		return swapChainImageViews;
	}

	std::pair<VkPipeline, VkPipelineLayout> VulkanRenderer::CreateGraphicsPipeline(VkRenderPass renderPass, VkCommandBuffer commandBuffer, VkDescriptorSetLayout descriptorSetLayout)
	{
		std::pair<VkPipeline, VkPipelineLayout>pipeline = {};

		VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;

		auto vertShaderCode = ReadFile("shaders/spv/vert.spv");
		auto fragShaderCode = ReadFile("shaders/spv/frag.spv");

		VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);
		
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr; 
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)m_SwapChainExtent.width;
		viewport.height = (float)m_SwapChainExtent.height;
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
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;  //VK_POLYGON_MODE_LINE VK_POLYGON_MODE_POINT
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f;
		rasterizer.depthBiasSlopeFactor = 0.0f;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr; 
		multisampling.alphaToCoverageEnable = VK_FALSE; 
		multisampling.alphaToOneEnable = VK_FALSE;

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
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pushConstantRangeCount = 0; 
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

		VkResult result = vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &graphicsPipelineLayout);

		if (result != VK_SUCCESS) {
			m_Messages.emplace_back("failed to create pipeline layout!");
		} else {
			m_Messages.emplace_back("create successfully pipeline layout!");
		}

		VkDynamicState dynamic_states[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamic_state = {};
		dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state.dynamicStateCount = 2;
		dynamic_state.pDynamicStates = dynamic_states;

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; 
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = graphicsPipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;
		pipelineInfo.pDynamicState = &dynamic_state;

		VkPipeline graphicsPipeline;

		result = vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
		
		if (result != VK_SUCCESS) {
			m_Messages.emplace_back("failed to create graphics pipeline!");
		} else {
			m_Messages.emplace_back("created successfully graphics pipeline!");
		}

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissors[1] = {};
		scissors[0].extent.width = m_SwapChainExtent.width;
		scissors[0].extent.height = m_SwapChainExtent.height;

		vkCmdSetScissor(commandBuffer, 0, 1, scissors);

		vkDestroyShaderModule(m_Device, fragShaderModule, nullptr);
		vkDestroyShaderModule(m_Device, vertShaderModule, nullptr);

		Notify();
		
		pipeline.first = graphicsPipeline;
		pipeline.second = graphicsPipelineLayout;

		return pipeline;
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
			m_Messages.emplace_back("failed to create shader module!");
		} else {
			m_Messages.emplace_back("created shader module successfully !");
		}

		Notify();

		return shaderModule;
	}

	VkRenderPass VulkanRenderer::CreateRenderPass()
	{
		VkRenderPass renderPass;

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_SwapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VkResult result = vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &renderPass);

		if (result != VK_SUCCESS) {
			m_Messages.emplace_back("failed to create render pass!");
		} else {
			m_Messages.emplace_back("created successfully render pass!");
		}

		Notify();

		return renderPass;
	}

	std::vector<VkFramebuffer> VulkanRenderer::CreateFramebuffers(VkRenderPass renderPass, std::vector<VkImageView> swapChainImageViews)
	{
		std::vector<VkFramebuffer> swapChainFramebuffers;

		swapChainFramebuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); i++) {

			VkImageView attachments[] = { swapChainImageViews[i] };

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = m_SwapChainExtent.width;
			framebufferInfo.height = m_SwapChainExtent.height;
			framebufferInfo.layers = 1;

			VkResult result = vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]);

			if (result != VK_SUCCESS) {
				m_Messages.emplace_back("failed to create framebuffer!");
			}
			else {
				m_Messages.emplace_back("created successfully framebuffer!");
			}

			Notify();
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
		poolInfo.flags = 0;

		VkResult result = vkCreateCommandPool(m_Device, &poolInfo, nullptr, &commandPool);
		
		if (result != VK_SUCCESS) {
			m_Messages.emplace_back("failed to create command pool!");
		} else {
			m_Messages.emplace_back("created successfully command pool!");
		}

		Notify();

		return commandPool;
	}

	std::vector<VkCommandBuffer> VulkanRenderer::CreateCommandBuffers(VkRenderPass renderPass, VkCommandPool commandPool, std::pair<VkPipeline, VkPipelineLayout>pipeline, std::vector<VkFramebuffer> swapChainFramebuffers)
	{
		std::vector<VkCommandBuffer> commandBuffers;

		commandBuffers.resize(swapChainFramebuffers.size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		VkResult result = vkAllocateCommandBuffers(m_Device, &allocInfo, commandBuffers.data());
		
		if (result  != VK_SUCCESS) {
			m_Messages.emplace_back("failed to allocate command buffers!");
		} else {
			m_Messages.emplace_back("allocated successfully command buffers!");
		}

		for (size_t i = 0; i < commandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0;
			beginInfo.pInheritanceInfo = nullptr;

			result = vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

			if (result != VK_SUCCESS) {
				m_Messages.emplace_back("failed to begin recording command buffer!");
			} else {
				m_Messages.emplace_back("begin recording command buffer!");
			}

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = swapChainFramebuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = m_SwapChainExtent;

			VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.first);

			VkViewport viewport;
			viewport.x = 0;
			viewport.y = 0;
			viewport.width = (float)m_SwapChainExtent.width;
			viewport.height = (float)m_SwapChainExtent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

			VkRect2D scissor = { { 0, 0 }, { (uint32_t)m_SwapChainExtent.width, (uint32_t)m_SwapChainExtent.height } };
			vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

			vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
			vkCmdEndRenderPass(commandBuffers[i]);

			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				m_Messages.emplace_back("failed to record command buffer!");
			}
		}

		Notify();

		return commandBuffers;
	}

	std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> VulkanRenderer::CreateSyncObjects()
	{
		std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> semaphores;

		std::vector<VkSemaphore> imageAvailableSemaphores = {};
		std::vector<VkSemaphore> renderFinishedSemaphores = {};

		imageAvailableSemaphores.resize(m_MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(m_MAX_FRAMES_IN_FLIGHT);
		m_InFlightFences.resize(m_MAX_FRAMES_IN_FLIGHT);
		m_ImagesInFlight.resize(m_SwapChainImages.size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < m_MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(m_Device, &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS) {
				m_Messages.emplace_back("failed to create semaphores!");
			}
		}

		Notify();

		semaphores.first = imageAvailableSemaphores;
		semaphores.second = renderFinishedSemaphores;

		return semaphores;
	}
	
	void VulkanRenderer::RecreateSwapChain() 
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(m_Window, &width, &height);

		while (width == 0 || height == 0) {
			glfwWaitEvents();
			glfwGetFramebufferSize(m_Window, &width, &height);
		}

		vkDeviceWaitIdle(m_Device);

		m_SwapChain = CreateSwapChain();
		m_SwapChainImageViews = CreateImageViews();
		m_RenderPass = CreateRenderPass();
		m_CommandPool = CreateCommandPool();
		VkCommandBuffer commandBuffer = CreateCommandBuffer(m_CommandPool);
		VkDescriptorSetLayout descriptorSetLayout = CreateDescriptorSetLayout();

		std::pair<VkPipeline, VkPipelineLayout>pipeline = CreateGraphicsPipeline(m_RenderPass, commandBuffer, descriptorSetLayout);
		m_GraphicsPipeline = pipeline.first;
		m_PipelineLayout = pipeline.second;

		m_SwapChainFramebuffers = CreateFramebuffers(m_RenderPass, m_SwapChainImageViews);
		m_DescriptorPool = CreateDescriptorPool();
		m_CommandBuffers = CreateCommandBuffers(m_RenderPass, m_CommandPool, pipeline, m_SwapChainFramebuffers);
	}

	VkDescriptorPool VulkanRenderer::CreateDescriptorPool()
	{
		VkDescriptorPool descriptorPool;

		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(m_SwapChainImages.size());
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(m_SwapChainImages.size());

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(m_SwapChainImages.size());

		if (vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool");
		}

		return descriptorPool;
	}

	VkDescriptorSetLayout VulkanRenderer::CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayout descriptorSetLayout;

		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		return descriptorSetLayout;
	}

	std::vector<VkDescriptorSet> VulkanRenderer::CreateDescriptorSets(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout)
	{
		std::vector<VkDescriptorSet> descriptorSets;
		std::vector<VkDescriptorSetLayout> layouts(m_SwapChainImages.size(), descriptorSetLayout);

		//VkDescriptorSetAllocateInfo allocInfo{};
		//allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		//allocInfo.descriptorPool = descriptorPool;
		//allocInfo.descriptorSetCount = static_cast<uint32_t>(m_SwapChainImages.size());
		//allocInfo.pSetLayouts = layouts.data();

		//descriptorSets.resize(m_SwapChainImages.size());
		//if (vkAllocateDescriptorSets(m_Device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
		//{
		//	throw std::runtime_error("failed to allocate descriptor sets");
		//}

		//for (size_t i = 0; i < m_SwapChainImages.size(); i++)
		//{
		//	VkDescriptorBufferInfo bufferInfo{};
		//	bufferInfo.buffer = uniformBuffers[i];
		//	bufferInfo.offset = 0;
		//	bufferInfo.range = sizeof(UniformBufferObject);

		//	VkDescriptorImageInfo imageInfo{};
		//	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		//	imageInfo.imageView = textureImageView;
		//	imageInfo.sampler = textureSampler;

		//	std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		//	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//	descriptorWrites[0].dstSet = descriptorSets[i];
		//	descriptorWrites[0].dstBinding = 0;
		//	descriptorWrites[0].dstArrayElement = 0;
		//	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		//	descriptorWrites[0].descriptorCount = 1;
		//	descriptorWrites[0].pBufferInfo = &bufferInfo;

		//	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//	descriptorWrites[1].dstSet = descriptorSets[i];
		//	descriptorWrites[1].dstBinding = 1;
		//	descriptorWrites[1].dstArrayElement = 0;
		//	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		//	descriptorWrites[1].descriptorCount = 1;
		//	descriptorWrites[1].pImageInfo = &imageInfo;

		//	vkUpdateDescriptorSets(m_Device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		//}

		return descriptorSets;
	}

	VkCommandBuffer VulkanRenderer::CreateCommandBuffer(VkCommandPool commandPool)
	{
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

		return commandBuffer;
	}

	void VulkanRenderer::BeginRenderPass(VkRenderPass renderPass, VkCommandBuffer commandBuffer, std::vector<VkFramebuffer> swapChainFramebuffers)
	{
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[1];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_SwapChainExtent;

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VulkanRenderer::EndRenderPass(VkCommandBuffer commandBuffer, VkCommandPool commandPool)
	{	
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkCmdEndRenderPass(commandBuffer);
		VkResult result = vkEndCommandBuffer(commandBuffer);

		std::cout << "EndRenderPass vkEndCommandBuffer result " << std::to_string(result) << std::endl;

		 result = vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		std::cout << "EndRenderPass vkQueueSubmit result " << std::to_string(result) << std::endl;
		
		result = vkQueueWaitIdle(m_GraphicsQueue);
		std::cout << "EndRenderPass vkQueueWaitIdle result " << std::to_string(result) << std::endl;

		vkFreeCommandBuffers(m_Device, commandPool, 1, &commandBuffer);
	}

	bool VulkanRenderer::Draw(VkCommandBuffer commandBuffer, VkSwapchainKHR swapChain)
	{
		vkWaitForFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(m_Device, swapChain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			//out of date
			return true;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		if (m_ImagesInFlight[imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(m_Device, 1, &m_ImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}

		m_ImagesInFlight[imageIndex] = m_ImagesInFlight[m_CurrentFrame];

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 0;
		submitInfo.pCommandBuffers = &commandBuffer;

		VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };

		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame]);

		if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FramebufferResized) {
			m_FramebufferResized = false;
			//out of date
			return true;
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		m_CurrentFrame = (m_CurrentFrame + 1) % m_MAX_FRAMES_IN_FLIGHT;

		//not out of date
		return false;
	}

	void VulkanRenderer::CleanupSwapChain(
		VkSwapchainKHR swapChain, VkRenderPass renderPass, VkCommandPool commandPool, std::pair<VkPipeline, VkPipelineLayout>pipeline, 
		std::vector<VkImageView> swapChainImageViews, std::vector<VkCommandBuffer> commandBuffers, std::vector<VkFramebuffer> swapChainFramebuffers,
		VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout
	)
	{
		for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
			vkDestroyFramebuffer(m_Device, swapChainFramebuffers[i], nullptr);
		}
		
		vkFreeCommandBuffers(m_Device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

		vkDestroyDescriptorSetLayout(m_Device, descriptorSetLayout, nullptr);
		vkDestroyPipeline(m_Device, pipeline.first, nullptr);
		vkDestroyPipelineLayout(m_Device, pipeline.second, nullptr);
		vkDestroyRenderPass(m_Device, renderPass, nullptr);
		vkDestroyDescriptorPool(m_Device, descriptorPool, nullptr);

		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			vkDestroyImageView(m_Device, swapChainImageViews[i], nullptr);
		}

		vkDestroySwapchainKHR(m_Device, swapChain, nullptr);
	}

	void VulkanRenderer::Destroy(VkCommandPool commandPool, std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> semaphores)
	{
		for (size_t i = 0; i < m_MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(m_Device, semaphores.first[i], nullptr);
			vkDestroySemaphore(m_Device, semaphores.second[i], nullptr);
			vkDestroyFence(m_Device, m_InFlightFences[i], nullptr);
			vkDestroyFence(m_Device, m_ImagesInFlight[i], nullptr);
		}

		vkDestroyCommandPool(m_Device, commandPool, nullptr);


		vkDestroyDevice(m_Device, nullptr);

		if (m_EnableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessengerCallback, nullptr);
		}

		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
		vkDestroyInstance(m_Instance, nullptr);

		m_Messages.emplace_back("VK instance destroyed and cleaned");
		Notify();
	}

	VulkanRenderer::~VulkanRenderer()
	{

	}

	void VulkanRenderer::Attach(IObserver* observer)
	{
		m_Observers.push_back(observer);
	}

	void VulkanRenderer::Detach(IObserver* observer)
	{
		m_Observers.remove(observer);
	}

	void VulkanRenderer::Notify()
	{
		std::list<IObserver*>::iterator iterator = m_Observers.begin();
		
		while (iterator != m_Observers.end()) {
			(*iterator)->Update(m_Messages);
			++iterator;
		}

		m_Messages.clear();
	}
}