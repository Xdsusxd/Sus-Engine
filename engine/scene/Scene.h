#pragma once

#include "scene/SceneObject.h"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

namespace Engine {

class Pipeline;

class Scene {
public:
    SceneObject& AddObject(const std::string& name);
    SceneObject* FindObject(const std::string& name);
    void RemoveObject(const std::string& name);

    void Update(float dt);
    void Render(VkCommandBuffer cmd, const Pipeline& pipeline, const glm::mat4& viewProj);

    std::vector<SceneObject>& GetObjects() { return m_Objects; }
    size_t ObjectCount() const { return m_Objects.size(); }

private:
    std::vector<SceneObject> m_Objects;
};

} // namespace Engine
