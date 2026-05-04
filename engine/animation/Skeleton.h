#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

namespace Engine {

static constexpr int MAX_BONES = 64;
static constexpr int BONE_NO_PARENT = -1;

struct Bone {
    std::string name;
    int         parentIndex = BONE_NO_PARENT;
    glm::mat4   inverseBindMatrix = glm::mat4(1.0f);
    glm::mat4   localBindTransform = glm::mat4(1.0f);
};

class Skeleton {
public:
    void AddBone(const std::string& name, int parentIndex,
                 const glm::mat4& localBindTransform);

    int FindBoneIndex(const std::string& name) const;
    const Bone& GetBone(int index) const { return m_Bones[index]; }
    int GetBoneCount() const { return static_cast<int>(m_Bones.size()); }
    const std::vector<Bone>& GetBones() const { return m_Bones; }

    // Compute world-space bind pose matrices (for debug visualization)
    void ComputeBindPose(std::vector<glm::mat4>& outWorldMatrices) const;

private:
    std::vector<Bone> m_Bones;
};

} // namespace Engine
