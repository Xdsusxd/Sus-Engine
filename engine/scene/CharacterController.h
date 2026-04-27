#pragma once

#include <glm/glm.hpp>

namespace Engine {

class Input;
class Camera;
struct SceneObject;

class CharacterController {
public:
    void Init(SceneObject* playerObj, Camera* camera);
    void Update(const Input& input, float dt);

    // Configurable parameters
    float moveSpeed     = 5.0f;
    float sprintMultiplier = 2.0f;
    float rotationSpeed = 10.0f;
    float jumpVelocity  = 6.0f;
    float gravity       = -15.0f;

private:
    SceneObject* m_Player = nullptr;
    Camera*      m_Camera = nullptr;

    glm::vec3    m_Velocity = {0.0f, 0.0f, 0.0f};
    bool         m_IsGrounded = true;
};

} // namespace Engine
