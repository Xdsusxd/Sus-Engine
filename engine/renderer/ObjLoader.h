#pragma once

#include "renderer/Mesh.h"
#include <string>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace Engine {

class ObjLoader {
public:
    static Mesh LoadFromFile(const std::string& filepath,
                             VmaAllocator allocator, VkDevice device,
                             VkCommandPool cmdPool, VkQueue queue);
};

} // namespace Engine
