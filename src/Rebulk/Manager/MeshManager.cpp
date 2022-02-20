#include "MeshManager.h"

namespace Rbk
{
	MeshManager::MeshManager(VulkanRenderer* renderer) : m_Renderer(renderer)
	{

	}

	Mesh MeshManager::Load(const char* path, bool shouldInverseTextureY)
	{
		if (!std::filesystem::exists(path)) {
			Rbk::Log::GetLogger()->critical("mesh file {} does not exits.", path);
			throw std::runtime_error("error loading a mesh file.");
		}

		return Rbk::TinyObjLoader::LoadMesh(path, shouldInverseTextureY);
	}

	void MeshManager::AddWorldMesh(const char* name, const char* path, const char* textureName, glm::vec3 pos, bool shouldInverseTextureY)
	{
		Mesh mesh = Load(path, shouldInverseTextureY);

		glm::mat4 view = glm::mat4(1.0f);

		UniformBufferObject ubo;
		ubo.model = glm::mat4(1.0f);
		ubo.model = glm::translate(ubo.model, pos);
		ubo.model = glm::scale(ubo.model, glm::vec3(0.1f, 0.1f, 0.1f));
		ubo.view = glm::translate(view, glm::vec3(0.0f, 0.0f, -0.0f));

		glm::mat4 projection;
		ubo.proj = glm::perspective(glm::radians(45.0f), m_Renderer->GetSwapChainExtent().width / (float)m_Renderer->GetSwapChainExtent().height, 0.1f, 100.0f);
		ubo.proj[1][1] *= -1;

		m_World.mesh.ubos.emplace_back(ubo);

		if (0 != m_World.mesh.meshNames.count(name)) {
			m_World.mesh.meshNames[name] += 1;
			return;
		}

		m_World.mesh.meshTextures.emplace(name, textureName);
		m_World.mesh.textureNames.emplace(m_World.count, textureName);
		m_World.mesh.vertices.insert(m_World.mesh.vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
		m_World.mesh.indices.insert(m_World.mesh.indices.end(), mesh.indices.begin(), mesh.indices.end());
		m_World.vertexIndicesCount += mesh.indices.size();
		m_World.mesh.meshNames.insert({ name, 1 });
		m_World.count += 1;

		Rbk::Log::GetLogger()->debug("Added mesh to the world {} from {}", name, path);
	}
}