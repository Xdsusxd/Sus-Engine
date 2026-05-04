#include "ai/EnemyAIComponent.h"
#include "physics/PhysicsSystem.h"
#include "audio/AudioSystem.h"
#include "core/Logger.h"
#include <glm/gtc/random.hpp>
#include <cmath>

namespace Engine {

void EnemyAIComponent::OnStart() {
    if (!m_Owner) return;

    m_BasePosition = m_Owner->transform.position;
    m_State = AIState::IDLE;
    m_StateTimer = glm::linearRand(1.0f, 3.0f);

    // Create physics body
    PhysicsBodyDesc desc;
    desc.type       = PhysicsBodyType::Dynamic;
    desc.position   = m_Owner->transform.position;
    desc.halfExtent = m_Owner->transform.scale * 0.5f;
    desc.mass       = 80.0f;
    desc.friction   = 0.5f;
    desc.restitution = 0.0f;
    
    m_BodyID = PhysicsSystem::Get().CreateBody(desc);
    m_Owner->physicsBody = m_BodyID;
    
    LOG_INFO("EnemyAI", "{} initialized at ({:.1f}, {:.1f}, {:.1f})", 
             m_Owner->name, m_BasePosition.x, m_BasePosition.y, m_BasePosition.z);
}

void EnemyAIComponent::OnUpdate(float dt) {
    if (!m_Owner || !m_Target || !m_BodyID.valid) return;

    // State machine evaluation
    float distanceToTarget = glm::distance(m_Owner->transform.position, m_Target->transform.position);

    if (distanceToTarget <= attackRadius) {
        m_State = AIState::ATTACK;
    } else if (distanceToTarget <= detectionRadius) {
        m_State = AIState::CHASE;
    } else if (m_State == AIState::CHASE || m_State == AIState::ATTACK) {
        // Lost target, go back to patrol
        m_State = AIState::IDLE;
        m_StateTimer = 1.0f;
        LOG_INFO("EnemyAI", "{} lost target. Returning to idle.", m_Owner->name);
    }

    // Execute state logic
    switch (m_State) {
        case AIState::IDLE:   UpdateIdle(dt); break;
        case AIState::PATROL: UpdatePatrol(dt); break;
        case AIState::CHASE:  UpdateChase(dt); break;
        case AIState::ATTACK: UpdateAttack(dt); break;
    }

    // Sync transform with physics body
    m_Owner->transform.position = PhysicsGetPosition(m_BodyID);
}

void EnemyAIComponent::UpdateIdle(float dt) {
    // Stop moving horizontally
    glm::vec3 vel = PhysicsGetLinearVelocity(m_BodyID);
    vel.x = 0.0f; vel.z = 0.0f;
    PhysicsSetLinearVelocity(m_BodyID, vel);

    m_StateTimer -= dt;
    if (m_StateTimer <= 0.0f) {
        m_State = AIState::PATROL;
        // Pick a random patrol point within 5 units of base position
        glm::vec2 offset = glm::circularRand(5.0f);
        m_PatrolTarget = m_BasePosition + glm::vec3(offset.x, 0.0f, offset.y);
        LOG_INFO("EnemyAI", "{} patrolling to ({:.1f}, {:.1f})", m_Owner->name, m_PatrolTarget.x, m_PatrolTarget.z);
    }
}

void EnemyAIComponent::UpdatePatrol(float dt) {
    MoveTowards(m_PatrolTarget, patrolSpeed, dt);

    float distToTarget = glm::length(glm::vec2(m_Owner->transform.position.x - m_PatrolTarget.x, 
                                               m_Owner->transform.position.z - m_PatrolTarget.z));
    if (distToTarget < 0.5f) {
        m_State = AIState::IDLE;
        m_StateTimer = glm::linearRand(2.0f, 4.0f);
    }
}

void EnemyAIComponent::UpdateChase(float dt) {
    MoveTowards(m_Target->transform.position, chaseSpeed, dt);
}

void EnemyAIComponent::UpdateAttack(float dt) {
    // Stop moving and attack
    glm::vec3 vel = PhysicsGetLinearVelocity(m_BodyID);
    vel.x = 0.0f; vel.z = 0.0f;
    PhysicsSetLinearVelocity(m_BodyID, vel);

    m_StateTimer -= dt;
    if (m_StateTimer <= 0.0f) {
        LOG_INFO("EnemyAI", "{} attacks {}!", m_Owner->name, m_Target->name);
        AudioSystem::Get().PlaySound3D("assets/audio/attack.wav", m_Owner->transform.position);
        m_StateTimer = 1.5f; // attack cooldown
    }
}

void EnemyAIComponent::MoveTowards(const glm::vec3& targetPos, float speed, float dt) {
    glm::vec3 dir = targetPos - m_Owner->transform.position;
    dir.y = 0.0f; // Only move horizontally

    glm::vec3 desiredVel = PhysicsGetLinearVelocity(m_BodyID);

    if (glm::length(dir) > 0.01f) {
        dir = glm::normalize(dir);
        desiredVel.x = dir.x * speed;
        desiredVel.z = dir.z * speed;

        // Smoothly rotate
        float targetYaw = glm::degrees(atan2f(-dir.z, dir.x)) + 90.0f;
        float currentYaw = m_Owner->transform.rotation.y;
        
        float diff = targetYaw - currentYaw;
        while (diff < -180.0f) diff += 360.0f;
        while (diff >  180.0f) diff -= 360.0f;
        
        m_Owner->transform.rotation.y += diff * 8.0f * dt;
    } else {
        desiredVel.x = 0.0f;
        desiredVel.z = 0.0f;
    }

    PhysicsSetLinearVelocity(m_BodyID, desiredVel);
}

} // namespace Engine
