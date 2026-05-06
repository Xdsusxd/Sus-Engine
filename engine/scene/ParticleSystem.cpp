#include "scene/ParticleSystem.h"
#include <glm/gtc/random.hpp>

namespace Engine {

void ParticleSystem::Init(VmaAllocator allocator, uint32_t maxParticles) {
    m_Allocator = allocator;
    m_ParticlePool.resize(maxParticles);
    m_InstanceData.reserve(maxParticles);
    m_PoolIndex = maxParticles - 1;
    
    // Allocate the instance buffer to hold maxParticles of ParticleInstance
    VkDeviceSize bufferSize = sizeof(ParticleInstance) * m_ParticlePool.size();
    m_InstanceBuffer.Create(m_Allocator, bufferSize, BufferUsage::DynamicVertex);
}

void ParticleSystem::Shutdown() {
    m_InstanceBuffer.Destroy();
}

void ParticleSystem::Update(float dt) {
    m_InstanceData.clear();

    for (auto& particle : m_ParticlePool) {
        if (!particle.active) continue;

        if (particle.lifeRemaining <= 0.0f) {
            particle.active = false;
            continue;
        }

        particle.lifeRemaining -= dt;
        particle.position += particle.velocity * dt;

        float life = particle.lifeRemaining / particle.lifeTime; // 1 to 0
        particle.size = glm::mix(particle.sizeEnd, particle.sizeBegin, life);

        ParticleInstance inst;
        inst.position = particle.position;
        inst.size = particle.size;
        inst.color = particle.color; // You could mix color here too
        inst.color.a *= life; // fade out

        m_InstanceData.push_back(inst);
    }

    // Upload instance data if any particles are active
    if (!m_InstanceData.empty()) {
        m_InstanceBuffer.Upload(m_InstanceData.data(), sizeof(ParticleInstance) * m_InstanceData.size());
    }
}

void ParticleSystem::Render(VkCommandBuffer cmd, const ParticlePipeline& pipeline, const ParticlePushConstants& pc) {
    if (m_InstanceData.empty()) return;

    pipeline.Bind(cmd);
    pipeline.PushConstants(cmd, pc);

    VkBuffer vertexBuffers[] = { m_InstanceBuffer.GetHandle() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

    // Draw 6 vertices per instance, with m_InstanceData.size() instances
    vkCmdDraw(cmd, 6, static_cast<uint32_t>(m_InstanceData.size()), 0, 0);
}

void ParticleSystem::Emit(const ParticleProps& particleProps) {
    Particle& particle = m_ParticlePool[m_PoolIndex];
    particle.active = true;
    particle.position = particleProps.position;

    // Velocity
    particle.velocity = particleProps.velocity 
        + particleProps.velocityVariation * glm::vec3(glm::linearRand(-1.0f, 1.0f), glm::linearRand(-1.0f, 1.0f), glm::linearRand(-1.0f, 1.0f));

    // Color
    particle.color = particleProps.colorBegin;

    // Size
    particle.sizeBegin = particleProps.sizeBegin + particleProps.sizeVariation * glm::linearRand(-1.0f, 1.0f);
    particle.sizeEnd = particleProps.sizeEnd;
    particle.size = particle.sizeBegin;

    // Life
    particle.lifeTime = particleProps.lifeTime;
    particle.lifeRemaining = particleProps.lifeTime;

    m_PoolIndex = (m_PoolIndex == 0) ? static_cast<uint32_t>(m_ParticlePool.size() - 1) : m_PoolIndex - 1;
}

} // namespace Engine
