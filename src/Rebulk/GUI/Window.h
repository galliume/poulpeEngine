#pragma once
#include "rebulkpch.h"

namespace Rbk
{
	static bool m_FramebufferResized;

	class Window
	{
		public:
			Window();
			void Init();
			inline GLFWwindow* Get() { return m_Window; };
			inline void SetVSync(bool active) { m_ActiveVSync = active; };
	
	private:
		GLFWwindow* m_Window;
		bool m_ActiveVSync = true;
	};
}