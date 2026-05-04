#pragma once

#include "scene/Component.h"
#include "scene/SceneObject.h"
#include "core/Logger.h"
#include <glm/glm.hpp>
#include <string>

namespace Engine {

class ProximityTriggerComponent : public Component {
public:
    ProximityTriggerComponent(SceneObject* target, float radius, const std::string& message)
        : m_Target(target), m_Radius(radius), m_Message(message) {}

    void OnUpdate(float dt) override {
        if (!m_Owner || !m_Target) return;

        float distance = glm::distance(m_Owner->transform.position, m_Target->transform.position);
        
        bool isInside = (distance <= m_Radius);
        if (isInside && !m_WasInside) {
            LOG_INFO("Gameplay", "Trigger Entered ({}): {}", m_Owner->name, m_Message);
            m_WasInside = true;
        } else if (!isInside && m_WasInside) {
            LOG_INFO("Gameplay", "Trigger Exited ({})", m_Owner->name);
            m_WasInside = false;
        }
    }

private:
    SceneObject* m_Target = nullptr;
    float        m_Radius = 0.0f;
    std::string  m_Message;
    bool         m_WasInside = false;
};

} // namespace Engine
