#pragma once

#include "scene/Transform.h"
#include "scene/Material.h"
#include <string>

namespace Engine {

class Mesh;

struct SceneObject {
    std::string name     = "Object";
    Transform   transform;
    Material    material;
    Mesh*       mesh     = nullptr;  // shared, NOT owned
    bool        visible  = true;
    bool        castShadow = true;

    // Optional per-frame rotation for animation
    glm::vec3   autoRotate = {0.0f, 0.0f, 0.0f}; // degrees per second
};

} // namespace Engine
