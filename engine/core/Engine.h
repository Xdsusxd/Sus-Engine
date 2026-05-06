#pragma once

#include "core/Window.h"
#include "core/Input.h"
#include "core/Time.h"
#include "renderer/VulkanContext.h"
#include "renderer/Swapchain.h"
#include "renderer/Renderer.h"
#include "renderer/Pipeline.h"
#include "renderer/GridPipeline.h"
#include "renderer/ParticlePipeline.h"
#include "scene/ParticleSystem.h"
#include "renderer/Camera.h"
#include "renderer/Mesh.h"
#include "renderer/Texture.h"
#include "scene/Scene.h"
#include "scene/CharacterController.h"
#include "animation/Skeleton.h"
#include "animation/Animator.h"
#include "editor/Editor.h"

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
    Renderer&      GetRenderer()      { return m_Renderer; }
    Camera&        GetCamera()        { return m_Camera; }
    Scene&         GetScene()         { return m_Scene; }

private:
    Window        m_Window;
    Input         m_Input;
    Time          m_Time;
    VulkanContext m_VulkanCtx;
    Swapchain     m_Swapchain;
    Renderer      m_Renderer;
    Pipeline      m_Pipeline;
    GridPipeline  m_GridPipeline;
    ParticlePipeline m_ParticlePipeline;
    Camera        m_Camera;
    Scene         m_Scene;
    Mesh          m_CubeMesh;
    Mesh          m_PyramidMesh;
    Texture       m_WhiteTexture;
    Skeleton      m_PlayerSkeleton;
    Animator      m_PlayerAnimator;
    bool          m_Running = false;
};

} // namespace Engine
