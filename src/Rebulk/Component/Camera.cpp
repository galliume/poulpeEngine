#include "rebulkpch.h"
#include "Camera.h"

namespace Rbk
{
    void Camera::Init()
    {
        m_Pos = glm::vec3(0.0f, 2.0f, 3.0f);
        m_Target = glm::vec3(0.0f, 0.0f, 0.0f);
        m_Direction = glm::normalize(m_Pos - m_Target);
        m_Up = glm::vec3(0.0f, 10.0f, 0.0f);
        m_CameraRight = glm::normalize(glm::cross(m_Up, m_Direction));
        //m_CameraUp = glm::cross(m_Direction, m_CameraRight);
        m_CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        m_CameraFront = glm::vec3(0.0f, 0.0f, -10.0f);

        m_View = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 3.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
    }

    /**
        fovy : viertical field of view
        s : aspect ratio viewport
        n : near distance
        f : far distance
    **/
    glm::mat4 Camera::FrustumProj(float fovy, float s, float n, float f)
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

    void Camera::Forward()
    {
        m_Pos += m_Speed * m_CameraFront;
    }

    void Camera::Backward()
    {
        m_Pos -= m_Speed * m_CameraFront;
    }
    void Camera::Up()
    {
        m_Pos -= m_CameraUp * m_Speed;
    }

    void Camera::Down()
    {
        m_Pos += m_CameraUp * m_Speed;
    }

    void Camera::Left()
    {
        m_Pos -= glm::normalize(glm::cross(m_CameraFront, m_CameraUp)) * m_Speed;
    }

    void Camera::Right()
    {
        m_Pos += glm::normalize(glm::cross(m_CameraFront, m_CameraUp)) * m_Speed;
    }

    glm::mat4 Camera::LookAt()
    {
        m_View = glm::lookAt(m_Pos, m_Pos + m_CameraFront, m_CameraUp);
        return m_View;
    }

    void Camera::UpdateSpeed(float timeStep)
    {
        m_Speed = 10.0f * timeStep;
    }

    void Camera::UpdateYP(float xoffset, float yoffset)
    {
        //Rbk::Log::GetLogger()->debug("Mouse xoffset {} yoffset {}", xoffset, yoffset);

        m_Yaw += xoffset;
        m_Pitch += yoffset;

        if (m_Pitch > 89.0f) {
            m_Pitch = 89.0f;
        }
        if (m_Pitch < -89.0f) {
            m_Pitch = -89.0f;
        }

        glm::vec3 direction;
        direction.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        direction.y = sin(glm::radians(m_Pitch));
        direction.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        m_CameraFront = glm::normalize(direction);
    }

    void Camera::Recenter()
    {
        m_CameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    }
}
