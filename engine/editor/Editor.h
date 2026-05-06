#pragma once

#include <string>

namespace Engine {

class Scene;

class Editor {
public:
    void Init();
    void Shutdown();

    // Renders all editor windows using ImGui.
    // Should be called between UISystem::BeginFrame() and UISystem::EndFrame()
    void Render(Scene& scene, float fps, float dt, const struct glm::vec3& camPos);

private:
    void DrawDockSpace();
    void DrawHierarchy(Scene& scene);
    void DrawInspector();
    void DrawMetrics(float fps, float dt, const struct glm::vec3& camPos);
    
    // Tracks the currently selected entity ID or pointer
    // For simplicity, we just store a pointer to the selected entity
    class Entity* m_SelectedEntity = nullptr;
};

} // namespace Engine
