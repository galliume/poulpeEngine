#pragma once
#include "rebulkpch.h"
#include "Rebulk/Renderer/Mesh.h"

namespace Rbk
{
	class IRendererAdapter
	{
	public:
		virtual void Init() = 0;
		virtual void AddShader(std::vector<char> vertexShaderCode, std::vector<char> fragShaderCode) = 0;
		virtual void AddTexture(Rbk::Mesh& mesh, const char* texturePath) = 0;
		virtual void AddMesh(Rbk::Mesh mesh) = 0;
		virtual void Draw() = 0;
		virtual void Destroy() = 0;
	};
}