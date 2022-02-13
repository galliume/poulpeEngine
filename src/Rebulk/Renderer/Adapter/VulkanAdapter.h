#pragma once
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
		VkRenderPass* rdrPass = nullptr;
	};

	class VulkanAdapter : public IRendererAdapter
	{
	public:

		VulkanAdapter(GLFWwindow* window);
		~VulkanAdapter();

		virtual void Init() override;
		virtual void AddCamera(Camera* camera) override;
		virtual void AddShader(std::string name, std::vector<char> vertexShaderCode, std::vector<char> fragShaderCode) override;
		virtual void AddMesh(Rbk::Mesh mesh, const char* textureName, glm::vec3 pos) override;
		virtual void AddTexture(const char* name, const char* path) override;
		virtual void PrepareDraw() override;
		virtual void Draw() override;
		virtual void Destroy() override;
		void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
		void SetWireFrameMode(bool wireFrameModeOn) { m_WireFrameModeOn = wireFrameModeOn; };
		inline VulkanMesh GetMesh() { return m_Meshes; };
		inline std::map<const char*, VulkanTexture> GetTextures() { return m_Textures; };
		inline VulkanShaders GetShaders() { return m_Shaders; };
		inline uint32_t GetSwapImageIndex() { return m_ImageIndex; };
		inline VulkanRenderer* Rdr() { return m_Renderer; };
		inline VkRenderPass RdrPass() { return m_RenderPass; };
		VkRenderPass* CreateImGuiRenderPass();
		inline void MakeSpin(bool spin) { m_MakeSpin = spin; };

		void Clear();
		void SouldResizeSwapChain();

		VImGuiInfo GetVImGuiInfo();

	private:
		void UpdatePositions();
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
		std::map<const char*, VulkanTexture> m_Textures;
		std::vector<VulkanPipeline>m_Pipelines;
		bool m_IsPrepared = false;
		bool m_WireFrameModeOn = false;
		bool m_MakeSpin = false;
		Camera* m_Camera;
	};
}