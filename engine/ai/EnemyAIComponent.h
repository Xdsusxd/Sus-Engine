#pragma once

#include "scene/Component.h"
#include "scene/SceneObject.h"
#include "physics/PhysicsBody.h"
#include <glm/glm.hpp>
#include <string>

namespace Engine {

enum class AIState {
    IDLE,
    PATROL,
    CHASE,
    ATTACK
};

class EnemyAIComponent : public Component {
public:
    EnemyAIComponent(SceneObject* target) : m_Target(target) {}

    void OnStart() override;
    void OnUpdate(float dt) override;

    // AI parameters
    float detectionRadius = 10.0f;
    float attackRadius    = 2.0f;
    float patrolSpeed     = 2.0f;
    float chaseSpeed      = 4.5f;

private:
    void UpdateIdle(float dt);
    void UpdatePatrol(float dt);
    void UpdateChase(float dt);
    void UpdateAttack(float dt);

    void MoveTowards(const glm::vec3& targetPos, float speed, float dt);

    SceneObject* m_Target = nullptr;
    AIState      m_State  = AIState::IDLE;
    
    glm::vec3    m_BasePosition;
    glm::vec3    m_PatrolTarget;
    float        m_StateTimer = 0.0f;

    PhysicsBodyID m_BodyID;
};

} // namespace Engine
