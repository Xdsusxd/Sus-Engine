#include "ui/UISystem.h"
#include "renderer/VulkanContext.h"
#include "renderer/Swapchain.h"
#include "core/Logger.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

namespace Engine {

bool UISystem::Init(VulkanContext& ctx, Swapchain& swapchain, GLFWwindow* window) {
    if (m_Initialized) return true;

    // 1. Create Descriptor Pool for ImGui
    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    
    if (vkCreateDescriptorPool(ctx.GetDevice(), &pool_info, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
        LOG_ERROR("UI", "Failed to create ImGui descriptor pool");
        return false;
    }

    // 2. Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // 3. Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = ctx.GetInstance();
    init_info.PhysicalDevice = ctx.GetPhysicalDevice();
    init_info.Device = ctx.GetDevice();
    init_info.QueueFamily = ctx.GetQueueFamilies().graphicsFamily;
    init_info.Queue = ctx.GetGraphicsQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = m_DescriptorPool;
    init_info.PipelineInfoMain.RenderPass = swapchain.GetRenderPass();
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.MinImageCount = swapchain.GetImageCount();
    init_info.ImageCount = swapchain.GetImageCount();
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = nullptr;
    
    if (!ImGui_ImplVulkan_Init(&init_info)) {
        LOG_ERROR("UI", "Failed to initialize ImGui Vulkan backend");
        return false;
    }

    m_Initialized = true;
    LOG_INFO("UI", "ImGui initialized successfully");
    return true;
}

void UISystem::Shutdown(VkDevice device) {
    if (!m_Initialized) return;

    vkDeviceWaitIdle(device);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (m_DescriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
        m_DescriptorPool = VK_NULL_HANDLE;
    }

    m_Initialized = false;
    LOG_INFO("UI", "ImGui shut down");
}

void UISystem::BeginFrame() {
    if (!m_Initialized) return;

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UISystem::EndFrame(VkCommandBuffer cmd) {
    if (!m_Initialized) return;

    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    if (draw_data) {
        ImGui_ImplVulkan_RenderDrawData(draw_data, cmd);
    }
}

} // namespace Engine
