#pragma once

#include <glm/glm.hpp>
#include <cstdint>

// Forward declarations
namespace JPH {
    class BodyID;
    class Body;
}

namespace Engine {

enum class PhysicsBodyType {
    Static,
    Dynamic,
    Kinematic
};

struct PhysicsBodyDesc {
    PhysicsBodyType type = PhysicsBodyType::Dynamic;
    glm::vec3 position   = {0.0f, 0.0f, 0.0f};
    glm::vec3 halfExtent = {0.5f, 0.5f, 0.5f};  // For box shapes
    float     radius     = 0.5f;                   // For sphere shapes
    float     mass       = 1.0f;
    float     friction   = 0.5f;
    float     restitution = 0.3f;
    bool      isSphere   = false;
};

// Opaque handle wrapping a Jolt BodyID
struct PhysicsBodyID {
    uint32_t index = 0;
    uint32_t sequence = 0;
    bool valid = false;
};

// Utility to convert between GLM and Jolt
glm::vec3 PhysicsGetPosition(const PhysicsBodyID& id);
glm::vec3 PhysicsGetRotationEuler(const PhysicsBodyID& id);
void      PhysicsSetLinearVelocity(const PhysicsBodyID& id, const glm::vec3& vel);
glm::vec3 PhysicsGetLinearVelocity(const PhysicsBodyID& id);

} // namespace Engine
