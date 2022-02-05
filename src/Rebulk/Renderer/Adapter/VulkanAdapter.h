#include "IRendererAdapter.h"
#include "Rebulk/Renderer/Vulkan/VulkanRenderer.h"
#include "Rebulk/Renderer/Mesh.h"

namespace Rbk
{
	struct VulkanShader
	{
		VkShaderModule vertex;
		VkShaderModule frag;
		VkPipeline pipeline;
	};

	struct VulkanMesh
	{
		Rbk::Mesh mesh;
		std::pair<VkBuffer, VkDeviceMemory> meshVBuffer;
		std::pair<VkBuffer, VkDeviceMemory> meshIBuffer;
	};

	class VulkanAdapter : public IRendererAdapter
	{
	public:

		VulkanAdapter(GLFWwindow* window);
		~VulkanAdapter();

		virtual void Init() override;
		virtual void AddShader(std::vector<char> vertexShaderCode, std::vector<char> fragShaderCode) override;
		virtual void AddMesh(Rbk::Mesh mesh) override;
		virtual void Draw() override;
		virtual void Destroy() override;

		void SouldResizeSwapChain();

	private:
		VulkanRenderer* m_Renderer = nullptr;
		VkRenderPass m_RenderPass = nullptr;
		VkSwapchainKHR m_SwapChain = nullptr;
		std::vector<VkImage> m_SwapChainImages = {};
		std::vector<VkFramebuffer> m_SwapChainFramebuffers = {};
		std::vector<VkImageView> m_SwapChainImageViews = {};
		std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> m_Semaphores = {};
		VkCommandPool m_CommandPool = nullptr;
		std::vector<VkCommandBuffer> m_CommandBuffers = {};
		VkPipeline m_Pipeline = nullptr;
		uint32_t m_ImageIndex = 0;
		std::pair<std::vector<VkBuffer>, std::vector<VkDeviceMemory>> m_UniformBuffers = {};
		std::vector<VkDescriptorSet> m_DescriptorSets = {};
		VkPipelineLayout m_PipelineLayout = nullptr;
		VkDescriptorPool m_DescriptorPool = nullptr;
		VkDescriptorSetLayout m_DescriptorSetLayout = nullptr;

		std::vector<VulkanShader>m_Shaders;
		std::vector<VulkanMesh>m_Meshes;
	};
}