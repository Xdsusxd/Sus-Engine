#include "renderer/ObjLoader.h"
#include "core/Logger.h"

#include <fstream>
#include <sstream>
#include <unordered_map>

namespace Engine {

struct FaceVertex {
    int posIdx = -1;
    int uvIdx  = -1;
    int normIdx = -1;

    bool operator==(const FaceVertex& o) const {
        return posIdx == o.posIdx && uvIdx == o.uvIdx && normIdx == o.normIdx;
    }
};

struct FaceVertexHash {
    size_t operator()(const FaceVertex& fv) const {
        size_t h = std::hash<int>()(fv.posIdx);
        h ^= std::hash<int>()(fv.uvIdx)  << 1;
        h ^= std::hash<int>()(fv.normIdx) << 2;
        return h;
    }
};

static FaceVertex ParseFaceToken(const std::string& token) {
    FaceVertex fv;
    std::istringstream ss(token);
    std::string part;

    // v/vt/vn or v//vn or v/vt or v
    if (std::getline(ss, part, '/')) {
        if (!part.empty()) fv.posIdx = std::stoi(part) - 1;
    }
    if (std::getline(ss, part, '/')) {
        if (!part.empty()) fv.uvIdx = std::stoi(part) - 1;
    }
    if (std::getline(ss, part, '/')) {
        if (!part.empty()) fv.normIdx = std::stoi(part) - 1;
    }

    return fv;
}

Mesh ObjLoader::LoadFromFile(const std::string& filepath,
                              VmaAllocator allocator, VkDevice device,
                              VkCommandPool cmdPool, VkQueue queue)
{
    std::ifstream file(filepath);
    if (!file.is_open()) {
        LOG_ERROR("ObjLoader", "Failed to open: {}", filepath);
        return {};
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;

    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<FaceVertex, uint32_t, FaceVertexHash> vertexCache;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v") {
            glm::vec3 pos;
            ss >> pos.x >> pos.y >> pos.z;
            positions.push_back(pos);
        }
        else if (prefix == "vn") {
            glm::vec3 n;
            ss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (prefix == "vt") {
            glm::vec2 uv;
            ss >> uv.x >> uv.y;
            uvs.push_back(uv);
        }
        else if (prefix == "f") {
            std::vector<FaceVertex> faceVerts;
            std::string token;
            while (ss >> token) {
                faceVerts.push_back(ParseFaceToken(token));
            }

            // Triangulate (fan triangulation for convex polygons)
            for (size_t i = 1; i + 1 < faceVerts.size(); i++) {
                FaceVertex tri[3] = { faceVerts[0], faceVerts[i], faceVerts[i + 1] };

                for (auto& fv : tri) {
                    auto it = vertexCache.find(fv);
                    if (it != vertexCache.end()) {
                        indices.push_back(it->second);
                    } else {
                        Vertex v{};
                        if (fv.posIdx >= 0 && fv.posIdx < static_cast<int>(positions.size()))
                            v.position = positions[fv.posIdx];
                        if (fv.uvIdx >= 0 && fv.uvIdx < static_cast<int>(uvs.size()))
                            v.uv = uvs[fv.uvIdx];
                        if (fv.normIdx >= 0 && fv.normIdx < static_cast<int>(normals.size()))
                            v.normal = normals[fv.normIdx];

                        v.color = {1.0f, 1.0f, 1.0f}; // Default white

                        uint32_t idx = static_cast<uint32_t>(vertices.size());
                        vertices.push_back(v);
                        vertexCache[fv] = idx;
                        indices.push_back(idx);
                    }
                }
            }
        }
    }

    // Auto-generate normals if none were present
    if (normals.empty() && !indices.empty()) {
        // Reset all normals to zero
        for (auto& v : vertices) v.normal = glm::vec3(0.0f);

        for (size_t i = 0; i + 2 < indices.size(); i += 3) {
            auto& v0 = vertices[indices[i]];
            auto& v1 = vertices[indices[i + 1]];
            auto& v2 = vertices[indices[i + 2]];

            glm::vec3 edge1 = v1.position - v0.position;
            glm::vec3 edge2 = v2.position - v0.position;
            glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

            v0.normal += faceNormal;
            v1.normal += faceNormal;
            v2.normal += faceNormal;
        }
        for (auto& v : vertices) {
            if (glm::length(v.normal) > 0.001f)
                v.normal = glm::normalize(v.normal);
        }
    }

    if (vertices.empty()) {
        LOG_ERROR("ObjLoader", "No vertices parsed from: {}", filepath);
        return {};
    }

    Mesh mesh;
    mesh.Create(allocator, device, cmdPool, queue, vertices, indices);
    LOG_INFO("ObjLoader", "Loaded OBJ: {} ({} verts, {} tris)",
             filepath, vertices.size(), indices.size() / 3);
    return mesh;
}

} // namespace Engine
