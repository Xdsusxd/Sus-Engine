#pragma once

#include "scene/Transform.h"
#include "scene/Material.h"
#include "physics/PhysicsBody.h"
#include "scene/Component.h"
#include <string>
#include <vector>
#include <memory>

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

    // Optional physics body
    PhysicsBodyID physicsBody;

    // Components
    std::vector<std::unique_ptr<Component>> components;

    template<typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        auto comp = std::make_unique<T>(std::forward<Args>(args)...);
        comp->m_Owner = this;
        T* ptr = comp.get();
        components.push_back(std::move(comp));
        ptr->OnStart();
        return ptr;
    }

    template<typename T>
    T* GetComponent() {
        for (auto& comp : components) {
            if (T* ptr = dynamic_cast<T*>(comp.get())) {
                return ptr;
            }
        }
        return nullptr;
    }

    void UpdateComponents(float dt) {
        for (auto& comp : components) {
            comp->OnUpdate(dt);
        }
    }

    ~SceneObject() {
        for (auto& comp : components) {
            comp->OnDestroy();
        }
    }
};

} // namespace Engine

