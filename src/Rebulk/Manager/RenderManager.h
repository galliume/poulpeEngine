#include "rebulkpch.h"

#include "Rebulk/Renderer/Adapter/IRendererAdapter.h"
#include "Rebulk/Renderer/Mesh.h"
#include "Rebulk/Core/TinyObjLoader.h"

namespace Rbk
{
	class RenderManager
	{
	public:
		RenderManager(GLFWwindow* window, IRendererAdapter* renderer);
		~RenderManager();

		void Init();
		void AddMesh(const char* path);
		void AddShader(std::vector<char> vertShaderCode, std::vector<char> fragShaderCode);
		void Draw();

		inline static RenderManager& Get() { return *s_Instance; };
		inline IRendererAdapter* Adp() { return m_Renderer; };

	private:
		static RenderManager* s_Instance;
		IRendererAdapter* m_Renderer = nullptr;
	};
}