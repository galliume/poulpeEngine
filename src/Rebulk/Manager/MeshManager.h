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
		void AddMesh(const char* name, const char* path, const char* textureName, glm::vec3 pos, bool shouldInverseTextureY = true);
		inline VulkanMesh* GetMeshes() { return &m_Meshes; };

	private:
		VulkanMesh m_Meshes;
		VulkanRenderer* m_Renderer = nullptr;
	};
}