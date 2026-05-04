#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

namespace Engine {

// A single keyframe for one bone
struct BoneKeyframe {
    float     time = 0.0f;
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // identity
    glm::vec3 scale    = {1.0f, 1.0f, 1.0f};
};

// All keyframes for a single bone within one animation
struct BoneTrack {
    int boneIndex = -1;
    std::vector<BoneKeyframe> keyframes;
};

// A complete animation clip (e.g., "walk", "idle", "attack")
class AnimationClip {
public:
    std::string name = "Untitled";
    float       duration = 0.0f;
    bool        looping  = true;

    void AddTrack(int boneIndex, const std::vector<BoneKeyframe>& keyframes);
    const BoneTrack* GetTrack(int boneIndex) const;
    const std::vector<BoneTrack>& GetTracks() const { return m_Tracks; }

    // Sample a bone's transform at a given time (with interpolation)
    bool Sample(int boneIndex, float time,
                glm::vec3& outPos, glm::quat& outRot, glm::vec3& outScale) const;

    // ── Factory: Create procedural animations for testing ─────
    static AnimationClip CreateIdleBob(float amplitude = 0.1f, float frequency = 2.0f);
    static AnimationClip CreateWalkCycle();

private:
    std::vector<BoneTrack> m_Tracks;
};

} // namespace Engine
