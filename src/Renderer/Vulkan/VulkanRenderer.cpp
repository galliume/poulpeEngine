#include "VulkanRenderer.h"

namespace Rebulk {

	VulkanRenderer::VulkanRenderer(uint32_t extensionCount, const char* const* extensions)
	{
		CreateInstance(extensionCount, extensions);
		EnumerateExtensions();
	}

	void VulkanRenderer::CreateInstance(uint32_t extensionCount, const char* const* extensions)
	{
#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif

		if (enableValidationLayers && !CheckValidationLayerSupport()) {
			throw std::runtime_error("Validations layers not available !");
		}

		VkApplicationInfo appInfo{};

		appInfo.apiVersion = VK_API_VERSION_1_0;
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Rebulkan";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = extensionCount;
		createInfo.ppEnabledExtensionNames = extensions;
		createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
		createInfo.ppEnabledLayerNames = m_ValidationLayers.data();

		VkResult result = VK_SUCCESS;

		result = vkCreateInstance(&createInfo, nullptr, &m_Instance);

		if (VK_SUCCESS != result) {
			Rebulk::Log::GetLogger()->critical("Can't create VK instance : {}", result);
			exit(-1);
		}

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

		return true;
	}

	VulkanRenderer::~VulkanRenderer()
	{
		vkDestroyInstance(m_Instance, 0);
		Rebulk::Log::GetLogger()->debug("VK instance destroyed");
	}
}