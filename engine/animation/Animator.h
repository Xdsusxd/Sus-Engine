#pragma once

#include "animation/Skeleton.h"
#include "animation/AnimationClip.h"
#include <vector>
#include <string>

namespace Engine {

// Blending mode for transitioning between animations
enum class AnimBlendMode {
    Override,  // Replace current animation
    Additive   // Add on top of current pose
};

class Animator {
public:
    void Init(const Skeleton* skeleton);

    // Clip management
    void Play(const std::string& clipName, float blendTime = 0.2f);
    void AddClip(const AnimationClip& clip);

    // Per-frame update
    void Update(float dt);

    // Get final bone matrices (local-to-world * inverse bind)
    // These are ready to be sent to the GPU for skinning
    const std::vector<glm::mat4>& GetBoneMatrices() const { return m_FinalMatrices; }

    // Get world-space bone positions (for debug visualization)
    const std::vector<glm::mat4>& GetWorldMatrices() const { return m_WorldMatrices; }

    float GetPlaybackTime() const { return m_CurrentTime; }
    float GetPlaybackSpeed() const { return m_Speed; }
    void  SetPlaybackSpeed(float speed) { m_Speed = speed; }

    bool IsPlaying() const { return m_CurrentClip != nullptr; }

private:
    void ComputePose();

    const Skeleton*         m_Skeleton    = nullptr;
    const AnimationClip*    m_CurrentClip = nullptr;
    const AnimationClip*    m_NextClip    = nullptr;
    float                   m_CurrentTime = 0.0f;
    float                   m_Speed       = 1.0f;

    // Blending
    float m_BlendTime     = 0.0f;
    float m_BlendProgress = 0.0f;

    std::vector<AnimationClip> m_Clips;
    std::vector<glm::mat4>     m_WorldMatrices;  // World-space transforms per bone
    std::vector<glm::mat4>     m_FinalMatrices;  // Final skinning matrices (world * inverseBind)
};

} // namespace Engine
