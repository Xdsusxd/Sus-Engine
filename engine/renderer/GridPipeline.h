#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace Engine {

class VulkanContext;

class GridPipeline {
public:
    bool Init(const VulkanContext& ctx, VkRenderPass renderPass);
    void Shutdown(VkDevice device);

    void Draw(VkCommandBuffer cmd, const void* viewProjData) const;

private:
    VkShaderModule CreateShaderModule(VkDevice device, const std::string& filepath);

    VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
    VkPipeline       m_Pipeline       = VK_NULL_HANDLE;
};

} // namespace Engine
