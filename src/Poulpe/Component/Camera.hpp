#pragma once
#include <glm/glm.hpp>

namespace Poulpe
{
    struct CameraUBO {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
        alignas(16) glm::mat4 projXview;
    };

    class Camera
    {
    public:
        void init();
        inline glm::mat4 getView() { return m_View; }
        void forward();
        void backward();
        void up();
        void down();
        void left();
        void right();
        glm::mat4 lookAt();
        void updateDeltaTime(float timeStep);
        void updateYP(float xoffset, float yoffset);
        void recenter();
        glm::mat4 frustumProj(float fovy, float s, float n, float f);
        glm::vec4 getPos() { return glm::vec4(m_Pos, 1.0f); }

    private:
        glm::vec3 m_Pos;
        glm::vec3 m_Target;
        glm::vec3 m_Direction;
        glm::vec3 m_Up;
        glm::vec3 m_CameraRight;
        glm::vec3 m_CameraUp;
        glm::mat4 m_View;
        glm::vec3 m_CameraFront;
        float m_Yaw = -90.0f;
        float m_Pitch;
        float m_Speed = 1.f;
        float m_DeltaTime = 1.f;
    };
}
