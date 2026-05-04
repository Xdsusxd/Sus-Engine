#include "scene/CharacterController.h"
#include "scene/SceneObject.h"
#include "renderer/Camera.h"
#include "core/Input.h"
#include "physics/PhysicsSystem.h"
#include "physics/PhysicsBody.h"

#include <cmath>
#include <algorithm>

namespace Engine {

void CharacterController::OnStart() {
    m_IsGrounded = true;

    if (!m_Owner) return;

    // Create a dynamic physics body for the player
    PhysicsBodyDesc desc;
    desc.type       = PhysicsBodyType::Dynamic;
    desc.position   = m_Owner->transform.position;
    desc.halfExtent = m_Owner->transform.scale * 0.5f;
    desc.mass       = 70.0f;  // ~human mass in kg
    desc.friction   = 0.8f;
    desc.restitution = 0.0f;  // No bouncing
    
    m_BodyID = PhysicsSystem::Get().CreateBody(desc);
    m_Owner->physicsBody = m_BodyID;
}

void CharacterController::OnUpdate(float dt) {
    if (!m_Owner || !m_Camera || !m_Input || !m_BodyID.valid) return;

    // ── 1. Calculate input movement relative to camera ────────
    float currentSpeed = moveSpeed;
    if (m_Input->IsKeyDown(Key::LeftShift)) currentSpeed *= sprintMultiplier;

    glm::vec3 camFwd = m_Camera->GetTarget() - m_Camera->GetPosition();
    camFwd.y = 0.0f;
    if (glm::length(camFwd) > 0.001f) {
        camFwd = glm::normalize(camFwd);
    } else {
        camFwd = glm::vec3(0, 0, -1);
    }

    glm::vec3 camRight = glm::normalize(glm::cross(camFwd, glm::vec3(0, 1, 0)));

    glm::vec3 moveDir(0.0f);
    if (m_Input->IsKeyDown(Key::W)) moveDir += camFwd;
    if (m_Input->IsKeyDown(Key::S)) moveDir -= camFwd;
    if (m_Input->IsKeyDown(Key::A)) moveDir -= camRight;
    if (m_Input->IsKeyDown(Key::D)) moveDir += camRight;

    // ── 2. Get current physics velocity ───────────────────────
    glm::vec3 currentVel = PhysicsGetLinearVelocity(m_BodyID);

    // ── 3. Apply horizontal movement ──────────────────────────
    glm::vec3 desiredVel = currentVel;
    
    if (glm::length(moveDir) > 0.0f) {
        moveDir = glm::normalize(moveDir);
        
        desiredVel.x = moveDir.x * currentSpeed;
        desiredVel.z = moveDir.z * currentSpeed;

        // Smoothly rotate character to face movement direction
        float targetYaw = glm::degrees(atan2f(-moveDir.z, moveDir.x)) + 90.0f;
        float currentYaw = m_Owner->transform.rotation.y;
        
        float diff = targetYaw - currentYaw;
        while (diff < -180.0f) diff += 360.0f;
        while (diff >  180.0f) diff -= 360.0f;
        
        m_Owner->transform.rotation.y += diff * rotationSpeed * dt;
    } else {
        // Stop horizontal movement (keep vertical from gravity)
        desiredVel.x = 0.0f;
        desiredVel.z = 0.0f;
    }

    // ── 4. Ground check (simple Y-based) ──────────────────────
    glm::vec3 physPos = PhysicsGetPosition(m_BodyID);
    float halfHeight = m_Owner->transform.scale.y * 0.5f;
    m_IsGrounded = (physPos.y <= halfHeight + 0.05f);

    // ── 5. Jumping ────────────────────────────────────────────
    if (m_IsGrounded && m_Input->IsKeyPressed(Key::Space)) {
        desiredVel.y = jumpVelocity;
    }
    // else keep currentVel.y (gravity from Jolt)

    // ── 6. Apply velocity to physics body ─────────────────────
    PhysicsSetLinearVelocity(m_BodyID, desiredVel);

    // ── 7. Sync physics position back to scene transform ──────
    m_Owner->transform.position = physPos;

    // ── 8. Update Camera Target ───────────────────────────────
    m_Camera->SetTarget(m_Owner->transform.position);
}

} // namespace Engine
