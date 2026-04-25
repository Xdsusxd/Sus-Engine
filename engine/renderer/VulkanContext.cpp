#include "renderer/VulkanContext.h"
#include "renderer/VkCheck.h"
#include "core/Logger.h"

#include <GLFW/glfw3.h>

#include <set>
#include <map>
#include <string>

namespace Engine {

// ═══════════════════════════════════════════════════════════════
// Public interface
// ═══════════════════════════════════════════════════════════════

bool VulkanContext::Init(GLFWwindow* window) {
    LOG_INFO("Vulkan", "Initializing Vulkan context...");

    if (!CreateInstance())      return false;
    if (!SetupDebugMessenger()) return false;
    if (!CreateSurface(window)) return false;
    if (!PickPhysicalDevice())  return false;
    if (!CreateLogicalDevice()) return false;

    LOG_INFO("Vulkan", "Vulkan context initialized successfully");
    return true;
}

void VulkanContext::Shutdown() {
    LOG_INFO("Vulkan", "Shutting down Vulkan context...");

    if (m_Device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_Device);
        vkDestroyDevice(m_Device, nullptr);
        m_Device = VK_NULL_HANDLE;
    }

    if (m_DebugMessenger != VK_NULL_HANDLE) {
        auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (func) {
            func(m_Instance, m_DebugMessenger, nullptr);
        }
        m_DebugMessenger = VK_NULL_HANDLE;
    }

    if (m_Surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        m_Surface = VK_NULL_HANDLE;
    }

    if (m_Instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_Instance, nullptr);
        m_Instance = VK_NULL_HANDLE;
    }

    LOG_INFO("Vulkan", "Vulkan context destroyed");
}

// ═══════════════════════════════════════════════════════════════
// Instance
// ═══════════════════════════════════════════════════════════════

bool VulkanContext::CreateInstance() {
    if (s_EnableValidation && !CheckValidationLayerSupport()) {
        LOG_ERROR("Vulkan", "Validation layers requested but not available!");
        return false;
    }

    VkApplicationInfo appInfo{};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "FY-Engine";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.pEngineName        = "FY-Engine";
    appInfo.engineVersion      = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_3;

    auto extensions = GetRequiredExtensions();

    VkInstanceCreateInfo createInfo{};
    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo        = &appInfo;
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    // Debug messenger for instance creation/destruction
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

    if (s_EnableValidation) {
        createInfo.enabledLayerCount   = static_cast<uint32_t>(m_ValidationLayers.size());
        createInfo.ppEnabledLayerNames = m_ValidationLayers.data();

        debugCreateInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = DebugCallback;

        createInfo.pNext = &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_Instance));
    LOG_INFO("Vulkan", "Vulkan instance created (API 1.3)");
    return true;
}

// ═══════════════════════════════════════════════════════════════
// Debug Messenger
// ═══════════════════════════════════════════════════════════════

bool VulkanContext::SetupDebugMessenger() {
    if (!s_EnableValidation) return true;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;

    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT"));
    if (!func) {
        LOG_WARN("Vulkan", "vkCreateDebugUtilsMessengerEXT not available");
        return true; // Non-fatal
    }

    VK_CHECK(func(m_Instance, &createInfo, nullptr, &m_DebugMessenger));
    LOG_INFO("Vulkan", "Debug messenger created");
    return true;
}

// ═══════════════════════════════════════════════════════════════
// Surface
// ═══════════════════════════════════════════════════════════════

bool VulkanContext::CreateSurface(GLFWwindow* window) {
    VK_CHECK(glfwCreateWindowSurface(m_Instance, window, nullptr, &m_Surface));
    LOG_INFO("Vulkan", "Window surface created");
    return true;
}

// ═══════════════════════════════════════════════════════════════
// Physical Device
// ═══════════════════════════════════════════════════════════════

bool VulkanContext::PickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        LOG_FATAL("Vulkan", "No GPUs with Vulkan support found!");
        return false;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

    // Rate and pick the best device
    std::multimap<int, VkPhysicalDevice> candidates;
    for (const auto& device : devices) {
        int score = RateDeviceSuitability(device);
        candidates.insert({score, device});
    }

    if (candidates.rbegin()->first > 0) {
        m_PhysicalDevice = candidates.rbegin()->second;
        m_QueueFamilies = FindQueueFamilies(m_PhysicalDevice);
    }

    if (m_PhysicalDevice == VK_NULL_HANDLE) {
        LOG_FATAL("Vulkan", "Failed to find a suitable GPU!");
        return false;
    }

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(m_PhysicalDevice, &props);
    LOG_INFO("Vulkan", "Selected GPU: {}", props.deviceName);
    LOG_INFO("Vulkan", "  Driver version: {}.{}.{}",
             VK_VERSION_MAJOR(props.driverVersion),
             VK_VERSION_MINOR(props.driverVersion),
             VK_VERSION_PATCH(props.driverVersion));
    LOG_INFO("Vulkan", "  API version: {}.{}.{}",
             VK_VERSION_MAJOR(props.apiVersion),
             VK_VERSION_MINOR(props.apiVersion),
             VK_VERSION_PATCH(props.apiVersion));

    return true;
}

