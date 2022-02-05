#include "IRendererAdapter.h"
#include "Rebulk/Renderer/Vulkan/VulkanRenderer.h"
#include "Rebulk/Renderer/Mesh.h"
#include "Rebulk/Core/TinyObjLoader.h"

namespace Rbk
{
	class VulkanAdapter : public IRendererAdapter
	{
	public:

		VulkanAdapter(GLFWwindow* window);
		~VulkanAdapter();
		virtual void Init() override;
		void SouldResizeSwapChain();
		virtual void Draw() override;
		virtual void Destroy() override;

	private:
		VulkanRenderer* m_Renderer = nullptr;
		VkRenderPass m_RenderPass = nullptr;
		VkSwapchainKHR m_SwapChain = nullptr;
		std::vector<VkImage> m_SwapChainImages;
		std::vector<VkFramebuffer> m_SwapChainFramebuffers = {};
		std::vector<VkImageView> m_SwapChainImageViews = {};
		std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>> m_Semaphores = {};
		VkCommandPool m_CommandPool = nullptr;
		std::vector<VkCommandBuffer> m_CommandBuffers = {};
		VkPipeline m_Pipeline = nullptr;
		uint32_t m_ImageIndex = 0;
		std::pair<VkBuffer, VkDeviceMemory> m_MeshVBuffer = {};
		std::pair<VkBuffer, VkDeviceMemory> m_MeshIBuffer = {};
		std::pair<std::vector<VkBuffer>, std::vector<VkDeviceMemory>> m_UniformBuffers = {};
		Rbk::Mesh m_MeshObj;
		std::vector<VkDescriptorSet> m_DescriptorSets = {};
		VkPipelineLayout m_PipelineLayout = nullptr;
		VkDescriptorPool m_DescriptorPool = nullptr;
		VkDescriptorSetLayout m_DescriptorSetLayout = nullptr;
	};
}