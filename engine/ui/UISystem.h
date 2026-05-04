#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

namespace Engine {

class VulkanContext;
class Swapchain;

class UISystem {
public:
    static UISystem& Get() {
        static UISystem instance;
        return instance;
    }

    bool Init(VulkanContext& ctx, Swapchain& swapchain, GLFWwindow* window);
    void Shutdown(VkDevice device);

    void BeginFrame();
    void EndFrame(VkCommandBuffer cmd);

private:
    UISystem() = default;
    ~UISystem() = default;

    UISystem(const UISystem&) = delete;
    UISystem& operator=(const UISystem&) = delete;

    VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
    bool m_Initialized = false;
};

} // namespace Engine
