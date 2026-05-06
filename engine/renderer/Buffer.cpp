#include "renderer/Buffer.h"
#include "renderer/VkCheck.h"
#include "core/Logger.h"

namespace Engine {

void Buffer::Create(VmaAllocator allocator, VkDeviceSize size, BufferUsage usage) {
    m_Allocator = allocator;
    m_Size = size;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size  = size;

    VmaAllocationCreateInfo allocInfo{};

    switch (usage) {
        case BufferUsage::Vertex:
            bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            allocInfo.usage  = VMA_MEMORY_USAGE_AUTO;
            allocInfo.flags  = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
            break;
        case BufferUsage::DynamicVertex:
            bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            allocInfo.usage  = VMA_MEMORY_USAGE_AUTO;
            allocInfo.flags  = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                               VMA_ALLOCATION_CREATE_MAPPED_BIT;
            break;
        case BufferUsage::Index:
            bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            allocInfo.usage  = VMA_MEMORY_USAGE_AUTO;
            allocInfo.flags  = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
            break;
        case BufferUsage::Uniform:
            bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            allocInfo.usage  = VMA_MEMORY_USAGE_AUTO;
            allocInfo.flags  = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                               VMA_ALLOCATION_CREATE_MAPPED_BIT;
            break;
        case BufferUsage::Staging:
            bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            allocInfo.usage  = VMA_MEMORY_USAGE_AUTO;
            allocInfo.flags  = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                               VMA_ALLOCATION_CREATE_MAPPED_BIT;
            break;
    }

    VK_CHECK_VOID(vmaCreateBuffer(allocator, &bufferInfo, &allocInfo,
                                   &m_Buffer, &m_Allocation, nullptr));
}

void Buffer::Destroy() {
    if (m_Buffer != VK_NULL_HANDLE && m_Allocator != VK_NULL_HANDLE) {
        vmaDestroyBuffer(m_Allocator, m_Buffer, m_Allocation);
        m_Buffer     = VK_NULL_HANDLE;
        m_Allocation = VK_NULL_HANDLE;
    }
}

void Buffer::Upload(const void* data, VkDeviceSize size) {
    void* mapped = nullptr;
    VK_CHECK_VOID(vmaMapMemory(m_Allocator, m_Allocation, &mapped));
    std::memcpy(mapped, data, static_cast<size_t>(size));
    vmaUnmapMemory(m_Allocator, m_Allocation);
}

void Buffer::CopyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue queue,
                         VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool        = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(device, &allocInfo, &cmd);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(cmd, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submitInfo{};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &cmd;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(device, commandPool, 1, &cmd);
}

} // namespace Engine
