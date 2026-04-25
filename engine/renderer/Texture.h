#pragma once

#include "renderer/Buffer.h"
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <string>

namespace Engine {

class VulkanContext;
class Renderer;

class Texture {
public:
    bool LoadFromFile(const VulkanContext& ctx, const Renderer& renderer, const std::string& filepath);
    bool CreateSolidColor(const VulkanContext& ctx, const Renderer& renderer,
                          uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    void Destroy(VkDevice device);

    VkImageView GetImageView() const { return m_ImageView; }
    VkSampler   GetSampler()   const { return m_Sampler; }

private:
    bool CreateImage(const VulkanContext& ctx, VmaAllocator allocator,
                     uint32_t width, uint32_t height, const void* pixels,
                     VkCommandPool cmdPool, VkQueue queue);
    bool CreateImageView(VkDevice device);
    bool CreateSampler(VkDevice device);

    void TransitionImageLayout(VkDevice device, VkCommandPool cmdPool, VkQueue queue,
                               VkImageLayout oldLayout, VkImageLayout newLayout);

    VkImage       m_Image      = VK_NULL_HANDLE;
    VmaAllocation m_Allocation = VK_NULL_HANDLE;
    VmaAllocator  m_Allocator  = VK_NULL_HANDLE;
    VkImageView   m_ImageView  = VK_NULL_HANDLE;
    VkSampler     m_Sampler    = VK_NULL_HANDLE;
    uint32_t      m_Width  = 0;
    uint32_t      m_Height = 0;
};

} // namespace Engine
