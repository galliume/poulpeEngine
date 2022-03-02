#pragma once
#include "rebulkpch.h"
#include "Rebulk/Renderer/Vulkan/VulkanRenderer.h"

namespace Rbk
{
	class TextureManager
	{
	public:
		TextureManager(VulkanRenderer* renderer);
		void AddSkyBox(std::vector<const char*> skyboxImages);
		inline Skybox GetSkyboxTexture() { return m_Skybox; };
		void AddTexture(const char* name, const char* path);
		inline std::map<const char*, Texture> GetTextures() { return m_Textures; };

	private:
		std::map<const char*, Texture> m_Textures;
		Skybox m_Skybox;
		VulkanRenderer* m_Renderer = nullptr;
	};
}