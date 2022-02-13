#pragma once
#include "rebulkpch.h"
#include "Rebulk/Component/Camera.h"
#include "Rebulk/GUI/Window.h"

namespace Rbk
{
	class KeyManager
	{
	public:
		KeyManager(Window* window, Camera* camera);
		void Init();
		void Event(int key, int scancode, int action, int mods);

	private:
		Window* m_Window;
		Camera* m_Camera;
	};
}