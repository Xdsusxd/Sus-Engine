#include "scene/Scene.h"
#include "renderer/Pipeline.h"
#include "renderer/Mesh.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace Engine {

SceneObject& Scene::AddObject(const std::string& name) {
    auto obj = std::make_unique<SceneObject>();
    obj->name = name;
    SceneObject& ref = *obj;
    m_Objects.push_back(std::move(obj));
    return ref;
}

SceneObject* Scene::FindObject(const std::string& name) {
    for (auto& obj : m_Objects) {
        if (obj->name == name) return obj.get();
    }
    return nullptr;
}

void Scene::RemoveObject(const std::string& name) {
    m_Objects.erase(
        std::remove_if(m_Objects.begin(), m_Objects.end(),
                        [&](const std::unique_ptr<SceneObject>& o) { return o->name == name; }),
        m_Objects.end());
}

void Scene::Update(float dt) {
    for (auto& obj : m_Objects) {
        if (obj->autoRotate.x != 0.0f || obj->autoRotate.y != 0.0f || obj->autoRotate.z != 0.0f) {
            obj->transform.rotation += obj->autoRotate * dt;
        }
        obj->UpdateComponents(dt);
    }
}

void Scene::Render(VkCommandBuffer cmd, const Pipeline& pipeline, const glm::mat4& viewProj) {
    for (const auto& obj : m_Objects) {
        if (!obj->visible || !obj->mesh) continue;

        glm::mat4 model = obj->transform.GetMatrix();
        glm::mat4 mvp   = viewProj * model;

        pipeline.PushMVP(cmd, &mvp);
        obj->mesh->Bind(cmd);
        obj->mesh->Draw(cmd);
    }
}

} // namespace Engine
