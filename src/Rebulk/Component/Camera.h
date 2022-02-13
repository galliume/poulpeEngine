#pragma once
#include "rebulkpch.h"

namespace Rbk
{
	class Camera
	{
	public:
		void Init(float width, float height);
		inline glm::mat4 GetView() { return m_View; };
		void Up();
		void Down();
		void Left();	
		void Right();
		glm::mat4 LookAt();

	private:
		glm::vec3 m_Pos;
		glm::vec3 m_Target;
		glm::vec3 m_Direction;
		glm::vec3 m_Up;
		glm::vec3 m_CameraRight;
		glm::vec3 m_CameraUp;
		glm::mat4 m_View;
		glm::vec3 m_Front;
		const float m_Speed = 0.05f;
	};
}