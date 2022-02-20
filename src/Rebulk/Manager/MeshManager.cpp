#include "MeshManager.h"

namespace Rbk
{
	MeshManager::MeshManager(VulkanRenderer* renderer) : m_Renderer(renderer)
	{

	}

	void MeshManager::AddMesh(const char* name, const char* path, const char* textureName, glm::vec3 pos, bool shouldInverseTextureY)
	{
		if (!std::filesystem::exists(path)) {
			Rbk::Log::GetLogger()->critical("mesh file {} does not exits.", path);
			return;
		}

		Mesh mesh = Rbk::TinyObjLoader::LoadMesh(path, shouldInverseTextureY);

		glm::mat4 view = glm::mat4(1.0f);

		UniformBufferObject ubo;
		ubo.model = glm::mat4(1.0f);
		ubo.model = glm::translate(ubo.model, pos);
		ubo.model = glm::scale(ubo.model, glm::vec3(0.1f, 0.1f, 0.1f));
		ubo.view = glm::translate(view, glm::vec3(0.0f, 0.0f, -0.0f));

		glm::mat4 projection;
		ubo.proj = glm::perspective(glm::radians(45.0f), m_Renderer->GetSwapChainExtent().width / (float)m_Renderer->GetSwapChainExtent().height, 0.1f, 100.0f);
		ubo.proj[1][1] *= -1;

		m_Meshes.mesh.ubos.emplace_back(ubo);
		

		if (0 != m_Meshes.mesh.meshNames.count(name)) {
			m_Meshes.mesh.meshNames[name] += 1;
			return;
		}

		m_Meshes.mesh.meshTextures.emplace(name, textureName);
		m_Meshes.mesh.textureNames.emplace(m_Meshes.count, textureName);
		m_Meshes.mesh.vertices.insert(m_Meshes.mesh.vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
		m_Meshes.mesh.indices.insert(m_Meshes.mesh.indices.end(), mesh.indices.begin(), mesh.indices.end());
		m_Meshes.vertexIndicesCount += mesh.indices.size();
		m_Meshes.mesh.meshNames.insert({ name, 1 });
		m_Meshes.count += 1;

		Rbk::Log::GetLogger()->debug("Added mesh {} from {}", name, path);
	}
}