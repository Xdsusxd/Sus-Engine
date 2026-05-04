#pragma once

#include <glm/glm.hpp>
#include "physics/PhysicsBody.h"
#include "scene/Component.h"

namespace Engine {

class Input;
class Camera;
struct SceneObject;

class CharacterController : public Component {
public:
    CharacterController(Camera* camera, Input* input) : m_Camera(camera), m_Input(input) {}

    void OnStart() override;
    void OnUpdate(float dt) override;

    // Configurable parameters
    float moveSpeed        = 5.0f;
    float sprintMultiplier = 2.0f;
    float rotationSpeed    = 10.0f;
    float jumpVelocity     = 6.0f;

private:
    Camera*       m_Camera = nullptr;
    Input*        m_Input = nullptr;
    PhysicsBodyID m_BodyID;
    bool          m_IsGrounded = true;
};

} // namespace Engine
