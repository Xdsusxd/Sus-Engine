#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <vector>
#include <cstdint>

struct GLFWwindow;

namespace Engine {

class VulkanContext;
class Swapchain;

// Maximum frames that can be processed concurrently
constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

class Renderer {
public:
    bool Init(const VulkanContext& ctx, Swapchain& swapchain);
    void Shutdown(VkDevice device);

    // ── Frame lifecycle ───────────────────────────────────────
    // Returns false if swapchain needs recreation
    bool BeginFrame(const VulkanContext& ctx, Swapchain& swapchain, GLFWwindow* window);
    void EndFrame(const VulkanContext& ctx, Swapchain& swapchain, GLFWwindow* window);

    // ── Accessors ─────────────────────────────────────────────
    VmaAllocator       GetAllocator()       const { return m_Allocator; }
    VkCommandPool      GetCommandPool()     const { return m_CommandPool; }
    VkCommandBuffer    GetCurrentCmdBuffer() const { return m_CommandBuffers[m_CurrentFrame]; }
    uint32_t           GetCurrentFrame()     const { return m_CurrentFrame; }
    uint32_t           GetImageIndex()       const { return m_ImageIndex; }

private:
    bool CreateVmaAllocator(const VulkanContext& ctx);
    bool CreateCommandPool(const VulkanContext& ctx);
    bool CreateCommandBuffers(VkDevice device);
    bool CreateSyncObjects(VkDevice device);

    // ── VMA ───────────────────────────────────────────────────
    VmaAllocator m_Allocator = VK_NULL_HANDLE;

    // ── Command buffers ───────────────────────────────────────
    VkCommandPool                m_CommandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_CommandBuffers;

    // ── Sync objects (per frame in flight) ────────────────────
    std::vector<VkSemaphore> m_ImageAvailableSemaphores;
    std::vector<VkSemaphore> m_RenderFinishedSemaphores;
    std::vector<VkFence>     m_InFlightFences;

    uint32_t m_CurrentFrame = 0;
    uint32_t m_ImageIndex   = 0;
};

} // namespace Engine
