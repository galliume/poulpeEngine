#include "Camera.hpp"

namespace Poulpe
{
    void Camera::init()
    {
        m_Pos = glm::vec3(-0.2f, 0.2f, 0.0f);
        m_Target = glm::vec3(0.0f, 0.0f, 0.0f);
        m_Direction = glm::normalize(m_Pos - m_Target);

        m_CameraUp = glm::vec3(0.0f, 1.0f,  0.0f);
        m_CameraFront = glm::vec3(0.0f, 0.0f, -1.0f);

        m_View = glm::lookAt(m_Pos, m_Pos + m_CameraFront, m_CameraUp);
    }

    void Camera::backward()
    {
      m_Pos -= m_CameraFront * (m_Speed * m_DeltaTime);
    }

    void Camera::down()
    {
      m_Pos += m_CameraUp * (m_Speed * m_DeltaTime);
    }

    void Camera::forward()
    {
      m_Pos += m_CameraFront * (m_Speed * m_DeltaTime);
    }

    void Camera::left()
    {
      m_Pos -= glm::normalize(glm::cross(m_CameraFront, m_CameraUp)) * (m_Speed * m_DeltaTime);
    }

    void Camera::right()
    {
      m_Pos += glm::normalize(glm::cross(m_CameraFront, m_CameraUp)) * (m_Speed * m_DeltaTime);
    }

    void Camera::up()
    {
      m_Pos -= m_CameraUp * (m_Speed * m_DeltaTime);
    }

    /**
        fovy : viertical field of view
        s : aspect ratio viewport
        n : near distance
        f : far distance
    **/
    glm::mat4 Camera::frustumProj(float fovy, float s, float n, float f)
    {
        //infine reverse frustum projection
        /*float g = 1.0f / tan(fovy * 0.5f);
        float e = 0.1f;

        frustumProj = glm::mat4(
            g / s, 0.0f, 0.0f, 0.0f,
            0.0f, g, 0.0f, 0.0f,
            0.0f, 0.0f, e, n * (1.0f - e),
            0.0f, 0.0f, 1.0f, 0.0f
        );*/

        //infine frustum projection
        //glm::mat4 frustumProj;
        //float g = 1.0f / tan(fovy * 0.5f);
        //float e = 1.0f - 0.00001f;

        //frustumProj = glm::mat4(
        //	g / s, 0.0f, 0.0f, 0.0f,
        //	0.0f, g, 0.0f, 0.0f,
        //	0.0f, 0.0f, e, -n * e,
        //	0.0f, 0.0f, 1.0f, 0.0f
        //);

        //frustum projection
        float g = 1.0f / std::tan(fovy * 0.5f);
        float k = f / (f - n);

        glm::mat4 frustumProj = glm::mat4(
          g / s, 0.0f, 0.0f, 0.0f,
          0.0f, g, 0.0f, 0.0f,
          0.0f, 0.0f, k, -n * k,
          0.0f, 0.0f, 1.0f, 0.0f
        );

        //reverse frustum projection
        //float g = 1.0f / std::tan(fovy * 0.5f);
        //float k = n / (n - f);

        //glm::mat4 frustumProj = glm::mat4(
        //  g / s, 0.0f, 0.0f, 0.0f,
        //  0.0f, g, 0.0f, 0.0f,
        //  0.0f, 0.0f, k, -f * k,
        //  0.0f, 0.0f, 1.0f, 0.0f
        //);
        //float k = f / (f - n);

        //float g = 1.0f / tan(fovy * 0.5f);
        //float e = 1.0f - 0.0f;

        //glm::mat4 frustumProj = glm::mat4(0.0f);
        //frustumProj[0][0] = g / s;
        //frustumProj[1][1] = g;
        //frustumProj[2][2] = e;
        //frustumProj[2][3] = -n * e;
        //frustumProj[3][3] = 1.0f;

        return frustumProj;
    }

    glm::mat4 Camera::lookAt()
    {
        m_View = glm::lookAt(m_Pos, m_Pos + m_CameraFront, m_CameraUp);
        return m_View;
    }

    void Camera::updateDeltaTime(float deltaTime)
    {
        m_DeltaTime = deltaTime;
    }

    void Camera::updatePos(double xoffset, double yoffset)
    {
        m_Yaw += static_cast<float>(xoffset);
        m_Pitch += static_cast<float>(yoffset);

        if (m_Pitch > 89.0f) {
            m_Pitch = 89.0f;
        }
        if (m_Pitch < -89.0f) {
            m_Pitch = -89.0f;
        }

        glm::vec3 direction{};
        direction.x = static_cast<float>(cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch)));
        direction.y = static_cast<float>(sin(glm::radians(m_Pitch)));
        direction.z = static_cast<float>(sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch)));
        m_CameraFront = glm::normalize(direction);
    }
}
