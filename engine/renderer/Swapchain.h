#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

struct GLFWwindow;

namespace Engine {

// Forward declaration
class VulkanContext;

class Swapchain {
public:
    bool Init(const VulkanContext& ctx, GLFWwindow* window);
    void Shutdown(VkDevice device);

    // Recreate swapchain (e.g. on window resize)
    bool Recreate(const VulkanContext& ctx, GLFWwindow* window);

    // ── Accessors ─────────────────────────────────────────────
    VkSwapchainKHR   GetHandle()      const { return m_Swapchain; }
    VkRenderPass     GetRenderPass()  const { return m_RenderPass; }
    VkFormat         GetImageFormat() const { return m_ImageFormat; }
    VkExtent2D       GetExtent()      const { return m_Extent; }
    uint32_t         GetImageCount()  const { return static_cast<uint32_t>(m_Images.size()); }
    VkFramebuffer    GetFramebuffer(uint32_t index) const { return m_Framebuffers[index]; }
    VkImageView      GetImageView(uint32_t index)   const { return m_ImageViews[index]; }
    VkImageView      GetDepthImageView() const { return m_DepthImageView; }

private:
    bool CreateSwapchain(const VulkanContext& ctx, GLFWwindow* window);
    bool CreateImageViews(VkDevice device);
    bool CreateDepthResources(const VulkanContext& ctx);
    bool CreateRenderPass(const VulkanContext& ctx);
    bool CreateFramebuffers(VkDevice device);

    void CleanupSwapchain(VkDevice device);

    // ── Helpers ───────────────────────────────────────────────
    VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
    VkPresentModeKHR   ChoosePresentMode(const std::vector<VkPresentModeKHR>& modes);
    VkExtent2D         ChooseExtent(const VkSurfaceCapabilitiesKHR& caps, GLFWwindow* window);
    VkFormat           FindDepthFormat(VkPhysicalDevice physicalDevice);
    VkFormat           FindSupportedFormat(VkPhysicalDevice physicalDevice,
                                           const std::vector<VkFormat>& candidates,
                                           VkImageTiling tiling,
                                           VkFormatFeatureFlags features);

    // ── Members ───────────────────────────────────────────────
    VkSwapchainKHR             m_Swapchain    = VK_NULL_HANDLE;
    VkRenderPass               m_RenderPass   = VK_NULL_HANDLE;
    VkFormat                   m_ImageFormat  = VK_FORMAT_UNDEFINED;
    VkFormat                   m_DepthFormat  = VK_FORMAT_UNDEFINED;
    VkExtent2D                 m_Extent       = {0, 0};

    // Color images (owned by swapchain, NOT destroyed by us)
    std::vector<VkImage>       m_Images;
    std::vector<VkImageView>   m_ImageViews;
    std::vector<VkFramebuffer> m_Framebuffers;

    // Depth buffer (owned by us)
    VkImage        m_DepthImage     = VK_NULL_HANDLE;
    VkDeviceMemory m_DepthMemory    = VK_NULL_HANDLE;
    VkImageView    m_DepthImageView = VK_NULL_HANDLE;
};

} // namespace Engine
