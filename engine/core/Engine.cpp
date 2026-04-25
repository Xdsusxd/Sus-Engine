#include "core/Engine.h"
#include "core/Logger.h"
#include "renderer/ObjLoader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <format>

namespace Engine {

bool EngineApp::Init(const EngineConfig& config) {
    // ── Logger ────────────────────────────────────────────────
    if (!Logger::Get().Init(config.logFile)) {
        return false;
    }
    LOG_INFO("Engine", "=== FY-Engine v0.1.0 ===");

    // ── Window ────────────────────────────────────────────────
    if (!m_Window.Init(config.window)) {
        LOG_FATAL("Engine", "Window initialization failed!");
        return false;
    }

    // ── Input ─────────────────────────────────────────────────
    if (!m_Input.Init(m_Window.GetHandle())) {
        LOG_FATAL("Engine", "Input initialization failed!");
        return false;
    }

    // ── Time ──────────────────────────────────────────────────
    m_Time.Init();

    // ── Vulkan Context ────────────────────────────────────────
    if (!m_VulkanCtx.Init(m_Window.GetHandle())) {
        LOG_FATAL("Engine", "Vulkan initialization failed!");
        return false;
    }

    // ── Swapchain ─────────────────────────────────────────────
    if (!m_Swapchain.Init(m_VulkanCtx, m_Window.GetHandle())) {
        LOG_FATAL("Engine", "Swapchain initialization failed!");
        return false;
    }

    // ── Renderer ──────────────────────────────────────────────
    if (!m_Renderer.Init(m_VulkanCtx, m_Swapchain)) {
        LOG_FATAL("Engine", "Renderer initialization failed!");
        return false;
    }

    // ── Pipelines ─────────────────────────────────────────────
    if (!m_Pipeline.Init(m_VulkanCtx, m_Swapchain.GetRenderPass())) {
        LOG_FATAL("Engine", "Pipeline initialization failed!");
        return false;
    }
    if (!m_GridPipeline.Init(m_VulkanCtx, m_Swapchain.GetRenderPass())) {
        LOG_FATAL("Engine", "Grid pipeline initialization failed!");
        return false;
    }

    // ── Camera ────────────────────────────────────────────────
    float aspect = static_cast<float>(m_Swapchain.GetExtent().width) /
                   static_cast<float>(m_Swapchain.GetExtent().height);
    m_Camera.Init(60.0f, aspect, 0.01f, 1000.0f);

    // ── Assets ────────────────────────────────────────────────
    m_CubeMesh = Mesh::CreateCube(
        m_Renderer.GetAllocator(), m_VulkanCtx.GetDevice(),
        m_Renderer.GetCommandPool(), m_VulkanCtx.GetGraphicsQueue());

    m_PyramidMesh = ObjLoader::LoadFromFile("assets/models/pyramid.obj",
        m_Renderer.GetAllocator(), m_VulkanCtx.GetDevice(),
        m_Renderer.GetCommandPool(), m_VulkanCtx.GetGraphicsQueue());

    m_WhiteTexture.CreateSolidColor(m_VulkanCtx, m_Renderer, 255, 255, 255);

    // ── Scene Setup ───────────────────────────────────────────
    auto& centralCube = m_Scene.AddObject("CentralCube");
    centralCube.mesh = &m_CubeMesh;
    centralCube.transform.position = {0.0f, 0.5f, 0.0f};
    centralCube.autoRotate = {25.0f, 45.0f, 0.0f};

    auto& pyramid = m_Scene.AddObject("Pyramid");
    pyramid.mesh = &m_PyramidMesh;
    pyramid.transform.position = {0.0f, 2.0f, 0.0f};
    pyramid.transform.scale = {0.5f, 0.5f, 0.5f};
    pyramid.autoRotate = {0.0f, -90.0f, 0.0f};

    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(90.0f * static_cast<float>(i));
        float x = cosf(angle) * 3.0f;
        float z = sinf(angle) * 3.0f;

        auto& obj = m_Scene.AddObject(std::format("OrbitCube_{}", i));
        obj.mesh = &m_CubeMesh;
        obj.transform.position = {x, 0.35f, z};
        obj.transform.scale = {0.7f, 0.7f, 0.7f};
        obj.transform.rotation = {0.0f, 90.0f * i, 0.0f};
        obj.autoRotate = {0.0f, 22.5f, 0.0f};
    }

