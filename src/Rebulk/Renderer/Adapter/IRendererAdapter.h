#pragma once
#include "rebulkpch.h"
#include "Rebulk/Renderer/Mesh.h"
#include "Rebulk/Component/Camera.h"

namespace Rbk
{
	class IRendererAdapter
	{
	public:
		virtual void Init() = 0;
		virtual void AddShader(std::string name, std::vector<char> vertexShaderCode, std::vector<char> fragShaderCode) = 0;
		virtual void AddCamera(Camera* camera) = 0;
		virtual void AddTexture(const char* name, const char* path) = 0;
		virtual void AddMesh(Rbk::Mesh mesh, const char* textureName, glm::vec3 pos) = 0;
		virtual void PrepareDraw() = 0;
		virtual void Draw() = 0;
		virtual void Destroy() = 0;
	};
}