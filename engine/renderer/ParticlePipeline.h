#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <glm/glm.hpp>

namespace Engine {

class VulkanContext;

// Particle data matching shader instance inputs
struct ParticleInstance {
    glm::vec3 position;
    float     size;
    glm::vec4 color;
};

// Push constant data
struct ParticlePushConstants {
    glm::mat4 viewProj;
    glm::vec3 cameraRight;
    float padding1;
    glm::vec3 cameraUp;
    float padding2;
};

class ParticlePipeline {
public:
    bool Init(const VulkanContext& ctx, VkRenderPass renderPass);
    void Shutdown(VkDevice device);

    void Bind(VkCommandBuffer cmd) const;
    void PushConstants(VkCommandBuffer cmd, const ParticlePushConstants& pc) const;

private:
    VkShaderModule CreateShaderModule(VkDevice device, const std::string& filepath);

    VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
    VkPipeline       m_Pipeline       = VK_NULL_HANDLE;
};

} // namespace Engine
