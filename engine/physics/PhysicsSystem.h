#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <memory>

// Forward declarations to avoid including Jolt headers in Engine headers
namespace JPH {
    class PhysicsSystem;
    class JobSystemThreadPool;
    class TempAllocatorImpl;
}

namespace Engine {

class PhysicsSystem {
public:
    static PhysicsSystem& Get() {
        static PhysicsSystem instance;
        return instance;
    }

    bool Init();
    void Update(float dt);
    void Shutdown();

    JPH::PhysicsSystem* GetJoltSystem() const { return m_PhysicsSystem.get(); }

private:
    PhysicsSystem();
    ~PhysicsSystem();

    std::unique_ptr<JPH::PhysicsSystem>       m_PhysicsSystem;
    std::unique_ptr<JPH::JobSystemThreadPool> m_JobSystem;
    std::unique_ptr<JPH::TempAllocatorImpl>   m_TempAllocator;
};

} // namespace Engine