// ═══════════════════════════════════════════════════════════════
// Logical Device
// ═══════════════════════════════════════════════════════════════

bool VulkanContext::CreateLogicalDevice() {
    std::set<uint32_t> uniqueQueueFamilies = {
        m_QueueFamilies.graphicsFamily,
        m_QueueFamilies.presentFamily
    };

    float queuePriority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    for (uint32_t family : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = family;
        queueInfo.queueCount       = 1;
        queueInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueInfo);
    }

    // Enable features we need
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.fillModeNonSolid  = VK_TRUE; // wireframe

    VkDeviceCreateInfo createInfo{};
    createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos       = queueCreateInfos.data();
    createInfo.pEnabledFeatures        = &deviceFeatures;
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(m_DeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

    // Validation layers on device (compatibility with older implementations)
    if (s_EnableValidation) {
        createInfo.enabledLayerCount   = static_cast<uint32_t>(m_ValidationLayers.size());
        createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    VK_CHECK(vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device));

    vkGetDeviceQueue(m_Device, m_QueueFamilies.graphicsFamily, 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, m_QueueFamilies.presentFamily,  0, &m_PresentQueue);

    LOG_INFO("Vulkan", "Logical device created (graphics queue: {}, present queue: {})",
             m_QueueFamilies.graphicsFamily, m_QueueFamilies.presentFamily);
    return true;
}

// ═══════════════════════════════════════════════════════════════
// Helpers
// ═══════════════════════════════════════════════════════════════

bool VulkanContext::CheckValidationLayerSupport() const {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> available(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, available.data());

    for (const char* layerName : m_ValidationLayers) {
        bool found = false;
        for (const auto& layer : available) {
            if (strcmp(layerName, layer.layerName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            LOG_WARN("Vulkan", "Validation layer not found: {}", layerName);
            return false;
        }
    }
    return true;
}

std::vector<const char*> VulkanContext::GetRequiredExtensions() const {
    uint32_t glfwExtCount = 0;
    const char** glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);

    std::vector<const char*> extensions(glfwExts, glfwExts + glfwExtCount);

    if (s_EnableValidation) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

QueueFamilyIndices VulkanContext::FindQueueFamilies(VkPhysicalDevice device) const {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> families(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, families.data());

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.IsComplete()) break;
    }

    return indices;
}

bool VulkanContext::IsDeviceSuitable(VkPhysicalDevice device) const {
    auto indices = FindQueueFamilies(device);
    if (!indices.IsComplete()) return false;

    if (!CheckDeviceExtensionSupport(device)) return false;

    auto swapchainSupport = QuerySwapchainSupport(device);
    if (swapchainSupport.formats.empty() || swapchainSupport.presentModes.empty()) {
        return false;
    }

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);
    return features.samplerAnisotropy == VK_TRUE;
}

bool VulkanContext::CheckDeviceExtensionSupport(VkPhysicalDevice device) const {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> available(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, available.data());

    std::set<std::string> required(m_DeviceExtensions.begin(), m_DeviceExtensions.end());
    for (const auto& ext : available) {
        required.erase(ext.extensionName);
    }

    return required.empty();
}

int VulkanContext::RateDeviceSuitability(VkPhysicalDevice device) const {
    if (!IsDeviceSuitable(device)) return 0;

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(device, &props);

    int score = 0;

    // Strongly prefer discrete GPUs
    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 10000;
    } else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
        score += 1000;
    }

    // More VRAM is better
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(device, &memProps);
    for (uint32_t i = 0; i < memProps.memoryHeapCount; i++) {
        if (memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
            score += static_cast<int>(memProps.memoryHeaps[i].size / (1024 * 1024)); // MB
        }
    }

    VkPhysicalDeviceProperties deviceProps;
    vkGetPhysicalDeviceProperties(device, &deviceProps);
    LOG_INFO("Vulkan", "  GPU candidate: {} (score: {})", deviceProps.deviceName, score);

    return score;
}

SwapchainSupportDetails VulkanContext::QuerySwapchainSupport() const {
    return QuerySwapchainSupport(m_PhysicalDevice);
}

SwapchainSupportDetails VulkanContext::QuerySwapchainSupport(VkPhysicalDevice device) const {
    SwapchainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);
    if (formatCount > 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);
    if (presentModeCount > 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

// ═══════════════════════════════════════════════════════════════
// Debug Callback
// ═══════════════════════════════════════════════════════════════

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT /*type*/,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* /*userData*/)
{
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        LOG_ERROR("VkValidation", "{}", callbackData->pMessage);
    } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        LOG_WARN("VkValidation", "{}", callbackData->pMessage);
    } else {
        LOG_TRACE("VkValidation", "{}", callbackData->pMessage);
    }

    return VK_FALSE;
}

} // namespace Engine
