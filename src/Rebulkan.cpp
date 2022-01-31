#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "rebulkpch.h"
#include "GUI/VulkanLayer.h"


static void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto renderer = reinterpret_cast<Rebulk::VulkanRenderer*>(glfwGetWindowUserPointer(window));
	renderer->m_FramebufferResized = true;
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


struct Mesh
{
	std::vector<Rebulk::Vertex> vertices;
	std::vector<uint32_t> indices;
};

bool LoadMesh(Mesh& mesh, const char* path)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path)) {
		throw std::runtime_error(warn + err);
	}
	
	std::unordered_map<Rebulk::Vertex, uint16_t> uniqueVertices{};

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {

			Rebulk::Vertex vertex{};

			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			if (attrib.texcoords.size() > 0) {
				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};
			}

			vertex.color = { 1.0f, 1.0f, 1.0f };
			mesh.vertices.push_back(vertex);

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint16_t>(mesh.vertices.size());
				mesh.vertices.push_back(vertex);
			}

			mesh.indices.push_back(uniqueVertices[vertex]);
		}
	}
}

int main(int argc, char** argv)
{
	Rebulk::Log::Init();
	
	const uint32_t WIDTH = 2560;
	const uint32_t HEIGHT = 1440;

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Rebulkan", nullptr, nullptr);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);//vsync

	Rebulk::VulkanRenderer* renderer = new Rebulk::VulkanRenderer(window);

	VkRenderPass renderPass = renderer->CreateRenderPass();

	auto vertShaderCode = ReadFile("shaders/spv/vert.spv");
	auto fragShaderCode = ReadFile("shaders/spv/frag.spv");

	VkShaderModule vertShaderModule = renderer->CreateShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = renderer->CreateShaderModule(fragShaderCode);

	std::vector<VkImage> swapChainImages = {};

	VkDescriptorSetLayout descriptorSetLayout = renderer->CreateDescriptorSetLayout();
	VkPipelineLayout pipelineLayout = renderer->CreatePipelineLayout(descriptorSetLayout);
	VkPipelineCache pipelineCache = 0;
	VkPipeline pipeline = renderer->CreateGraphicsPipeline(renderPass, pipelineLayout, pipelineCache, vertShaderModule, fragShaderModule);
	VkSwapchainKHR swapChain = renderer->CreateSwapChain(swapChainImages);
	std::vector<VkImageView> swapChainImageViews = renderer->CreateImageViews(swapChainImages);
	std::vector<VkFramebuffer> swapChainFramebuffers = renderer->CreateFramebuffers(renderPass, swapChainImageViews);
	VkCommandPool commandPool = renderer->CreateCommandPool();
	std::vector<VkCommandBuffer> commandBuffers = renderer->AllocateCommandBuffers(commandPool, swapChainFramebuffers.size());
	VkDescriptorPool descriptorPool = renderer->CreateDescriptorPool(swapChainImages);
	std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> semaphores = renderer->CreateSyncObjects(swapChainImages);

	//const std::vector<Rebulk::Vertex> vertices = {
	//	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
	//	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
	//	{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
	//	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
	//};

	//const std::vector<uint16_t> indices = {
	//	0, 1, 2, 2, 3, 0
	//};

	//std::pair<VkBuffer, VkDeviceMemory> vertexBuffer = renderer->CreateVertexBuffer(commandPool, vertices);
	//std::pair<VkBuffer, VkDeviceMemory> indexBuffer = renderer->CreateIndexBuffer(commandPool, indices);

	glfwSetWindowUserPointer(window, renderer);
	glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);

	double lastTime = glfwGetTime();
	bool show_demo_window = true;

	uint32_t imageIndex = 0;

	Mesh meshObj;
	LoadMesh(meshObj, "mesh/moon.obj");

	std::pair<VkBuffer, VkDeviceMemory> meshVBuffer = renderer->CreateVertexBuffer(commandPool, meshObj.vertices);
	std::pair<VkBuffer, VkDeviceMemory> meshIBuffer = renderer->CreateIndexBuffer(commandPool, meshObj.indices);
	std::pair<std::vector<VkBuffer>, std::vector<VkDeviceMemory>> uniformBuffers = renderer->CreateUniformBuffers(swapChainImageViews);

	std::vector<VkDescriptorSet> descriptorSets = renderer->CreateDescriptorSets(descriptorPool, descriptorSetLayout, swapChainImages, uniformBuffers);

	while (!glfwWindowShouldClose(window)) {

		double currentTime = glfwGetTime();
		double timeStep = currentTime - lastTime;

		glfwPollEvents();

		if (renderer->SouldResizeSwapChain(swapChain)) {
			VkSwapchainKHR old = swapChain;
			swapChain = renderer->CreateSwapChain(swapChainImages, old);
			renderer->DestroySwapchain(renderer->GetDevice(), old, swapChainFramebuffers, swapChainImageViews);
			renderer->DestroySemaphores(semaphores);
			renderer->ResetCurrentFrameIndex();
			swapChainImageViews = renderer->CreateImageViews(swapChainImages);
			swapChainFramebuffers = renderer->CreateFramebuffers(renderPass, swapChainImageViews);
			semaphores = renderer->CreateSyncObjects(swapChainImages);
			renderer->ResetCommandPool(commandPool);
			commandBuffers = renderer->AllocateCommandBuffers(commandPool, swapChainFramebuffers.size());
		}

		for (size_t i = 0; i < commandBuffers.size() -1 ; i++) {

			imageIndex = renderer->AcquireNextImageKHR(swapChain, semaphores);

			renderer->BeginCommandBuffer(commandBuffers[imageIndex]);

			VkImageMemoryBarrier renderBeginBarrier = renderer->SetupImageMemoryBarrier(
				swapChainImages[imageIndex], 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			);

			renderer->AddPipelineBarrier(
				commandBuffers[imageIndex], renderBeginBarrier, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT
			);

			renderer->BeginRenderPass(renderPass, commandBuffers[imageIndex], swapChainFramebuffers[imageIndex]);
			renderer->SetViewPort(commandBuffers[imageIndex]);
			renderer->SetScissor(commandBuffers[imageIndex]);
			renderer->BindPipeline(commandBuffers[imageIndex], pipeline);
			renderer->Draw(commandBuffers[imageIndex], meshVBuffer.first, meshObj.vertices, meshIBuffer.first, meshObj.indices, uniformBuffers.first[imageIndex], descriptorSets[imageIndex], pipelineLayout);
			renderer->UpdateUniformBuffer(uniformBuffers.second[imageIndex]);
			renderer->EndRenderPass(commandBuffers[imageIndex]);

			VkImageMemoryBarrier renderEndBarrier = renderer->SetupImageMemoryBarrier(
				swapChainImages[imageIndex], VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			);

			renderer->AddPipelineBarrier(
				commandBuffers[imageIndex], renderEndBarrier, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_DEPENDENCY_BY_REGION_BIT
			);

			renderer->EndCommandBuffer(commandBuffers[imageIndex]);

			if (-1 != imageIndex) {
				renderer->QueueSubmit(imageIndex, commandBuffers[imageIndex], semaphores);
				renderer->QueuePresent(imageIndex, swapChain, semaphores);
			}	
		}
		
		renderer->WaitIdle(); 
		renderer->ResetCommandPool(commandPool);

		glfwSwapBuffers(window);
		lastTime = currentTime;
	}
	
	//renderer->DestroySwapchain(renderer->GetDevice(), imguiSwapChain, imguiSwapChainFramebuffers, imguiSwapChainImageViews);
	//renderer->DestroyRenderPass(imguiRenderPass, imguiCommandPool, imguiCommandBuffers);
	renderer->DestroySwapchain(renderer->GetDevice(), swapChain, swapChainFramebuffers, swapChainImageViews);
	renderer->DestroySemaphores(semaphores);
	renderer->DestroyVertexBuffer(meshVBuffer.first, meshVBuffer.second);
	renderer->DestroyVertexBuffer(meshIBuffer.first, meshIBuffer.second);
	
	/*renderer->DestroyVertexBuffer(vertexBuffer.first, vertexBuffer.second);
	renderer->DestroyVertexBuffer(indexBuffer.first, indexBuffer.second);*/
	renderer->DestroyPipeline(pipeline, pipelineLayout, descriptorPool, descriptorSetLayout);
	renderer->DestroyRenderPass(renderPass, commandPool, commandBuffers);
	renderer->Destroy();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}