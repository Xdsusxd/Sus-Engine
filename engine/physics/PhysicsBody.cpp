#include "physics/PhysicsBody.h"
#include "physics/PhysicsSystem.h"
#include "core/Logger.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

#include <glm/gtc/quaternion.hpp>

namespace Engine {

// ── Layer constants (must match PhysicsSystem.cpp) ─────────────────────────
namespace Layers {
    static constexpr JPH::ObjectLayer NON_MOVING = 0;
    static constexpr JPH::ObjectLayer MOVING = 1;
}

// ── Helper: JPH::Vec3 <-> glm::vec3 ──────────────────────────────────────
static JPH::Vec3 ToJolt(const glm::vec3& v) { return JPH::Vec3(v.x, v.y, v.z); }
static glm::vec3 ToGLM(const JPH::Vec3& v)  { return glm::vec3(v.GetX(), v.GetY(), v.GetZ()); }

static JPH::BodyID ToJoltID(const PhysicsBodyID& id) {
    return JPH::BodyID(id.index | (id.sequence << 24));
}

static PhysicsBodyID FromJoltID(const JPH::BodyID& id) {
    PhysicsBodyID result;
    result.index = id.GetIndex();
    result.sequence = id.GetSequenceNumber();
    result.valid = !id.IsInvalid();
    return result;
}

// ── Quaternion to Euler (degrees) ─────────────────────────────────────────
static glm::vec3 QuatToEulerDeg(const JPH::Quat& q) {
    glm::quat gq(q.GetW(), q.GetX(), q.GetY(), q.GetZ());
    glm::vec3 euler = glm::degrees(glm::eulerAngles(gq));
    return euler;
}

// ── Public API ────────────────────────────────────────────────────────────

glm::vec3 PhysicsGetPosition(const PhysicsBodyID& id) {
    if (!id.valid) return glm::vec3(0.0f);
    auto* sys = PhysicsSystem::Get().GetJoltSystem();
    if (!sys) return glm::vec3(0.0f);
    const auto& bi = sys->GetBodyInterface();
    return ToGLM(bi.GetCenterOfMassPosition(ToJoltID(id)));
}

glm::vec3 PhysicsGetRotationEuler(const PhysicsBodyID& id) {
    if (!id.valid) return glm::vec3(0.0f);
    auto* sys = PhysicsSystem::Get().GetJoltSystem();
    if (!sys) return glm::vec3(0.0f);
    const auto& bi = sys->GetBodyInterface();
    return QuatToEulerDeg(bi.GetRotation(ToJoltID(id)));
}

void PhysicsSetLinearVelocity(const PhysicsBodyID& id, const glm::vec3& vel) {
    if (!id.valid) return;
    auto* sys = PhysicsSystem::Get().GetJoltSystem();
    if (!sys) return;
    auto& bi = sys->GetBodyInterface();
    bi.SetLinearVelocity(ToJoltID(id), ToJolt(vel));
}

glm::vec3 PhysicsGetLinearVelocity(const PhysicsBodyID& id) {
    if (!id.valid) return glm::vec3(0.0f);
    auto* sys = PhysicsSystem::Get().GetJoltSystem();
    if (!sys) return glm::vec3(0.0f);
    const auto& bi = sys->GetBodyInterface();
    return ToGLM(bi.GetLinearVelocity(ToJoltID(id)));
}

} // namespace Engine
