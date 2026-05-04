#include "animation/Skeleton.h"
#include "core/Logger.h"

namespace Engine {

void Skeleton::AddBone(const std::string& name, int parentIndex,
                       const glm::mat4& localBindTransform) {
    Bone bone;
    bone.name = name;
    bone.parentIndex = parentIndex;
    bone.localBindTransform = localBindTransform;

    // Compute inverse bind matrix: need world-space bind transform first
    // We'll compute it from the chain of parents
    glm::mat4 worldBind = localBindTransform;
    if (parentIndex >= 0 && parentIndex < static_cast<int>(m_Bones.size())) {
        // Walk up the parent chain to get world transform
        std::vector<int> chain;
        int idx = parentIndex;
        while (idx != BONE_NO_PARENT) {
            chain.push_back(idx);
            idx = m_Bones[idx].parentIndex;
        }
        // Apply from root to parent
        glm::mat4 parentWorld(1.0f);
        for (int i = static_cast<int>(chain.size()) - 1; i >= 0; --i) {
            parentWorld *= m_Bones[chain[i]].localBindTransform;
        }
        worldBind = parentWorld * localBindTransform;
    }
    bone.inverseBindMatrix = glm::inverse(worldBind);

    m_Bones.push_back(bone);
    LOG_INFO("Skeleton", "Added bone '{}' (index: {}, parent: {})",
             name, m_Bones.size() - 1, parentIndex);
}

int Skeleton::FindBoneIndex(const std::string& name) const {
    for (int i = 0; i < static_cast<int>(m_Bones.size()); ++i) {
        if (m_Bones[i].name == name) return i;
    }
    return -1;
}

void Skeleton::ComputeBindPose(std::vector<glm::mat4>& outWorldMatrices) const {
    outWorldMatrices.resize(m_Bones.size(), glm::mat4(1.0f));

    for (int i = 0; i < static_cast<int>(m_Bones.size()); ++i) {
        if (m_Bones[i].parentIndex == BONE_NO_PARENT) {
            outWorldMatrices[i] = m_Bones[i].localBindTransform;
        } else {
            outWorldMatrices[i] = outWorldMatrices[m_Bones[i].parentIndex] *
                                   m_Bones[i].localBindTransform;
        }
    }
}

} // namespace Engine