    m_Running = true;
    LOG_INFO("Engine", "All systems initialized successfully");
    return true;
}

void EngineApp::Run() {
    LOG_INFO("Engine", "Entering main loop...");

    while (m_Running && m_Window.IsOpen()) {
        // ── Frame start ───────────────────────────────────────
        m_Time.Update();
        m_Window.PollEvents();

        float dt = m_Time.DeltaTime();

        // ── Check for ESC to close ────────────────────────────
        if (m_Input.IsKeyPressed(Key::Escape)) {
            LOG_INFO("Engine", "Escape pressed — closing");
            m_Running = false;
        }

        // ── Update title with FPS ─────────────────────────────
        static float fpsTimer = 0.0f;
        fpsTimer += dt;
        if (fpsTimer >= 0.5f) {
            auto title = std::format("FY-Engine | FPS: {:.0f} | dt: {:.2f}ms", 
                                      m_Time.FPS(), dt * 1000.0f);
            m_Window.SetTitle(title);
            fpsTimer = 0.0f;
        }

        // ── Handle resize ─────────────────────────────────────
        if (m_Window.WasResized()) {
            m_Window.ResetResizedFlag();
            float newAspect = static_cast<float>(m_Swapchain.GetExtent().width) /
                              static_cast<float>(m_Swapchain.GetExtent().height);
            m_Camera.SetAspect(newAspect);
        }

        // ── Update scene & camera ─────────────────────────────
        m_Camera.Update(m_Input, dt);
        m_Scene.Update(dt);

        // ── Render frame ──────────────────────────────────────
        if (m_Renderer.BeginFrame(m_VulkanCtx, m_Swapchain, m_Window.GetHandle())) {
            VkCommandBuffer cmd = m_Renderer.GetCurrentCmdBuffer();
            glm::mat4 viewProj = m_Camera.GetViewProjection();

            // ── 1. Draw grid ──────────────────────────────────
            m_GridPipeline.Draw(cmd, &viewProj);

            // ── 2. Draw scene objects ─────────────────────────
            m_Pipeline.Bind(cmd);
            m_Scene.Render(cmd, m_Pipeline, viewProj);

            m_Renderer.EndFrame(m_VulkanCtx, m_Swapchain, m_Window.GetHandle());
        }

        // ── Frame end ─────────────────────────────────────────
        m_Input.Update();
    }

    LOG_INFO("Engine", "Main loop ended — total frames: {}, runtime: {:.1f}s",
             m_Time.FrameCount(), m_Time.TotalTime());
}

void EngineApp::Shutdown() {
    LOG_INFO("Engine", "Shutting down...");
    vkDeviceWaitIdle(m_VulkanCtx.GetDevice());

    m_WhiteTexture.Destroy(m_VulkanCtx.GetDevice());
    m_CubeMesh.Destroy();
    m_PyramidMesh.Destroy();
    m_GridPipeline.Shutdown(m_VulkanCtx.GetDevice());
    m_Pipeline.Shutdown(m_VulkanCtx.GetDevice());
    m_Renderer.Shutdown(m_VulkanCtx.GetDevice());
    m_Swapchain.Shutdown(m_VulkanCtx.GetDevice());
    m_VulkanCtx.Shutdown();
    m_Input.Shutdown();
    m_Window.Shutdown();
    LOG_INFO("Engine", "=== FY-Engine shutdown complete ===");
    Logger::Get().Shutdown();
}

} // namespace Engine
