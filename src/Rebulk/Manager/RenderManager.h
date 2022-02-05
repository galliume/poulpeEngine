#include "rebulkpch.h"

#include "Rebulk/Renderer/Adapter/IRendererAdapter.h"


namespace Rbk
{
	class RenderManager
	{
	public:
		RenderManager(GLFWwindow* window, IRendererAdapter* renderer);
		~RenderManager();

		void Init();

		inline static RenderManager& Get() { return *s_Instance; };
		inline IRendererAdapter* Rdr() { return m_Renderer; };

	private:
		static RenderManager* s_Instance;
		IRendererAdapter* m_Renderer = nullptr;
	};
}