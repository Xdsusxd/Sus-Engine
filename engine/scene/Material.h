#pragma once

#include <glm/glm.hpp>

namespace Engine {

class Texture;

struct Material {
    glm::vec3 albedo  = {1.0f, 1.0f, 1.0f};
    float     metallic  = 0.0f;
    float     roughness = 0.5f;
    Texture*  texture   = nullptr; // optional albedo texture
};

} // namespace Engine
