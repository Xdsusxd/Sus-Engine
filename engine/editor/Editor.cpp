#include "editor/Editor.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "scene/Component.h"
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Engine {

void Editor::Init() {
}

void Editor::Shutdown() {
}

void Editor::Render(Scene& scene, float fps, float dt, const glm::vec3& camPos) {
    DrawDockSpace();
    DrawHierarchy(scene);
    DrawInspector();
    DrawMetrics(fps, dt, camPos);
}

void Editor::DrawDockSpace() {
    static bool dockspaceOpen = true;
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen) {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    } else {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);

    if (!opt_padding)
        ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) { /* TODO: Trigger engine shutdown */ }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::End();
}

void Editor::DrawHierarchy(Scene& scene) {
    ImGui::Begin("Hierarchy");

    const auto& entities = scene.GetEntities();
    for (const auto& entity : entities) {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (m_SelectedEntity == entity.get()) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        // Just use name for now, no children tree implemented yet
        bool opened = ImGui::TreeNodeEx((void*)(intptr_t)entity->id, flags, "%s", entity->name.c_str());
        
        if (ImGui::IsItemClicked()) {
            m_SelectedEntity = entity.get();
        }

        if (opened) {
            ImGui::TreePop();
        }
    }

    // Click on empty space in window to deselect
    if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
        m_SelectedEntity = nullptr;
    }

    ImGui::End();
}

void Editor::DrawInspector() {
    ImGui::Begin("Inspector");

    if (m_SelectedEntity) {
        // Name
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        strncpy(buffer, m_SelectedEntity->name.c_str(), sizeof(buffer) - 1);
        if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
            m_SelectedEntity->name = std::string(buffer);
        }

        ImGui::Separator();

        // Transform
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat3("Position", glm::value_ptr(m_SelectedEntity->transform.position), 0.1f);
            
            // Rotation as Euler angles (in degrees for UI)
            glm::vec3 eulerDegrees = glm::degrees(glm::eulerAngles(m_SelectedEntity->transform.rotation));
            if (ImGui::DragFloat3("Rotation", glm::value_ptr(eulerDegrees), 1.0f)) {
                m_SelectedEntity->transform.rotation = glm::quat(glm::radians(eulerDegrees));
            }

            ImGui::DragFloat3("Scale", glm::value_ptr(m_SelectedEntity->transform.scale), 0.1f);
        }

        // TODO: List all components and draw their inspector UI
        // Currently, we'd need reflection or virtual DrawInspector() methods on components.
        // For now, we'll just show transform.
    } else {
        ImGui::TextDisabled("No entity selected");
    }

    ImGui::End();
}

void Editor::DrawMetrics(float fps, float dt, const glm::vec3& camPos) {
    ImGui::Begin("Engine Metrics");
    ImGui::Text("FPS: %.1f", fps);
    ImGui::Text("Delta Time: %.3f ms", dt * 1000.0f);
    ImGui::Text("Camera Pos: (%.1f, %.1f, %.1f)", camPos.x, camPos.y, camPos.z);
    ImGui::End();
}

} // namespace Engine
