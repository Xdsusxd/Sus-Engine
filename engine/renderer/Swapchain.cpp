#include "renderer/Swapchain.h"
#include "renderer/VulkanContext.h"
#include "renderer/VkCheck.h"
#include "core/Logger.h"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <array>
#include <limits>

namespace Engine {

// ═══════════════════════════════════════════════════════════════
// Public interface
// ═══════════════════════════════════════════════════════════════

bool Swapchain::Init(const VulkanContext& ctx, GLFWwindow* window) {
    if (!CreateSwapchain(ctx, window))      return false;
    if (!CreateImageViews(ctx.GetDevice())) return false;
    if (!CreateDepthResources(ctx))         return false;
    if (!CreateRenderPass(ctx))             return false;
    if (!CreateFramebuffers(ctx.GetDevice())) return false;

    LOG_INFO("Swapchain", "Swapchain initialized: {}x{}, {} images, format {}",
             m_Extent.width, m_Extent.height, m_Images.size(),
             static_cast<int>(m_ImageFormat));
    return true;
}

void Swapchain::Shutdown(VkDevice device) {
    CleanupSwapchain(device);

    if (m_RenderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(device, m_RenderPass, nullptr);
        m_RenderPass = VK_NULL_HANDLE;
    }

    LOG_INFO("Swapchain", "Swapchain shut down");
}

bool Swapchain::Recreate(const VulkanContext& ctx, GLFWwindow* window) {
    // Wait for minimization
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(ctx.GetDevice());
    CleanupSwapchain(ctx.GetDevice());

    if (!CreateSwapchain(ctx, window))          return false;
    if (!CreateImageViews(ctx.GetDevice()))     return false;
    if (!CreateDepthResources(ctx))             return false;
    if (!CreateFramebuffers(ctx.GetDevice()))   return false;

    LOG_INFO("Swapchain", "Swapchain recreated: {}x{}", m_Extent.width, m_Extent.height);
    return true;
}

// ═══════════════════════════════════════════════════════════════
// Swapchain Creation
// ═══════════════════════════════════════════════════════════════

bool Swapchain::CreateSwapchain(const VulkanContext& ctx, GLFWwindow* window) {
    auto support = ctx.QuerySwapchainSupport();

    auto surfaceFormat = ChooseSurfaceFormat(support.formats);
    auto presentMode   = ChoosePresentMode(support.presentModes);
    auto extent        = ChooseExtent(support.capabilities, window);

    // Request one more image than minimum for triple buffering
    uint32_t imageCount = support.capabilities.minImageCount + 1;
    if (support.capabilities.maxImageCount > 0 &&
        imageCount > support.capabilities.maxImageCount) {
        imageCount = support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = ctx.GetSurface();
    createInfo.minImageCount    = imageCount;
    createInfo.imageFormat      = surfaceFormat.format;
    createInfo.imageColorSpace  = surfaceFormat.colorSpace;
    createInfo.imageExtent      = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    auto indices = ctx.GetQueueFamilies();
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices   = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = nullptr;
    }

    createInfo.preTransform   = support.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode    = presentMode;
    createInfo.clipped        = VK_TRUE;
    createInfo.oldSwapchain   = VK_NULL_HANDLE;

    VK_CHECK(vkCreateSwapchainKHR(ctx.GetDevice(), &createInfo, nullptr, &m_Swapchain));

    // Retrieve swapchain images
    vkGetSwapchainImagesKHR(ctx.GetDevice(), m_Swapchain, &imageCount, nullptr);
    m_Images.resize(imageCount);
    vkGetSwapchainImagesKHR(ctx.GetDevice(), m_Swapchain, &imageCount, m_Images.data());

    m_ImageFormat = surfaceFormat.format;
    m_Extent = extent;

    return true;
}

// ═══════════════════════════════════════════════════════════════
// Image Views
// ═══════════════════════════════════════════════════════════════

bool Swapchain::CreateImageViews(VkDevice device) {
    m_ImageViews.resize(m_Images.size());

    for (size_t i = 0; i < m_Images.size(); i++) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image    = m_Images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format   = m_ImageFormat;

        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = 1;

        VK_CHECK(vkCreateImageView(device, &viewInfo, nullptr, &m_ImageViews[i]));
    }

    return true;
}

// ═══════════════════════════════════════════════════════════════
// Depth Buffer
// ═══════════════════════════════════════════════════════════════

bool Swapchain::CreateDepthResources(const VulkanContext& ctx) {
    m_DepthFormat = FindDepthFormat(ctx.GetPhysicalDevice());

    // Create depth image
    VkImageCreateInfo imageInfo{};
    imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType     = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width  = m_Extent.width;
    imageInfo.extent.height = m_Extent.height;
    imageInfo.extent.depth  = 1;
    imageInfo.mipLevels     = 1;
    imageInfo.arrayLayers   = 1;
    imageInfo.format        = m_DepthFormat;
    imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateImage(ctx.GetDevice(), &imageInfo, nullptr, &m_DepthImage));

    // Allocate memory
    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(ctx.GetDevice(), m_DepthImage, &memReqs);

    // Find suitable memory type
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(ctx.GetPhysicalDevice(), &memProps);

    uint32_t memTypeIndex = UINT32_MAX;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if ((memReqs.memoryTypeBits & (1 << i)) &&
            (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
            memTypeIndex = i;
            break;
        }
    }

    if (memTypeIndex == UINT32_MAX) {
        LOG_ERROR("Swapchain", "Failed to find suitable memory type for depth buffer");
        return false;
    }

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize  = memReqs.size;
    allocInfo.memoryTypeIndex = memTypeIndex;

    VK_CHECK(vkAllocateMemory(ctx.GetDevice(), &allocInfo, nullptr, &m_DepthMemory));
    VK_CHECK(vkBindImageMemory(ctx.GetDevice(), m_DepthImage, m_DepthMemory, 0));

    // Create depth image view
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image    = m_DepthImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format   = m_DepthFormat;
    viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.levelCount     = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount     = 1;

    VK_CHECK(vkCreateImageView(ctx.GetDevice(), &viewInfo, nullptr, &m_DepthImageView));

    LOG_INFO("Swapchain", "Depth buffer created (format: {})", static_cast<int>(m_DepthFormat));
    return true;
}

