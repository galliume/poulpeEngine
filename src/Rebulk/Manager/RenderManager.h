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
		void AddTexture(const char* name, const char* path);
		void AddMesh(const char* path, const char* textureName, UniformBufferObject ubo, bool shouldInverseTextureY = true);
		void AddShader(std::string name, std::vector<char> vertShaderCode, std::vector<char> fragShaderCode);
		void PrepareDraw();
		void Draw();

		inline static RenderManager& Get() { return *s_Instance; };
		inline IRendererAdapter* Adp() { return m_Renderer; };

	private:
		static RenderManager* s_Instance;
		IRendererAdapter* m_Renderer = nullptr;
		GLFWwindow* m_Window;
	};
}