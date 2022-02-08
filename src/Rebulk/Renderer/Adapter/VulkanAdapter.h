#include "rebulkpch.h"
#include "IRendererAdapter.h"
#include "Rebulk/Renderer/Vulkan/VulkanRenderer.h"
#include "Rebulk/Renderer/Mesh.h"

namespace Rbk
{
	struct VImGuiInfo
	{
		ImGui_ImplVulkan_InitInfo info = {};
		VkCommandBuffer cmdBuffer = nullptr;
		VkPipeline pipeline = nullptr;
	};

	class VulkanAdapter : public IRendererAdapter
	{
	public:

		VulkanAdapter(GLFWwindow* window);
		~VulkanAdapter();

		virtual void Init() override;
		virtual void AddShader(std::vector<char> vertexShaderCode, std::vector<char> fragShaderCode) override;
		virtual void AddMesh(Rbk::Mesh mesh) override;
		virtual void AddTexture(Rbk::Mesh& mesh, const char* texturePath) override;
		virtual void AddUniformObject(UniformBufferObject ubo) override;
		virtual void PrepareDraw() override;
		virtual void Draw() override;
		virtual void Destroy() override;
		
		inline VulkanRenderer* Rdr() { return m_Renderer; };
		inline VkRenderPass RdrPass() { return m_RenderPass; };

		void Clear();
		void SouldResizeSwapChain();

		VImGuiInfo GetVImGuiInfo();

	private:
		void Draw(VulkanShaders vShader, VulkanMesh vMesh);

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
		uint32_t m_ImageIndex = 0;
		std::pair<std::vector<VkBuffer>, std::vector<VkDeviceMemory>> m_UniformBuffers = {};
		VkDescriptorSet m_DescriptorSet = nullptr;
		VkDescriptorSetLayout m_DescriptorSetLayout = nullptr;
		VkPipelineLayout m_PipelineLayout = nullptr;
		VkDescriptorPool m_DescriptorPool = nullptr;
		VulkanShaders m_Shaders;
		VulkanMesh m_Meshes;
		std::vector<VulkanPipeline>m_Pipelines;
		bool m_IsPrepared = false;
	};
}