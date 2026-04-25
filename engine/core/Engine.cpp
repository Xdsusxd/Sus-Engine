#include "core/Engine.h"
#include "core/Logger.h"

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

    // ── Input (needs window handle for callbacks) ─────────────
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

        // ── Update title with FPS every ~0.5s ─────────────────
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
            m_Swapchain.Recreate(m_VulkanCtx, m_Window.GetHandle());
        }

        // TODO: Update game systems here
        // TODO: Render here

        // ── Frame end ─────────────────────────────────────────
        m_Input.Update();
        m_Window.SwapBuffers();
    }

    LOG_INFO("Engine", "Main loop ended — total frames: {}, runtime: {:.1f}s",
             m_Time.FrameCount(), m_Time.TotalTime());
}

void EngineApp::Shutdown() {
    LOG_INFO("Engine", "Shutting down...");
    vkDeviceWaitIdle(m_VulkanCtx.GetDevice());
    m_Swapchain.Shutdown(m_VulkanCtx.GetDevice());
    m_VulkanCtx.Shutdown();
    m_Input.Shutdown();
    m_Window.Shutdown();
    LOG_INFO("Engine", "=== FY-Engine shutdown complete ===");
    Logger::Get().Shutdown();
}

} // namespace Engine
