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

	static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

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

	VulkanRenderer::VulkanRenderer(GLFWwindow* window) : m_Window(window)
	{
#ifdef NDEBUG
		m_EnableValidationLayers = false;
#else
		m_EnableValidationLayers = true;
#endif
	}

	void VulkanRenderer::Init()
	{
		EnumerateExtensions();
		LoadRequiredExtensions();
		CreateInstance();
		SetupDebugMessenger();
		CreateSurface();
		PickPhysicalDevice();
		CreateLogicalDevice();
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
			std::string message = std::string("Can't create VK instance : " + result);
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
		QueueFamilyIndices indices;
		
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);

			if (presentSupport) {
				indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}
			i++;
		}

		return indices;
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

	VulkanRenderer::~VulkanRenderer()
	{
		vkDestroyDevice(m_Device, nullptr);

		if (m_EnableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessengerCallback, nullptr);
		}

		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
		vkDestroyInstance(m_Instance, 0);

		m_Messages.emplace_back("VK instance destroyed");
		Notify();
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