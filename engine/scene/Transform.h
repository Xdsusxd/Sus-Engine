#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Engine {

struct Transform {
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 rotation = {0.0f, 0.0f, 0.0f}; // euler degrees (pitch, yaw, roll)
    glm::vec3 scale    = {1.0f, 1.0f, 1.0f};

    glm::mat4 GetMatrix() const {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), position);
        m = glm::rotate(m, glm::radians(rotation.y), {0, 1, 0}); // Yaw
        m = glm::rotate(m, glm::radians(rotation.x), {1, 0, 0}); // Pitch
        m = glm::rotate(m, glm::radians(rotation.z), {0, 0, 1}); // Roll
        m = glm::scale(m, scale);
        return m;
    }
};

} // namespace Engine
