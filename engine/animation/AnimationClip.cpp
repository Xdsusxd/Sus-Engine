#include "animation/AnimationClip.h"

#include <algorithm>
#include <cmath>

namespace Engine {

void AnimationClip::AddTrack(int boneIndex, const std::vector<BoneKeyframe>& keyframes) {
    BoneTrack track;
    track.boneIndex = boneIndex;
    track.keyframes = keyframes;
    m_Tracks.push_back(std::move(track));

    // Update duration
    for (auto& kf : m_Tracks.back().keyframes) {
        if (kf.time > duration) duration = kf.time;
    }
}

const BoneTrack* AnimationClip::GetTrack(int boneIndex) const {
    for (auto& track : m_Tracks) {
        if (track.boneIndex == boneIndex) return &track;
    }
    return nullptr;
}

bool AnimationClip::Sample(int boneIndex, float time,
                           glm::vec3& outPos, glm::quat& outRot, glm::vec3& outScale) const {
    const BoneTrack* track = GetTrack(boneIndex);
    if (!track || track->keyframes.empty()) return false;

    // Handle looping
    float t = time;
    if (looping && duration > 0.0f) {
        t = fmodf(t, duration);
        if (t < 0.0f) t += duration;
    } else {
        t = std::clamp(t, 0.0f, duration);
    }

    auto& kfs = track->keyframes;

    // Single keyframe
    if (kfs.size() == 1) {
        outPos   = kfs[0].position;
        outRot   = kfs[0].rotation;
        outScale = kfs[0].scale;
        return true;
    }

    // Find surrounding keyframes
    int nextIdx = 0;
    for (int i = 0; i < static_cast<int>(kfs.size()); ++i) {
        if (kfs[i].time > t) {
            nextIdx = i;
            break;
        }
        nextIdx = i; // last one if time is past all keyframes
    }
    int prevIdx = (nextIdx > 0) ? nextIdx - 1 : (looping ? static_cast<int>(kfs.size()) - 1 : 0);

    // Interpolation factor
    float prevTime = kfs[prevIdx].time;
    float nextTime = kfs[nextIdx].time;
    float alpha = 0.0f;
    
    if (nextTime != prevTime) {
        float span = nextTime - prevTime;
        if (span < 0.0f) span += duration; // wrapping
        float elapsed = t - prevTime;
        if (elapsed < 0.0f) elapsed += duration;
        alpha = elapsed / span;
    }
    alpha = std::clamp(alpha, 0.0f, 1.0f);

    // Lerp position and scale, Slerp rotation
    outPos   = glm::mix(kfs[prevIdx].position, kfs[nextIdx].position, alpha);
    outRot   = glm::slerp(kfs[prevIdx].rotation, kfs[nextIdx].rotation, alpha);
    outScale = glm::mix(kfs[prevIdx].scale, kfs[nextIdx].scale, alpha);

    return true;
}

// ── Factory: Idle bobbing animation ──────────────────────────────────────

AnimationClip AnimationClip::CreateIdleBob(float amplitude, float frequency) {
    AnimationClip clip;
    clip.name = "IdleBob";
    clip.looping = true;

    float period = 1.0f / frequency;
    clip.duration = period;

    // Bone 0: Root — bobs up and down
    std::vector<BoneKeyframe> rootKFs;
    int steps = 8;
    for (int i = 0; i <= steps; ++i) {
        float t = (static_cast<float>(i) / steps) * period;
        BoneKeyframe kf;
        kf.time = t;
        kf.position = {0.0f, sinf(t * frequency * 2.0f * 3.14159f) * amplitude, 0.0f};
        kf.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        kf.scale    = {1.0f, 1.0f, 1.0f};
        rootKFs.push_back(kf);
    }
    clip.AddTrack(0, rootKFs);

    return clip;
}

// ── Factory: Walk cycle animation ────────────────────────────────────────

AnimationClip AnimationClip::CreateWalkCycle() {
    AnimationClip clip;
    clip.name = "WalkCycle";
    clip.looping = true;
    clip.duration = 1.0f; // 1 second per cycle

    const float PI = 3.14159265f;

    // Bone 0: Root (pelvis) — slight bounce
    {
        std::vector<BoneKeyframe> kfs;
        for (int i = 0; i <= 8; ++i) {
            float t = static_cast<float>(i) / 8.0f;
            BoneKeyframe kf;
            kf.time = t;
            kf.position = {0.0f, fabsf(sinf(t * 2.0f * PI * 2.0f)) * 0.05f, 0.0f};
            kf.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
            kf.scale = {1.0f, 1.0f, 1.0f};
            kfs.push_back(kf);
        }
        clip.AddTrack(0, kfs);
    }

    // Bone 1: Left leg — swing forward/backward
    {
        std::vector<BoneKeyframe> kfs;
        for (int i = 0; i <= 8; ++i) {
            float t = static_cast<float>(i) / 8.0f;
            float angle = sinf(t * 2.0f * PI) * 30.0f; // +/- 30 degrees
            BoneKeyframe kf;
            kf.time = t;
            kf.position = {0.0f, 0.0f, 0.0f};
            kf.rotation = glm::angleAxis(glm::radians(angle), glm::vec3(1, 0, 0));
            kf.scale = {1.0f, 1.0f, 1.0f};
            kfs.push_back(kf);
        }
        clip.AddTrack(1, kfs);
    }

    // Bone 2: Right leg — opposite phase
    {
        std::vector<BoneKeyframe> kfs;
        for (int i = 0; i <= 8; ++i) {
            float t = static_cast<float>(i) / 8.0f;
            float angle = sinf(t * 2.0f * PI + PI) * 30.0f; // opposite phase
            BoneKeyframe kf;
            kf.time = t;
            kf.position = {0.0f, 0.0f, 0.0f};
            kf.rotation = glm::angleAxis(glm::radians(angle), glm::vec3(1, 0, 0));
            kf.scale = {1.0f, 1.0f, 1.0f};
            kfs.push_back(kf);
        }
        clip.AddTrack(2, kfs);
    }

    // Bone 3: Left arm — opposite to left leg
    {
        std::vector<BoneKeyframe> kfs;
        for (int i = 0; i <= 8; ++i) {
            float t = static_cast<float>(i) / 8.0f;
            float angle = sinf(t * 2.0f * PI + PI) * 20.0f;
            BoneKeyframe kf;
            kf.time = t;
            kf.position = {0.0f, 0.0f, 0.0f};
            kf.rotation = glm::angleAxis(glm::radians(angle), glm::vec3(1, 0, 0));
            kf.scale = {1.0f, 1.0f, 1.0f};
            kfs.push_back(kf);
        }
        clip.AddTrack(3, kfs);
    }

    // Bone 4: Right arm — same phase as left leg
    {
        std::vector<BoneKeyframe> kfs;
        for (int i = 0; i <= 8; ++i) {
            float t = static_cast<float>(i) / 8.0f;
            float angle = sinf(t * 2.0f * PI) * 20.0f;
            BoneKeyframe kf;
            kf.time = t;
            kf.position = {0.0f, 0.0f, 0.0f};
            kf.rotation = glm::angleAxis(glm::radians(angle), glm::vec3(1, 0, 0));
            kf.scale = {1.0f, 1.0f, 1.0f};
            kfs.push_back(kf);
        }
        clip.AddTrack(4, kfs);
    }

    return clip;
}

} // namespace Engine
