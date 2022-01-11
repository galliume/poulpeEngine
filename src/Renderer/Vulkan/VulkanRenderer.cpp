#include "VulkanRenderer.h"

namespace Rebulk {

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRenderer::DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		//m_Message = pCallbackData->pMessage;

		Notify();
		return VK_FALSE;
	}

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pCallback);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	VulkanRenderer::VulkanRenderer()
	{
#ifdef NDEBUG
		m_EnableValidationLayers = false;
#else
		m_EnableValidationLayers = true;
#endif
	}

	void VulkanRenderer::Init()
	{
		LoadRequiredExtensions();
		CreateInstance();
		EnumerateExtensions();
	}

	void VulkanRenderer::CreateInstance()
	{
		if (IsValidationLayersEnabled() && !CheckValidationLayerSupport()) {
			throw std::runtime_error("Validations layers not available !");
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

		if (m_EnableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
			createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		VkResult result = VK_SUCCESS;

		result = vkCreateInstance(&createInfo, nullptr, &m_Instance);

		if (VK_SUCCESS != result) {
			Rebulk::Log::GetLogger()->critical("Can't create VK instance : {}", result);
			exit(-1);
		}

		m_InstanceCreated = true;
		//m_Message = "VK instance created";
		Rebulk::Log::GetLogger()->trace("VK instance created");
	}

	void VulkanRenderer::EnumerateExtensions()
	{
		vkEnumerateInstanceExtensionProperties(nullptr, &m_ExtensionCount, nullptr);

		std::vector<VkExtensionProperties> extensions(m_ExtensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &m_ExtensionCount, extensions.data());

		Rebulk::Log::GetLogger()->trace("{} available extensions:\n", m_ExtensionCount);

		for (const auto& extension : extensions) {
			Rebulk::Log::GetLogger()->trace("\t {} \n", extension.extensionName);
		}

		m_Extensions = extensions;
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

		return true;
	}

	void VulkanRenderer::LoadRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (m_EnableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		m_RequiredExtensions = extensions;
		m_Title = std::string("extensions loaded");
		for_each(extensions.begin(), extensions.end(), [this](const char* text) { 
			std::string message = std::string(text);
			m_Messages.push_back(message);
		});
		Notify();
	}

	void VulkanRenderer::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		//createInfo.pfnUserCallback = Rebulk::VulkanLayer::DebugCallback;

	}

	void VulkanRenderer::SetupDebugMessenger()
	{
		if (!m_EnableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		PopulateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &callback) != VK_SUCCESS)
		{
			throw std::runtime_error("can't create debug messenger");
		}
	}

	VulkanRenderer::~VulkanRenderer()
	{
		vkDestroyInstance(m_Instance, 0);
		Rebulk::Log::GetLogger()->debug("VK instance destroyed");
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
			(*iterator)->Update(m_Title, m_Messages);
			++iterator;
		}
	}
}