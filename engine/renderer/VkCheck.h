#pragma once

#include "core/Logger.h"
#include <vulkan/vulkan.h>
#include <string>

namespace Engine {

// ── VK_CHECK macro — validates every Vulkan call ──────────────
inline const char* VkResultToString(VkResult result) {
    switch (result) {
        case VK_SUCCESS:                        return "VK_SUCCESS";
        case VK_NOT_READY:                      return "VK_NOT_READY";
        case VK_TIMEOUT:                        return "VK_TIMEOUT";
        case VK_EVENT_SET:                      return "VK_EVENT_SET";
        case VK_EVENT_RESET:                    return "VK_EVENT_RESET";
        case VK_INCOMPLETE:                     return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY:       return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:     return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED:    return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST:              return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED:        return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT:        return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT:    return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT:      return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER:      return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS:         return "VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED:     return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_SURFACE_LOST_KHR:         return "VK_ERROR_SURFACE_LOST_KHR";
        case VK_ERROR_OUT_OF_DATE_KHR:          return "VK_ERROR_OUT_OF_DATE_KHR";
        case VK_SUBOPTIMAL_KHR:                 return "VK_SUBOPTIMAL_KHR";
        default:                                return "UNKNOWN_VK_RESULT";
    }
}

#define VK_CHECK(expr)                                                          \
    do {                                                                        \
        VkResult _vk_result_ = (expr);                                          \
        if (_vk_result_ != VK_SUCCESS) {                                        \
            LOG_ERROR("Vulkan", "{} failed: {} (line {} in {})",                \
                      #expr, ::Engine::VkResultToString(_vk_result_),           \
                      __LINE__, __FILE__);                                      \
            return false;                                                       \
        }                                                                       \
    } while (0)

// Variant that doesn't return — for use in void functions
#define VK_CHECK_VOID(expr)                                                     \
    do {                                                                        \
        VkResult _vk_result_ = (expr);                                          \
        if (_vk_result_ != VK_SUCCESS) {                                        \
            LOG_ERROR("Vulkan", "{} failed: {} (line {} in {})",                \
                      #expr, ::Engine::VkResultToString(_vk_result_),           \
                      __LINE__, __FILE__);                                      \
        }                                                                       \
    } while (0)

} // namespace Engine
