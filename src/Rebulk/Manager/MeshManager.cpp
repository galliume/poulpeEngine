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
		
		float fovy = glm::radians(60.0f);
		float f = 100.0f;
		float n = 0.1f;
		float s = 2560.0f / 1440.0f;
		float g = 1.0f / std::tan(fovy * 0.5f);
		float k = f / (f - n);

		glm::mat4 frustumProj = glm::mat4(
			g / s, 0.0f, 0.0f, 0.0f,
			0.0f, g, 0.0f, 0.0f,
			0.0f, 0.0f, k, -n * k,
			0.0f, 0.0f, 1.0f, 0.0f
		);


		//T const tanHalfFovy = tan(fovy / static_cast<T>(2));
		//Result[0][0] = static_cast<T>(1) / (aspect * tanHalfFovy);
		//Result[1][1] = static_cast<T>(1) / (tanHalfFovy);
		//Result[2][2] = zFar / (zNear - zFar);
		//Result[2][3] = -static_cast<T>(1);
		//Result[3][2] = -(zFar * zNear) / (zFar - zNear);

		//frustumProj[1][1] *= -1;
		//glm::mat4 projection;
		//ubo.proj = glm::perspective(glm::radians(60.0f), m_Renderer->GetSwapChainExtent().width / (float)m_Renderer->GetSwapChainExtent().height, 0.1f, 100.0f);
		ubo.proj = frustumProj;
		//ubo.proj[1][1] *= -1;
		
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