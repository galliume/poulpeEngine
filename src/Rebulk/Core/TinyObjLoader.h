#pragma once
#include "rebulkpch.h"
#include "Rebulk/Renderer/Mesh.h"


namespace Rbk
{
	class TinyObjLoader
	{
	public:
		static bool LoadMesh(Rbk::Mesh& mesh, const char* path);
	};
}
