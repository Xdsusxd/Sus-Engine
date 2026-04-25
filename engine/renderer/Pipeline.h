#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace Engine {

class VulkanContext;

class Pipeline {
public:
    bool Init(const VulkanContext& ctx, VkRenderPass renderPass);
    void Shutdown(VkDevice device);

    void Bind(VkCommandBuffer cmd) const;
    void PushMVP(VkCommandBuffer cmd, const void* mvpData) const;

    VkPipelineLayout GetLayout() const { return m_PipelineLayout; }

private:
    bool CreatePipelineLayout(VkDevice device);
    bool CreateGraphicsPipeline(VkDevice device, VkRenderPass renderPass);
    
    VkShaderModule CreateShaderModule(VkDevice device, const std::string& filepath);

    VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
    VkPipeline       m_Pipeline       = VK_NULL_HANDLE;
};

} // namespace Engine
