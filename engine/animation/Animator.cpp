#include "animation/Animator.h"
#include "core/Logger.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace Engine {

void Animator::Init(const Skeleton* skeleton) {
    m_Skeleton = skeleton;
    if (!m_Skeleton) return;

    int boneCount = m_Skeleton->GetBoneCount();
    m_WorldMatrices.resize(boneCount, glm::mat4(1.0f));
    m_FinalMatrices.resize(boneCount, glm::mat4(1.0f));

    LOG_INFO("Animator", "Initialized with {} bones", boneCount);
}

void Animator::AddClip(const AnimationClip& clip) {
    m_Clips.push_back(clip);
    LOG_INFO("Animator", "Added clip '{}' (duration: {:.2f}s, {} tracks)",
             clip.name, clip.duration, clip.GetTracks().size());
}

void Animator::Play(const std::string& clipName, float blendTime) {
    for (auto& clip : m_Clips) {
        if (clip.name == clipName) {
            if (m_CurrentClip && blendTime > 0.0f) {
                // Crossfade from current to new
                m_NextClip = &clip;
                m_BlendTime = blendTime;
                m_BlendProgress = 0.0f;
            } else {
                m_CurrentClip = &clip;
                m_CurrentTime = 0.0f;
            }
            return;
        }
    }
    LOG_WARN("Animator", "Clip '{}' not found", clipName);
}

void Animator::Update(float dt) {
    if (!m_Skeleton || !m_CurrentClip) return;

    m_CurrentTime += dt * m_Speed;

    // Handle blending transition
    if (m_NextClip) {
        m_BlendProgress += dt / m_BlendTime;
        if (m_BlendProgress >= 1.0f) {
            m_CurrentClip = m_NextClip;
            m_NextClip = nullptr;
            m_CurrentTime = m_BlendProgress * m_CurrentClip->duration;
            m_BlendProgress = 0.0f;
        }
    }

    ComputePose();
}

void Animator::ComputePose() {
    if (!m_Skeleton || !m_CurrentClip) return;

    int boneCount = m_Skeleton->GetBoneCount();
    auto& bones = m_Skeleton->GetBones();

    // For each bone, sample the animation to get local transform
    for (int i = 0; i < boneCount; ++i) {
        glm::vec3 pos(0.0f);
        glm::quat rot(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 scl(1.0f);

        bool sampled = m_CurrentClip->Sample(i, m_CurrentTime, pos, rot, scl);

        // If blending, also sample the next clip and interpolate
        if (m_NextClip && m_BlendProgress > 0.0f) {
            glm::vec3 pos2(0.0f);
            glm::quat rot2(1.0f, 0.0f, 0.0f, 0.0f);
            glm::vec3 scl2(1.0f);

            bool sampled2 = m_NextClip->Sample(i, m_CurrentTime, pos2, rot2, scl2);
            if (sampled && sampled2) {
                pos = glm::mix(pos, pos2, m_BlendProgress);
                rot = glm::slerp(rot, rot2, m_BlendProgress);
                scl = glm::mix(scl, scl2, m_BlendProgress);
            } else if (sampled2) {
                pos = pos2;
                rot = rot2;
                scl = scl2;
                sampled = true;
            }
        }

        // Build local matrix
        glm::mat4 localMatrix(1.0f);
        if (sampled) {
            localMatrix = glm::translate(glm::mat4(1.0f), pos) *
                          glm::mat4_cast(rot) *
                          glm::scale(glm::mat4(1.0f), scl);
        } else {
            // Use bind pose if no animation data for this bone
            localMatrix = bones[i].localBindTransform;
        }

        // Compute world matrix
        if (bones[i].parentIndex == BONE_NO_PARENT) {
            m_WorldMatrices[i] = localMatrix;
        } else {
            m_WorldMatrices[i] = m_WorldMatrices[bones[i].parentIndex] * localMatrix;
        }

        // Final skinning matrix = worldMatrix * inverseBindMatrix
        m_FinalMatrices[i] = m_WorldMatrices[i] * bones[i].inverseBindMatrix;
    }
}

} // namespace Engine
