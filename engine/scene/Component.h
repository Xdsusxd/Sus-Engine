#pragma once

namespace Engine {

struct SceneObject;

class Component {
public:
    virtual ~Component() = default;

    virtual void OnStart() {}
    virtual void OnUpdate(float dt) {}
    virtual void OnDestroy() {}

    SceneObject* GetOwner() const { return m_Owner; }

protected:
    friend struct SceneObject;
    SceneObject* m_Owner = nullptr;
};

} // namespace Engine
