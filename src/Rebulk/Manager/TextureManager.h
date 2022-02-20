#pragma once
#include "rebulkpch.h"
#include "Rebulk/Renderer/Vulkan/VulkanRenderer.h"

namespace Rbk
{
	class TextureManager
	{
	public:
		TextureManager(VulkanRenderer* renderer);
		void AddTexture(const char* name, const char* path);
		inline std::map<const char*, Texture> GetTextures() { return m_Textures; };

	private:
		std::map<const char*, Texture> m_Textures;
		VulkanRenderer* m_Renderer = nullptr;
	};
}