#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/stdout_color_sinks.h"

std::shared_ptr<spdlog::logger>Logger = spdlog::stdout_color_mt("ReBulkan");

int main(int argc, char** argv)
{
	spdlog::set_pattern("%^[%T] %n: %v%$");
	spdlog::set_level(spdlog::level::debug);

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	const std::vector<const char*> validationLayers = { "VK_LAYER_LUNARG_standard_validation" };
	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkApplicationInfo appInfo {};
	appInfo.apiVersion = VK_VERSION_1_2;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Rebulkan";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	
	VkInstanceCreateInfo createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	//createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	//createInfo.ppEnabledLayerNames = validationLayers.data();


	VkInstance instance = VK_NULL_HANDLE;
	if (VK_SUCCESS != vkCreateInstance(&createInfo, nullptr, &instance))
	{
		Logger->critical("Can't create VK instance");
		return -1;
	}

	Logger->debug("VK instance created");

	const uint32_t WIDTH = 2560;
	const uint32_t HEIGHT = 1440;

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Rebulkan", nullptr, nullptr);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	vkDestroyInstance(instance, 0);
}