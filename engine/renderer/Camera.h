#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Engine {

class Input;

enum class CameraMode {
    Free,
    ThirdPerson
};

class Camera {
public:
    void Init(float fovDegrees, float aspect, float nearPlane, float farPlane);

    void Update(const Input& input, float dt);

    void SetAspect(float aspect) { m_Aspect = aspect; UpdateProjection(); }

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const { return m_Projection; }
    glm::mat4 GetViewProjection() const { return m_Projection * GetViewMatrix(); }

    glm::vec3 GetPosition() const { return m_Position; }
    
    // ── Third Person specific ─────────────────────────────────
    void SetMode(CameraMode mode) { m_Mode = mode; }
    void SetTarget(const glm::vec3& target) { m_Target = target; }
    void SetDistance(float distance) { m_Distance = distance; }

    // ── Config ────────────────────────────────────────────────
    float moveSpeed    = 3.0f;
    float lookSpeed    = 0.15f;
    float scrollSpeed  = 2.0f;
    float orbitSpeed   = 0.3f;
    float minDistance  = 1.0f;
    float maxDistance  = 20.0f;

private:
    void UpdateProjection();

    CameraMode m_Mode = CameraMode::Free;

    // Free camera
    glm::vec3 m_Position = {0.0f, 1.0f, 3.0f};
    
    // Third Person camera
    glm::vec3 m_Target   = {0.0f, 0.0f, 0.0f};
    float     m_Distance = 5.0f;

    // Shared
    float     m_Yaw      = -90.0f; // Look towards -Z
    float     m_Pitch    = -15.0f;

    float m_Fov   = 60.0f;
    float m_Aspect = 16.0f / 9.0f;
    float m_Near   = 0.01f;
    float m_Far    = 1000.0f;

    glm::mat4 m_Projection{1.0f};
};

} // namespace Engine
