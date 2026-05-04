#include "physics/PhysicsSystem.h"
#include "core/Logger.h"

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

#include <cstdarg>
#include <thread>

namespace Engine {

// ── Jolt Callbacks ─────────────────────────────────────────────────────────

static void TraceImpl(const char* inFMT, ...) {
    va_list list;
    va_start(list, inFMT);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), inFMT, list);
    va_end(list);
    LOG_INFO("Jolt", "{}", buffer);
}

static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, JPH::uint inLine) {
    LOG_ERROR("Jolt", "{}:{}: ({}) {}", inFile, inLine, inExpression, (inMessage ? inMessage : ""));
    return true; // Break into debugger
}

// ── Layer Definitions ──────────────────────────────────────────────────────

namespace Layers {
    static constexpr JPH::ObjectLayer NON_MOVING = 0;
    static constexpr JPH::ObjectLayer MOVING = 1;
    static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
}

namespace BroadPhaseLayers {
    static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
    static constexpr JPH::BroadPhaseLayer MOVING(1);
    static constexpr JPH::uint NUM_LAYERS(2);
}

// ── Interfaces ─────────────────────────────────────────────────────────────

class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
public:
    BPLayerInterfaceImpl() {
        mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
        mObjectToBroadPhase[Layers::MOVING]     = BroadPhaseLayers::MOVING;
    }
    virtual JPH::uint GetNumBroadPhaseLayers() const override { return BroadPhaseLayers::NUM_LAYERS; }
    virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override {
        return mObjectToBroadPhase[inLayer];
    }
    virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override {
        switch ((JPH::BroadPhaseLayer::Type)inLayer) {
            case (JPH::BroadPhaseLayer::Type)0: return "NON_MOVING";
            case (JPH::BroadPhaseLayer::Type)1: return "MOVING";
            default: return "UNKNOWN";
        }
    }
private:
    JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
};

class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
public:
    virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override {
        switch (inLayer1) {
            case Layers::NON_MOVING: return inLayer2 == BroadPhaseLayers::MOVING;
            case Layers::MOVING:     return true;
            default: return false;
        }
    }
};

class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
public:
    virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override {
        switch (inObject1) {
            case Layers::NON_MOVING: return inObject2 == Layers::MOVING;
            case Layers::MOVING:     return true;
            default: return false;
        }
    }
};

// ── Static instances ───────────────────────────────────────────────────────

static BPLayerInterfaceImpl broadPhaseLayerInterface;
static ObjectVsBroadPhaseLayerFilterImpl objectVsBroadphaseLayerFilter;
static ObjectLayerPairFilterImpl objectVsObjectLayerFilter;

// ── Helper ─────────────────────────────────────────────────────────────────

static JPH::Vec3 ToJolt(const glm::vec3& v) { return JPH::Vec3(v.x, v.y, v.z); }

static PhysicsBodyID FromJoltID(const JPH::BodyID& id) {
    PhysicsBodyID result;
    result.index = id.GetIndex();
    result.sequence = id.GetSequenceNumber();
    result.valid = !id.IsInvalid();
    return result;
}

static JPH::BodyID ToJoltID(const PhysicsBodyID& id) {
    return JPH::BodyID(id.index | (id.sequence << 24));
}

// ── Init / Update / Shutdown ───────────────────────────────────────────────

bool PhysicsSystem::Init() {
    JPH::RegisterDefaultAllocator();

    JPH::Trace = TraceImpl;
    JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl;)

    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();

    m_TempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
    m_JobSystem = std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

    m_PhysicsSystem = std::make_unique<JPH::PhysicsSystem>();
    m_PhysicsSystem->Init(1024, 0, 1024, 1024, 
                          broadPhaseLayerInterface,
                          objectVsBroadphaseLayerFilter,
                          objectVsObjectLayerFilter);

    // Set reasonable gravity
    m_PhysicsSystem->SetGravity(JPH::Vec3(0.0f, -9.81f, 0.0f));

    LOG_INFO("Physics", "Jolt Physics initialized successfully");
    return true;
}

void PhysicsSystem::Update(float dt) {
    if (!m_PhysicsSystem) return;

    // Jolt recommends a fixed timestep; clamp to avoid spiral of death
    float clampedDt = std::min(dt, 1.0f / 30.0f);
    int numSteps = static_cast<int>(std::ceil(clampedDt / (1.0f / 60.0f)));
    if (numSteps < 1) numSteps = 1;

    m_PhysicsSystem->Update(clampedDt, numSteps, m_TempAllocator.get(), m_JobSystem.get());
}

void PhysicsSystem::Shutdown() {
    LOG_INFO("Physics", "Shutting down Jolt Physics...");
    m_PhysicsSystem.reset();
    m_JobSystem.reset();
    m_TempAllocator.reset();

    JPH::UnregisterTypes();
    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
}

// ── Body Creation / Removal ────────────────────────────────────────────────

PhysicsBodyID PhysicsSystem::CreateBody(const PhysicsBodyDesc& desc) {
    if (!m_PhysicsSystem) return {};

    auto& bi = m_PhysicsSystem->GetBodyInterface();

    // Create shape
    JPH::ShapeRefC shape;
    if (desc.isSphere) {
        shape = new JPH::SphereShape(desc.radius);
    } else {
        shape = new JPH::BoxShape(ToJolt(desc.halfExtent));
    }

    // Determine motion type and layer
    JPH::EMotionType motionType;
    JPH::ObjectLayer layer;
    JPH::EActivation activation;
    
    switch (desc.type) {
        case PhysicsBodyType::Static:
            motionType = JPH::EMotionType::Static;
            layer = Layers::NON_MOVING;
            activation = JPH::EActivation::DontActivate;
            break;
        case PhysicsBodyType::Kinematic:
            motionType = JPH::EMotionType::Kinematic;
            layer = Layers::MOVING;
            activation = JPH::EActivation::Activate;
            break;
        case PhysicsBodyType::Dynamic:
        default:
            motionType = JPH::EMotionType::Dynamic;
            layer = Layers::MOVING;
            activation = JPH::EActivation::Activate;
            break;
    }

    JPH::BodyCreationSettings settings(shape, ToJolt(desc.position),
                                        JPH::Quat::sIdentity(),
                                        motionType, layer);
    settings.mFriction = desc.friction;
    settings.mRestitution = desc.restitution;

    JPH::Body* body = bi.CreateBody(settings);
    if (!body) {
        LOG_ERROR("Physics", "Failed to create physics body!");
        return {};
    }

    bi.AddBody(body->GetID(), activation);

    LOG_INFO("Physics", "Created {} body at ({:.1f}, {:.1f}, {:.1f})",
             desc.type == PhysicsBodyType::Static ? "static" :
             desc.type == PhysicsBodyType::Dynamic ? "dynamic" : "kinematic",
             desc.position.x, desc.position.y, desc.position.z);

    return FromJoltID(body->GetID());
}

void PhysicsSystem::RemoveBody(const PhysicsBodyID& id) {
    if (!id.valid || !m_PhysicsSystem) return;
    
    auto& bi = m_PhysicsSystem->GetBodyInterface();
    JPH::BodyID joltId = ToJoltID(id);
    bi.RemoveBody(joltId);
    bi.DestroyBody(joltId);
}

PhysicsSystem::PhysicsSystem() = default;
PhysicsSystem::~PhysicsSystem() = default;

} // namespace Engine
