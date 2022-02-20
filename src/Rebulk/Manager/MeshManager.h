#pragma once
#include "rebulkpch.h"
#include "Rebulk/Renderer/Vulkan/VulkanRenderer.h"
#include "Rebulk/Core/TinyObjLoader.h"

namespace Rbk
{
	class MeshManager
	{
	public:
		MeshManager(VulkanRenderer* renderer);
		void AddWorldMesh(const char* name, const char* path, const char* textureName, glm::vec3 pos, bool shouldInverseTextureY = true);
		inline VulkanMesh* GetWorld() { return &m_World; };
	
	private:
		Mesh Load(const char* path, bool shouldInverseTextureY);

	private:
		VulkanMesh m_World;
		VulkanRenderer* m_Renderer = nullptr;
	};
}