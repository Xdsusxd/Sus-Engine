#pragma once

#include "renderer/Buffer.h"
#include "renderer/ParticlePipeline.h"
#include <vector>
#include <glm/glm.hpp>

namespace Engine {

class Renderer;

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec4 color;
    float size;
    float sizeBegin, sizeEnd;
    float lifeTime;
    float lifeRemaining;
    bool  active = false;
};

struct ParticleProps {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 velocityVariation;
    glm::vec4 colorBegin;
    glm::vec4 colorEnd;
    float sizeBegin;
    float sizeEnd;
    float sizeVariation;
    float lifeTime;
};

class ParticleSystem {
public:
    static ParticleSystem& Get() {
        static ParticleSystem instance;
        return instance;
    }

    void Init(VmaAllocator allocator, uint32_t maxParticles = 1000);
    void Shutdown();

    void Update(float dt);
    void Render(VkCommandBuffer cmd, const ParticlePipeline& pipeline, const ParticlePushConstants& pc);

    void Emit(const ParticleProps& particleProps);

private:
    ParticleSystem() = default;
    ~ParticleSystem() = default;

    ParticleSystem(const ParticleSystem&) = delete;
    ParticleSystem& operator=(const ParticleSystem&) = delete;

    std::vector<Particle>         m_ParticlePool;
    uint32_t                      m_PoolIndex = 999;
    
    // For rendering
    std::vector<ParticleInstance> m_InstanceData;
    Buffer                        m_InstanceBuffer;
    VmaAllocator                  m_Allocator = VK_NULL_HANDLE;
};

} // namespace Engine