// ═══════════════════════════════════════════════════════════════
// Render Pass
// ═══════════════════════════════════════════════════════════════

bool Swapchain::CreateRenderPass(const VulkanContext& ctx) {
    // Color attachment
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format         = m_ImageFormat;
    colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Depth attachment
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format         = m_DepthFormat;
    depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorRef{};
    colorRef.attachment = 0;
    colorRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthRef{};
    depthRef.attachment = 1;
    depthRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &colorRef;
    subpass.pDepthStencilAttachment = &depthRef;

    // Subpass dependency — wait for color output before writing
    VkSubpassDependency dependency{};
    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass    = 0;
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                               VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                               VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments    = attachments.data();
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies   = &dependency;

    VK_CHECK(vkCreateRenderPass(ctx.GetDevice(), &renderPassInfo, nullptr, &m_RenderPass));
    LOG_INFO("Swapchain", "Render pass created (color + depth)");
    return true;
}

// ═══════════════════════════════════════════════════════════════
// Framebuffers
// ═══════════════════════════════════════════════════════════════

bool Swapchain::CreateFramebuffers(VkDevice device) {
    m_Framebuffers.resize(m_ImageViews.size());

    for (size_t i = 0; i < m_ImageViews.size(); i++) {
        std::array<VkImageView, 2> attachments = {
            m_ImageViews[i],
            m_DepthImageView
        };

        VkFramebufferCreateInfo fbInfo{};
        fbInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.renderPass      = m_RenderPass;
        fbInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        fbInfo.pAttachments    = attachments.data();
        fbInfo.width           = m_Extent.width;
        fbInfo.height          = m_Extent.height;
        fbInfo.layers          = 1;

        VK_CHECK(vkCreateFramebuffer(device, &fbInfo, nullptr, &m_Framebuffers[i]));
    }

    LOG_INFO("Swapchain", "{} framebuffers created", m_Framebuffers.size());
    return true;
}

// ═══════════════════════════════════════════════════════════════
// Cleanup (for recreation)
// ═══════════════════════════════════════════════════════════════

void Swapchain::CleanupSwapchain(VkDevice device) {
    // Depth resources
    if (m_DepthImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(device, m_DepthImageView, nullptr);
        m_DepthImageView = VK_NULL_HANDLE;
    }
    if (m_DepthImage != VK_NULL_HANDLE) {
        vkDestroyImage(device, m_DepthImage, nullptr);
        m_DepthImage = VK_NULL_HANDLE;
    }
    if (m_DepthMemory != VK_NULL_HANDLE) {
        vkFreeMemory(device, m_DepthMemory, nullptr);
        m_DepthMemory = VK_NULL_HANDLE;
    }

    // Framebuffers
    for (auto fb : m_Framebuffers) {
        vkDestroyFramebuffer(device, fb, nullptr);
    }
    m_Framebuffers.clear();

    // Image views (NOT images — those are owned by the swapchain)
    for (auto view : m_ImageViews) {
        vkDestroyImageView(device, view, nullptr);
    }
    m_ImageViews.clear();
    m_Images.clear();

    // Swapchain itself
    if (m_Swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device, m_Swapchain, nullptr);
        m_Swapchain = VK_NULL_HANDLE;
    }
}

// ═══════════════════════════════════════════════════════════════
// Helpers
// ═══════════════════════════════════════════════════════════════

VkSurfaceFormatKHR Swapchain::ChooseSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& formats)
{
    // Prefer SRGB with B8G8R8A8 format
    for (const auto& fmt : formats) {
        if (fmt.format == VK_FORMAT_B8G8R8A8_SRGB &&
            fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return fmt;
        }
    }
    // Fall back to first available
    return formats[0];
}

VkPresentModeKHR Swapchain::ChoosePresentMode(
    const std::vector<VkPresentModeKHR>& modes)
{
    // Prefer mailbox (triple buffering) for low-latency rendering
    for (const auto& mode : modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            LOG_INFO("Swapchain", "Present mode: Mailbox (triple buffering)");
            return mode;
        }
    }
    // FIFO is always available (vsync)
    LOG_INFO("Swapchain", "Present mode: FIFO (vsync)");
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::ChooseExtent(
    const VkSurfaceCapabilitiesKHR& caps, GLFWwindow* window)
{
    if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return caps.currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D extent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    extent.width  = std::clamp(extent.width,  caps.minImageExtent.width,  caps.maxImageExtent.width);
    extent.height = std::clamp(extent.height, caps.minImageExtent.height, caps.maxImageExtent.height);

    return extent;
}

VkFormat Swapchain::FindDepthFormat(VkPhysicalDevice physicalDevice) {
    return FindSupportedFormat(
        physicalDevice,
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

VkFormat Swapchain::FindSupportedFormat(
    VkPhysicalDevice physicalDevice,
    const std::vector<VkFormat>& candidates,
    VkImageTiling tiling,
    VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR &&
            (props.linearTilingFeatures & features) == features) {
            return format;
        }
        if (tiling == VK_IMAGE_TILING_OPTIMAL &&
            (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    LOG_ERROR("Swapchain", "Failed to find supported format!");
    return candidates[0]; // Fallback
}

} // namespace Engine
