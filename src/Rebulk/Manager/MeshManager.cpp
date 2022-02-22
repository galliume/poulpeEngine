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
		Mesh mesh;

		if (0 == m_WorldMeshesLoaded.count(name)) {
			mesh = Load(path, shouldInverseTextureY);
			mesh.name = name;
			mesh.texture = textureName;
		} else {
			mesh = m_WorldMeshes[m_WorldMeshesLoaded[name][1]];
		}

		glm::mat4 view = glm::mat4(1.0f);

		UniformBufferObject ubo;
		ubo.model = glm::mat4(1.0f);
		ubo.model = glm::translate(ubo.model, pos);
		//ubo.model = glm::scale(ubo.model, glm::vec3(0.05f, 0.05f, 0.05f));
		

		float fovy = 60.0f;
		float f = std::pow(2, -20);
		float n = 1.0f;
		float s = 2560 / 1440;

		float g = 1.0f / tan(fovy * 0.5f);
		float e = 0.1f;

		glm::mat4 frustumProj = glm::mat4(
			g / s, 0.0f, 0.0f, 0.0f,
			0.0f, g, 0.0f, 0.0f,
			0.0f, 0.0f, e, n * (1.0f - e),
			0.0f, 0.0f, 1.0f, 0.0f
		);

		//ubo.proj = frustumProj;
		
		//glm::mat4 projection;
		ubo.proj = glm::perspective(glm::radians(45.0f), m_Renderer->GetSwapChainExtent().width / (float)m_Renderer->GetSwapChainExtent().height, 0.1f, 100.0f);
		ubo.proj[1][1] *= -1;

		mesh.ubos.emplace_back(ubo);

		if (0 != m_WorldMeshesLoaded.count(name)) {
			m_WorldMeshesLoaded[name][0] += 1;
			m_WorldMeshes[m_WorldMeshesLoaded[name][1]] = mesh;
			return;
		} 
		
		uint32_t index = m_WorldMeshes.size();
		m_WorldMeshesLoaded.insert({ name, { 1, index }});	
		m_WorldMeshes.emplace_back(mesh);

		Rbk::Log::GetLogger()->debug("Added mesh to the world {} from {}", name, path);
	}

	uint32_t MeshManager::GetWorldVerticesCount()
	{
		uint32_t total = 0;

		for (Mesh mesh : m_WorldMeshes) {
			total += mesh.vertices.size();
		}

		return total;
	}

	uint32_t MeshManager::GetWorldIndicesCount()
	{
		uint32_t total = 0;

		for (Mesh mesh : m_WorldMeshes) {
			total += mesh.indices.size();
		}

		return total;
	}

	uint32_t MeshManager::GetWorldInstancedCount()
	{
		uint32_t total = 0;

		for (Mesh mesh : m_WorldMeshes) {
			total += mesh.ubos.size();
		}

		return total;
	}
}