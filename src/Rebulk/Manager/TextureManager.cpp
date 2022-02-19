#include "TextureManager.h"

namespace Rbk
{
	TextureManager::TextureManager(VulkanRenderer* renderer) : m_Renderer(renderer)
	{

	}

	void TextureManager::AddTexture(const char* name, const char* path)
	{
		if (0 != m_Textures.count(name)) {
			std::cout << "Texture " << name << " already imported" << std::endl;
			return;
		}

		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		if (!pixels) {
			Rbk::Log::GetLogger()->warn("failed to load texture image %s", name);
			return;
		}

		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

		VkCommandPool commandPool = m_Renderer->CreateCommandPool();

		VkCommandBuffer commandBuffer = m_Renderer->AllocateCommandBuffers(commandPool)[0];
		m_Renderer->BeginCommandBuffer(commandBuffer);
		m_Renderer->CreateTextureImage(commandBuffer, pixels, texWidth, texHeight, mipLevels, textureImage, textureImageMemory, VK_FORMAT_R8G8B8A8_SRGB);

		VkImageView textureImageView = m_Renderer->CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, mipLevels);
		VkSampler textureSampler = m_Renderer->CreateTextureSampler(mipLevels);

		VkDeviceMemory colorImageMemory;
		VkImage colorImage;
		m_Renderer->CreateImage(m_Renderer->GetSwapChainExtent().width, m_Renderer->GetSwapChainExtent().height, 1, m_Renderer->GetMsaaSamples(), m_Renderer->GetSwapChainImageFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory);
		VkImageView colorImageView = m_Renderer->CreateImageView(colorImage, m_Renderer->GetSwapChainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT, 1);

		VkImage depthImage;
		VkDeviceMemory depthImageMemory;
		m_Renderer->CreateImage(m_Renderer->GetSwapChainExtent().width, m_Renderer->GetSwapChainExtent().height, 1, m_Renderer->GetMsaaSamples(), m_Renderer->FindDepthFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
		VkImageView depthImageView = m_Renderer->CreateImageView(depthImage, m_Renderer->FindDepthFormat(), 1, VK_IMAGE_ASPECT_DEPTH_BIT);

		Rbk::Texture texture;
		texture.name = name;
		texture.image = textureImage;
		texture.imageMemory = textureImageMemory;
		texture.imageView = textureImageView;
		texture.sampler = textureSampler;
		texture.mipLevels = mipLevels;
		texture.width = texWidth;
		texture.height = texHeight;
		texture.channels = texChannels;
		texture.colorImageView = colorImageView;
		texture.colorImage = colorImage;
		texture.colorImageMemory = colorImageMemory;
		texture.depthImage = depthImage;
		texture.depthImageView = depthImageView;
		texture.depthImageMemory = depthImageMemory;

		m_Textures.emplace(name, texture);

		Rbk::Log::GetLogger()->debug("Added texture {} from {}", name, path);

		vkDestroyCommandPool(m_Renderer->GetDevice(), commandPool, nullptr);
	}
}