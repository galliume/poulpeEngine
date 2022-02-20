#pragma once
#include "rebulkpch.h"
#include "Rebulk/Renderer/Mesh.h"
#include "Rebulk/Component/Camera.h"
#include "Rebulk/Manager/TextureManager.h"
#include "Rebulk/Manager/MeshManager.h"
#include "Rebulk/Manager/ShaderManager.h"

namespace Rbk
{
	class IRendererAdapter
	{
	public:
		virtual void Init() = 0;
		virtual void AddTextureManager(TextureManager* textureManager) = 0;
		virtual void AddMeshManager(MeshManager* meshManager) = 0;
		virtual void AddShaderManager(ShaderManager* shaderManager) = 0;
		virtual void AddCamera(Camera* camera) = 0;
		virtual void PrepareWorld() = 0;
		virtual void PrepareDraw() = 0;
		virtual void Draw() = 0;
		virtual void Destroy() = 0;
	};
}