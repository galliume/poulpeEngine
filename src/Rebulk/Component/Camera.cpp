#include "Camera.h"

namespace Rbk
{
	void Camera::Init(float width, float height)
	{
		m_Pos = glm::vec3(0.0f, 0.0f, 3.0f);
		m_Target = glm::vec3(0.0f, 0.0f, 0.0f);
		m_Direction = glm::normalize(m_Pos - m_Target);
		m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
		m_CameraRight = glm::normalize(glm::cross(m_Up, m_Direction));
		m_CameraUp = glm::cross(m_Direction, m_CameraRight);
		m_Front = glm::vec3(0.0f, 0.0f, -1.0f);

		m_View = glm::lookAt(
			glm::vec3(0.0f, 0.0f, 3.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
	}

	void Camera::Up()
	{
		Rbk::Log::GetLogger()->debug("go up");
		m_Pos += m_Speed * m_Front;
	}

	void Camera::Down()
	{
		Rbk::Log::GetLogger()->debug("go down");
		m_Pos -= m_Speed * m_Front;
	}

	void Camera::Left()
	{
		Rbk::Log::GetLogger()->debug("go left");
		m_Pos -= glm::normalize(glm::cross(m_Front, m_CameraUp)) * m_Speed;
	}

	void Camera::Right()
	{
		Rbk::Log::GetLogger()->debug("go right");
		 m_Pos += glm::normalize(glm::cross(m_Front, m_CameraUp)) * m_Speed;
	}

	glm::mat4 Camera::LookAt()
	{
		m_View = glm::lookAt(m_Pos, m_Pos + m_Front, m_CameraUp);
		return m_View;
	}

	void Camera::UpdateSpeed(float timeStep)
	{
		m_Speed = 2.5f * timeStep;
	}
}