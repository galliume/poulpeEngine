#include "VulkanRenderer.h"
#include "rebulkpch.h"

namespace Rebulk {

	VulkanRenderer::VulkanRenderer(uint32_t extensionCount, const char* const* extensions)
	{
		CreateInstance(extensionCount, extensions);
		EnumerateExtensions();
	}

	void VulkanRenderer::CreateInstance(uint32_t extensionCount, const char* const* extensions)
	{
		const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
		const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

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
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		VkResult result = VK_SUCCESS;

		result = vkCreateInstance(&createInfo, nullptr, &m_Instance);

		if (VK_SUCCESS != result)
		{
			Rebulk::Log::GetLogger()->critical("Can't create VK instance : {}", result);
			exit(-1);
		}

		Rebulk::Log::GetLogger()->trace("VK instance created");
	}

	void VulkanRenderer::EnumerateExtensions()
	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		Rebulk::Log::GetLogger()->trace("{} available extensions:\n", extensionCount);

		for (const auto& extension : extensions) {
			Rebulk::Log::GetLogger()->trace("\t {} \n", extension.extensionName);
		}
	}

	VulkanRenderer::~VulkanRenderer()
	{
		vkDestroyInstance(m_Instance, 0);
		Rebulk::Log::GetLogger()->debug("VK instance destroyed");
	}
}