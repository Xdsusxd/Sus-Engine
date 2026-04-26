#include "renderer/Camera.h"
#include "core/Input.h"

#include <algorithm>

namespace Engine {

void Camera::Init(float fovDegrees, float aspect, float nearPlane, float farPlane) {
    m_Fov    = fovDegrees;
    m_Aspect = aspect;
    m_Near   = nearPlane;
    m_Far    = farPlane;
    UpdateProjection();
}

void Camera::Update(const Input& input, float dt) {
    if (m_Mode == CameraMode::Free) {
        // ── Free Camera (FPS Style) ───────────────────────────
        if (input.IsMouseDown(MouseButton::Right)) {
            float dx = static_cast<float>(input.GetMouseDeltaX());
            float dy = static_cast<float>(input.GetMouseDeltaY());

            m_Yaw   += dx * lookSpeed;
            m_Pitch -= dy * lookSpeed;
            m_Pitch  = std::clamp(m_Pitch, -89.0f, 89.0f);
        }

        float yawRad   = glm::radians(m_Yaw);
        float pitchRad = glm::radians(m_Pitch);

        glm::vec3 forward;
        forward.x = cosf(pitchRad) * cosf(yawRad);
        forward.y = sinf(pitchRad);
        forward.z = cosf(pitchRad) * sinf(yawRad);
        forward = glm::normalize(forward);

        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
        glm::vec3 up    = glm::normalize(glm::cross(right, forward));

        float speed = moveSpeed * dt;
        if (input.IsKeyDown(Key::LeftShift)) speed *= 2.5f;

        if (input.IsKeyDown(Key::W)) m_Position += forward * speed;
        if (input.IsKeyDown(Key::S)) m_Position -= forward * speed;
        if (input.IsKeyDown(Key::A)) m_Position -= right   * speed;
        if (input.IsKeyDown(Key::D)) m_Position += right   * speed;
        if (input.IsKeyDown(Key::E) || input.IsKeyDown(Key::Space))   m_Position += up * speed;
        if (input.IsKeyDown(Key::Q) || input.IsKeyDown(Key::LeftControl)) m_Position -= up * speed;

        float scroll = static_cast<float>(input.GetScrollDelta());
        if (scroll != 0.0f) {
            m_Position += forward * scroll * scrollSpeed;
        }
    } 
    else if (m_Mode == CameraMode::ThirdPerson) {
        // ── Third Person Camera (Orbit) ───────────────────────
        if (input.IsMouseDown(MouseButton::Right)) {
            float dx = static_cast<float>(input.GetMouseDeltaX());
            float dy = static_cast<float>(input.GetMouseDeltaY());

            m_Yaw   += dx * orbitSpeed;
            m_Pitch -= dy * orbitSpeed;
            m_Pitch  = std::clamp(m_Pitch, -89.0f, 89.0f);
        }

        // Scroll zoom modifies distance
        float scroll = static_cast<float>(input.GetScrollDelta());
        if (scroll != 0.0f) {
            m_Distance -= scroll * scrollSpeed;
            m_Distance  = std::clamp(m_Distance, minDistance, maxDistance);
        }

        // Calculate position based on target and spherical coordinates
        float yawRad   = glm::radians(m_Yaw);
        float pitchRad = glm::radians(m_Pitch);

        glm::vec3 offset;
        offset.x = m_Distance * cosf(pitchRad) * cosf(yawRad);
        offset.y = m_Distance * sinf(pitchRad);
        offset.z = m_Distance * cosf(pitchRad) * sinf(yawRad);

        // Basic offset check (prevent camera going too far below the target)
        // This is a naive "raycast" to a plane just slightly below target.y
        if (offset.y < -m_Distance * 0.8f) {
            // Keep the camera from dipping entirely under the floor
             offset.y = -m_Distance * 0.8f;
        }

        // The camera position is orbit offset BACKWARDS from the target
        // Wait, standard spherical coords above usually point FROM origin to POS.
        // If the camera looks at Target, the camera position is Target - offset.
        m_Position = m_Target - offset;
    }
}

glm::mat4 Camera::GetViewMatrix() const {
    float yawRad   = glm::radians(m_Yaw);
    float pitchRad = glm::radians(m_Pitch);

    glm::vec3 forward;
    forward.x = cosf(pitchRad) * cosf(yawRad);
    forward.y = sinf(pitchRad);
    forward.z = cosf(pitchRad) * sinf(yawRad);
    forward = glm::normalize(forward);

    if (m_Mode == CameraMode::ThirdPerson) {
        return glm::lookAt(m_Position, m_Target, glm::vec3(0, 1, 0));
    }

    return glm::lookAt(m_Position, m_Position + forward, glm::vec3(0, 1, 0));
}

void Camera::UpdateProjection() {
    m_Projection = glm::perspective(glm::radians(m_Fov), m_Aspect, m_Near, m_Far);
    m_Projection[1][1] *= -1; // Flip Y for Vulkan (clip space Y is inverted vs OpenGL)
}

} // namespace Engine
