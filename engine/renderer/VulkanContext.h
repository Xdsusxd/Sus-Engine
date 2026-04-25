#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

struct GLFWwindow;

namespace Engine {

struct QueueFamilyIndices {
    uint32_t graphicsFamily = UINT32_MAX;
    uint32_t presentFamily  = UINT32_MAX;

    bool IsComplete() const {
        return graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX;
    }
};

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR        capabilities{};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};

class VulkanContext {
public:
    bool Init(GLFWwindow* window);
    void Shutdown();

    // ── Accessors ─────────────────────────────────────────────
    VkInstance       GetInstance()       const { return m_Instance; }
    VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
    VkDevice         GetDevice()         const { return m_Device; }
    VkSurfaceKHR     GetSurface()        const { return m_Surface; }
    VkQueue          GetGraphicsQueue()  const { return m_GraphicsQueue; }
    VkQueue          GetPresentQueue()   const { return m_PresentQueue; }
    
    const QueueFamilyIndices& GetQueueFamilies() const { return m_QueueFamilies; }

    SwapchainSupportDetails QuerySwapchainSupport() const;
    SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device) const;

private:
    bool CreateInstance();
    bool SetupDebugMessenger();
    bool CreateSurface(GLFWwindow* window);
    bool PickPhysicalDevice();
    bool CreateLogicalDevice();

    // ── Helpers ───────────────────────────────────────────────
    bool CheckValidationLayerSupport() const;
    std::vector<const char*> GetRequiredExtensions() const;
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;
    bool IsDeviceSuitable(VkPhysicalDevice device) const;
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device) const;
    int  RateDeviceSuitability(VkPhysicalDevice device) const;

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void* userData);

    // ── Members ───────────────────────────────────────────────
    VkInstance               m_Instance       = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR             m_Surface        = VK_NULL_HANDLE;
    VkPhysicalDevice         m_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice                 m_Device         = VK_NULL_HANDLE;
    VkQueue                  m_GraphicsQueue  = VK_NULL_HANDLE;
    VkQueue                  m_PresentQueue   = VK_NULL_HANDLE;
    QueueFamilyIndices       m_QueueFamilies;

#ifdef NDEBUG
    static constexpr bool s_EnableValidation = false;
#else
    static constexpr bool s_EnableValidation = true;
#endif

    const std::vector<const char*> m_ValidationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> m_DeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
};

} // namespace Engine
