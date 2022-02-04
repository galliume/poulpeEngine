#pragma once
#include "rebulkpch.h"
#include "Renderer/Mesh.h"


namespace Rbk
{
	class TinyObjLoader
	{
	public:
		static bool LoadMesh(Rbk::Mesh& mesh, const char* path);
	};
}
