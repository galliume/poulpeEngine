#pragma once
#include "rebulkpch.h"
#include "Rebulk/Component/Camera.h"
#include "Rebulk/GUI/Window.h"

namespace Rbk
{
	static bool m_CanMoveCamera;
	static bool m_FirtMouseMove;

	class InputManager
	{
	public:
		InputManager(Window* window, Camera* camera);
		void Init();
		void Key(int key, int scancode, int action, int mods);
		void Mouse(double xPos, double yPo);
		void MouseButton(int button, int action, int mods);
	private:
		Window* m_Window;
		Camera* m_Camera;
		float m_LastX;
		float m_LastY;
	};
}
