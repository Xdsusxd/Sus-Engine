#include "scene/CharacterController.h"
#include "scene/SceneObject.h"
#include "renderer/Camera.h"
#include "core/Input.h"

#include <cmath>
#include <algorithm>

namespace Engine {

void CharacterController::Init(SceneObject* playerObj, Camera* camera) {
    m_Player = playerObj;
    m_Camera = camera;
    m_Velocity = {0.0f, 0.0f, 0.0f};
    m_IsGrounded = true;
}

void CharacterController::Update(const Input& input, float dt) {
    if (!m_Player || !m_Camera) return;

    // ── 1. Calculate input movement relative to camera ────────
    float currentSpeed = moveSpeed;
    if (input.IsKeyDown(Key::LeftShift)) currentSpeed *= sprintMultiplier;

    glm::vec3 camFwd = m_Camera->GetTarget() - m_Camera->GetPosition();
    camFwd.y = 0.0f;
    if (glm::length(camFwd) > 0.001f) {
        camFwd = glm::normalize(camFwd);
    } else {
        camFwd = glm::vec3(0, 0, -1);
    }

    glm::vec3 camRight = glm::normalize(glm::cross(camFwd, glm::vec3(0, 1, 0)));

    glm::vec3 moveDir(0.0f);
    if (input.IsKeyDown(Key::W)) moveDir += camFwd;
    if (input.IsKeyDown(Key::S)) moveDir -= camFwd;
    if (input.IsKeyDown(Key::A)) moveDir -= camRight;
    if (input.IsKeyDown(Key::D)) moveDir += camRight;

    if (glm::length(moveDir) > 0.0f) {
        moveDir = glm::normalize(moveDir);
        
        // Update horizontal velocity
        m_Velocity.x = moveDir.x * currentSpeed;
        m_Velocity.z = moveDir.z * currentSpeed;

        // Smoothly rotate character to face movement direction
        float targetYaw = glm::degrees(atan2f(-moveDir.z, moveDir.x)) + 90.0f;
        float currentYaw = m_Player->transform.rotation.y;
        
        float diff = targetYaw - currentYaw;
        while (diff < -180.0f) diff += 360.0f;
        while (diff >  180.0f) diff -= 360.0f;
        
        m_Player->transform.rotation.y += diff * rotationSpeed * dt;
    } else {
        // Friction / stop when no input
        m_Velocity.x = 0.0f;
        m_Velocity.z = 0.0f;
    }

    // ── 2. Jumping and Gravity ────────────────────────────────
    if (m_IsGrounded && input.IsKeyPressed(Key::Space)) {
        m_Velocity.y = jumpVelocity;
        m_IsGrounded = false;
    }

    if (!m_IsGrounded) {
        m_Velocity.y += gravity * dt;
    }

    // ── 3. Apply Velocity and Collision (Floor) ───────────────
    m_Player->transform.position += m_Velocity * dt;

    // Extremely basic floor collision (assume floor is at y = 0.5 for cube center)
    // For a unit cube centered at origin, its bottom is at y = -0.5. If scaled to 1, bottom is at position.y - 0.5.
    // Let's assume the origin of the mesh is at its center, so it rests on y = 0 when pos.y = 0.5
    float floorY = 0.5f; 
    
    if (m_Player->transform.position.y <= floorY) {
        m_Player->transform.position.y = floorY;
        m_Velocity.y = 0.0f;
        m_IsGrounded = true;
    } else {
        m_IsGrounded = false;
    }

    // ── 4. Update Camera Target ───────────────────────────────
    m_Camera->SetTarget(m_Player->transform.position);
}

} // namespace Engine
