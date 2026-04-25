#include "renderer/Renderer.h"
#include "renderer/VulkanContext.h"
#include "renderer/Swapchain.h"
#include "renderer/VkCheck.h"
#include "core/Logger.h"

#include <array>

namespace Engine {

// ═══════════════════════════════════════════════════════════════
// Init / Shutdown
// ═══════════════════════════════════════════════════════════════

bool Renderer::Init(const VulkanContext& ctx, Swapchain& /*swapchain*/) {
    LOG_INFO("Renderer", "Initializing renderer...");

    if (!CreateVmaAllocator(ctx))           return false;
    if (!CreateCommandPool(ctx))            return false;
    if (!CreateCommandBuffers(ctx.GetDevice())) return false;
    if (!CreateSyncObjects(ctx.GetDevice()))    return false;

    LOG_INFO("Renderer", "Renderer initialized ({} frames in flight)", MAX_FRAMES_IN_FLIGHT);
    return true;
}

void Renderer::Shutdown(VkDevice device) {
    LOG_INFO("Renderer", "Shutting down renderer...");

    // Sync objects
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, m_ImageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device, m_RenderFinishedSemaphores[i], nullptr);
        vkDestroyFence(device, m_InFlightFences[i], nullptr);
    }
    m_ImageAvailableSemaphores.clear();
    m_RenderFinishedSemaphores.clear();
    m_InFlightFences.clear();

    // Command pool (implicitly frees all command buffers)
    if (m_CommandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device, m_CommandPool, nullptr);
        m_CommandPool = VK_NULL_HANDLE;
    }
    m_CommandBuffers.clear();

    // VMA
    if (m_Allocator != VK_NULL_HANDLE) {
        vmaDestroyAllocator(m_Allocator);
        m_Allocator = VK_NULL_HANDLE;
    }

    LOG_INFO("Renderer", "Renderer shut down");
}

// ═══════════════════════════════════════════════════════════════
// Frame lifecycle
// ═══════════════════════════════════════════════════════════════

bool Renderer::BeginFrame(const VulkanContext& ctx, Swapchain& swapchain, GLFWwindow* window) {
    VkDevice device = ctx.GetDevice();

    // Wait for previous frame with this index to finish
    vkWaitForFences(device, 1, &m_InFlightFences[m_CurrentFrame],
                    VK_TRUE, UINT64_MAX);

    // Acquire next swapchain image
    VkResult result = vkAcquireNextImageKHR(
        device, swapchain.GetHandle(), UINT64_MAX,
        m_ImageAvailableSemaphores[m_CurrentFrame],
        VK_NULL_HANDLE, &m_ImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        swapchain.Recreate(ctx, window);
        return false; // Skip this frame
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        LOG_ERROR("Renderer", "Failed to acquire swapchain image");
        return false;
    }

    // Only reset fence if we're actually submitting work
    vkResetFences(device, 1, &m_InFlightFences[m_CurrentFrame]);

    // Reset and begin command buffer
    VkCommandBuffer cmd = m_CommandBuffers[m_CurrentFrame];
    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;

    VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));

    // Begin render pass
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.01f, 0.01f, 0.02f, 1.0f}}; // Near-black
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo rpInfo{};
    rpInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpInfo.renderPass        = swapchain.GetRenderPass();
    rpInfo.framebuffer       = swapchain.GetFramebuffer(m_ImageIndex);
    rpInfo.renderArea.offset = {0, 0};
    rpInfo.renderArea.extent = swapchain.GetExtent();
    rpInfo.clearValueCount   = static_cast<uint32_t>(clearValues.size());
    rpInfo.pClearValues      = clearValues.data();

    vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Set dynamic viewport and scissor
    VkViewport viewport{};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = static_cast<float>(swapchain.GetExtent().width);
    viewport.height   = static_cast<float>(swapchain.GetExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchain.GetExtent();
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    return true;
}

void Renderer::EndFrame(const VulkanContext& ctx, Swapchain& swapchain, GLFWwindow* window) {
    VkCommandBuffer cmd = m_CommandBuffers[m_CurrentFrame];

    vkCmdEndRenderPass(cmd);
    VK_CHECK_VOID(vkEndCommandBuffer(cmd));

    // Submit
    VkSemaphore waitSemaphores[]   = { m_ImageAvailableSemaphores[m_CurrentFrame] };
    VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submitInfo{};
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = waitSemaphores;
    submitInfo.pWaitDstStageMask    = waitStages;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &cmd;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = signalSemaphores;

    VK_CHECK_VOID(vkQueueSubmit(ctx.GetGraphicsQueue(), 1, &submitInfo,
                                 m_InFlightFences[m_CurrentFrame]));

    // Present
    VkSwapchainKHR swapchains[] = { swapchain.GetHandle() };

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = signalSemaphores;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = swapchains;
    presentInfo.pImageIndices      = &m_ImageIndex;

    VkResult result = vkQueuePresentKHR(ctx.GetPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        swapchain.Recreate(ctx, window);
    } else if (result != VK_SUCCESS) {
        LOG_ERROR("Renderer", "Failed to present swapchain image");
    }

    // Advance to next frame
    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

// ═══════════════════════════════════════════════════════════════
// VMA Allocator
// ═══════════════════════════════════════════════════════════════

bool Renderer::CreateVmaAllocator(const VulkanContext& ctx) {
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.instance       = ctx.GetInstance();
    allocatorInfo.physicalDevice = ctx.GetPhysicalDevice();
    allocatorInfo.device         = ctx.GetDevice();
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;

    VK_CHECK(vmaCreateAllocator(&allocatorInfo, &m_Allocator));
    LOG_INFO("Renderer", "VMA allocator created");
    return true;
}

// ═══════════════════════════════════════════════════════════════
// Command Pool & Buffers
// ═══════════════════════════════════════════════════════════════

bool Renderer::CreateCommandPool(const VulkanContext& ctx) {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = ctx.GetQueueFamilies().graphicsFamily;

    VK_CHECK(vkCreateCommandPool(ctx.GetDevice(), &poolInfo, nullptr, &m_CommandPool));
    LOG_INFO("Renderer", "Command pool created (queue family: {})",
             ctx.GetQueueFamilies().graphicsFamily);
    return true;
}

bool Renderer::CreateCommandBuffers(VkDevice device) {
    m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool        = m_CommandPool;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, m_CommandBuffers.data()));
    LOG_INFO("Renderer", "{} command buffers allocated", MAX_FRAMES_IN_FLIGHT);
    return true;
}

// ═══════════════════════════════════════════════════════════════
// Sync Objects
// ═══════════════════════════════════════════════════════════════

bool Renderer::CreateSyncObjects(VkDevice device) {
    m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start signaled so first frame doesn't block

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]));
        VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]));
        VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &m_InFlightFences[i]));
    }

    LOG_INFO("Renderer", "Sync objects created ({} sets)", MAX_FRAMES_IN_FLIGHT);
    return true;
}

} // namespace Engine
