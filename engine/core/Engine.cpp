#include "core/Engine.h"
#include "core/Logger.h"
#include "renderer/ObjLoader.h"
#include "physics/PhysicsSystem.h"
#include "audio/AudioSystem.h"
#include "ui/UISystem.h"
#include <imgui.h>
#include "scene/CharacterController.h"
#include "scene/ProximityTriggerComponent.h"
#include "ai/EnemyAIComponent.h"
#include "animation/Skeleton.h"

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

    // ── UI ────────────────────────────────────────────────────
    if (!UISystem::Get().Init(m_VulkanCtx, m_Swapchain, m_Window.GetHandle())) {
        LOG_FATAL("Engine", "UI initialization failed!");
        return false;
    }

    // ── Physics ───────────────────────────────────────────────
    if (!PhysicsSystem::Get().Init()) {
        LOG_FATAL("Engine", "Physics initialization failed!");
        return false;
    }

    if (!AudioSystem::Get().Init()) {
        LOG_FATAL("Engine", "Audio initialization failed!");
        return false;
    }

    // Create a static floor (large flat box at y = 0)
    PhysicsBodyDesc floorDesc;
    floorDesc.type       = PhysicsBodyType::Static;
    floorDesc.position   = {0.0f, -0.5f, 0.0f};  // Top surface at y = 0
    floorDesc.halfExtent = {50.0f, 0.5f, 50.0f};  // 100x1x100 slab
    floorDesc.friction   = 0.8f;
    PhysicsSystem::Get().CreateBody(floorDesc);

    // ── Camera ────────────────────────────────────────────────
    float aspect = static_cast<float>(m_Swapchain.GetExtent().width) /
                   static_cast<float>(m_Swapchain.GetExtent().height);
    m_Camera.Init(60.0f, aspect, 0.01f, 1000.0f);
    m_Camera.SetMode(CameraMode::ThirdPerson);
    m_Camera.SetDistance(8.0f);

    // ── Assets ────────────────────────────────────────────────
    m_CubeMesh = Mesh::CreateCube(
        m_Renderer.GetAllocator(), m_VulkanCtx.GetDevice(),
        m_Renderer.GetCommandPool(), m_VulkanCtx.GetGraphicsQueue());

    m_PyramidMesh = ObjLoader::LoadFromFile("assets/models/pyramid.obj",
        m_Renderer.GetAllocator(), m_VulkanCtx.GetDevice(),
        m_Renderer.GetCommandPool(), m_VulkanCtx.GetGraphicsQueue());

    m_WhiteTexture.CreateSolidColor(m_VulkanCtx, m_Renderer, 255, 255, 255);

    // ── Scene Setup ───────────────────────────────────────────
    auto& centralCube = m_Scene.AddObject("Player");
    centralCube.mesh = &m_CubeMesh;
    centralCube.transform.position = {0.0f, 0.5f, 0.0f};

    auto& pyramid = m_Scene.AddObject("Pyramid");
    pyramid.mesh = &m_PyramidMesh;
    pyramid.transform.position = {0.0f, 2.0f, 0.0f};
    pyramid.transform.scale = {0.5f, 0.5f, 0.5f};
    pyramid.autoRotate = {0.0f, -90.0f, 0.0f};
    
    // Add trigger after adding player
    // wait, we need player to be added first, which it is.
    pyramid.AddComponent<ProximityTriggerComponent>(m_Scene.FindObject("Player"), 3.0f, "Welcome to the Pyramid Zone!");

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

    // ── Animated Stick Figure (5 bones) ───────────────────────
    // Bone 0: Pelvis (root)      — at (5, 1.0, 0)
    // Bone 1: Left Leg  (child)  — offset (0, -0.5, 0)
    // Bone 2: Right Leg (child)  — offset (0, -0.5, 0)
    // Bone 3: Left Arm  (child)  — offset (-0.6, 0.3, 0)
    // Bone 4: Right Arm (child)  — offset (0.6, 0.3, 0)
    m_PlayerSkeleton.AddBone("Pelvis", BONE_NO_PARENT,
        glm::translate(glm::mat4(1.0f), {5.0f, 1.0f, 0.0f}));
    m_PlayerSkeleton.AddBone("LeftLeg", 0,
        glm::translate(glm::mat4(1.0f), {-0.2f, -0.5f, 0.0f}));
    m_PlayerSkeleton.AddBone("RightLeg", 0,
        glm::translate(glm::mat4(1.0f), {0.2f, -0.5f, 0.0f}));
    m_PlayerSkeleton.AddBone("LeftArm", 0,
        glm::translate(glm::mat4(1.0f), {-0.6f, 0.3f, 0.0f}));
    m_PlayerSkeleton.AddBone("RightArm", 0,
        glm::translate(glm::mat4(1.0f), {0.6f, 0.3f, 0.0f}));

    // Create visual cubes for each bone
    const char* boneNames[] = {"BonePelvis", "BoneLeftLeg", "BoneRightLeg", "BoneLeftArm", "BoneRightArm"};
    const glm::vec3 boneSizes[] = {
        {0.4f, 0.3f, 0.25f},  // pelvis
        {0.15f, 0.4f, 0.15f}, // left leg
        {0.15f, 0.4f, 0.15f}, // right leg
        {0.12f, 0.35f, 0.12f},// left arm
        {0.12f, 0.35f, 0.12f} // right arm
    };
    for (int i = 0; i < 5; ++i) {
        auto& bone = m_Scene.AddObject(boneNames[i]);
        bone.mesh = &m_CubeMesh;
        bone.transform.scale = boneSizes[i];
    }

    // Initialize animator
    m_PlayerAnimator.Init(&m_PlayerSkeleton);
    m_PlayerAnimator.AddClip(AnimationClip::CreateIdleBob(0.08f, 1.5f));
    m_PlayerAnimator.AddClip(AnimationClip::CreateWalkCycle());
    m_PlayerAnimator.Play("IdleBob");

    // Initialize controller AFTER all scene objects are added
    if (auto* playerObj = m_Scene.FindObject("Player")) {
        playerObj->AddComponent<CharacterController>(&m_Camera, &m_Input);
        
        // Add an Enemy
        auto& enemy = m_Scene.AddObject("Enemy");
        enemy.mesh = &m_CubeMesh;
        enemy.transform.position = {5.0f, 0.5f, -5.0f};
        enemy.transform.scale = {0.8f, 1.2f, 0.8f};
        enemy.AddComponent<EnemyAIComponent>(playerObj);
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

        UISystem::Get().BeginFrame();

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

        // ── Physics ───────────────────────────────────────────
        PhysicsSystem::Get().Update(dt);

        // ── Animation ─────────────────────────────────────────
        bool isMoving = m_Input.IsKeyDown(Key::W) || m_Input.IsKeyDown(Key::S) || 
                        m_Input.IsKeyDown(Key::A) || m_Input.IsKeyDown(Key::D);
        static bool wasMoving = false;
        if (isMoving && !wasMoving) {
            m_PlayerAnimator.Play("WalkCycle", 0.2f);
            wasMoving = true;
        } else if (!isMoving && wasMoving) {
            m_PlayerAnimator.Play("IdleBob", 0.2f);
            wasMoving = false;
        }

        m_PlayerAnimator.Update(dt);

        // Sync visual bones to animated skeleton
        const char* boneNames[] = {"BonePelvis", "BoneLeftLeg", "BoneRightLeg", "BoneLeftArm", "BoneRightArm"};
        auto& worldMats = m_PlayerAnimator.GetWorldMatrices();
        auto* playerObj = m_Scene.FindObject("Player");
        if (playerObj) {
            glm::mat4 playerMat = playerObj->transform.GetMatrix();
            // Offset the stick figure so it's not exactly inside the player cube, but floating above/beside
            playerMat = glm::translate(playerMat, glm::vec3(0.0f, 1.5f, 0.0f)); 

            for (int i = 0; i < 5; ++i) {
                auto* boneObj = m_Scene.FindObject(boneNames[i]);
                if (boneObj) {
                    glm::mat4 finalMat = playerMat * worldMats[i];
                    // Extract position
                    boneObj->transform.position = glm::vec3(finalMat[3]);
                    // Very crude rotation extraction for visual purposes
                    boneObj->transform.rotation.y = playerObj->transform.rotation.y;
                }
            }
        }

        // ── Update scene & camera ─────────────────────────────
        m_Camera.Update(m_Input, dt);
        m_Scene.Update(dt);

        // ── Audio ─────────────────────────────────────────────
        glm::vec3 camPos = m_Camera.GetPosition();
        glm::vec3 camFwd = glm::normalize(m_Camera.GetTarget() - camPos);
        glm::vec3 camUp  = glm::vec3(0, 1, 0); // Simplified UP vector
        AudioSystem::Get().SetListenerPosition(camPos, camFwd, camUp);
        AudioSystem::Get().Update();

        // ── Render frame ──────────────────────────────────────
        if (m_Renderer.BeginFrame(m_VulkanCtx, m_Swapchain, m_Window.GetHandle())) {
            VkCommandBuffer cmd = m_Renderer.GetCurrentCmdBuffer();
            glm::mat4 viewProj = m_Camera.GetViewProjection();

            // ── 1. Draw grid ──────────────────────────────────
            m_GridPipeline.Draw(cmd, &viewProj);

            // ── 2. Draw scene objects ─────────────────────────
            m_Pipeline.Bind(cmd);
            m_Scene.Render(cmd, m_Pipeline, viewProj);

            // ── 3. Draw UI ────────────────────────────────────
            ImGui::Begin("Engine Metrics");
            ImGui::Text("FPS: %.1f", m_Time.FPS());
            ImGui::Text("Delta Time: %.3f ms", dt * 1000.0f);
            
            auto camPos = m_Camera.GetPosition();
            ImGui::Text("Camera Pos: (%.1f, %.1f, %.1f)", camPos.x, camPos.y, camPos.z);
            ImGui::End();

            UISystem::Get().EndFrame(cmd);

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
    
    PhysicsSystem::Get().Shutdown();
    AudioSystem::Get().Shutdown();
    UISystem::Get().Shutdown(m_VulkanCtx.GetDevice());

    m_Renderer.Shutdown(m_VulkanCtx.GetDevice());
    m_Swapchain.Shutdown(m_VulkanCtx.GetDevice());
    m_VulkanCtx.Shutdown();
    m_Input.Shutdown();
    m_Window.Shutdown();
    LOG_INFO("Engine", "=== FY-Engine shutdown complete ===");
    Logger::Get().Shutdown();
}

} // namespace Engine
