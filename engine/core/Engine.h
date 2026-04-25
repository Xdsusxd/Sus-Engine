#pragma once

#include "core/Window.h"
#include "core/Input.h"
#include "core/Time.h"
#include "renderer/VulkanContext.h"
#include "renderer/Swapchain.h"

namespace Engine {

struct EngineConfig {
    WindowConfig window;
    std::string  logFile = "fyengine.log";
};

class EngineApp {
public:
    bool Init(const EngineConfig& config = {});
    void Run();
    void Shutdown();

    // Accessors for subsystems
    Window&        GetWindow()        { return m_Window; }
    Input&         GetInput()         { return m_Input; }
    Time&          GetTime()          { return m_Time; }
    VulkanContext& GetVulkanContext() { return m_VulkanCtx; }
    Swapchain&     GetSwapchain()     { return m_Swapchain; }

private:
    Window        m_Window;
    Input         m_Input;
    Time          m_Time;
    VulkanContext m_VulkanCtx;
    Swapchain     m_Swapchain;
    bool          m_Running = false;
};

} // namespace Engine
