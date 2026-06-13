#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

namespace recorz::gpu {

class VkContext;

class Buffer {
public:
    Buffer() = default;
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    bool create(
        VkContext& vk,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties);

    bool upload(VkContext& vk, const void* data, VkDeviceSize size);

    void destroy(VkDevice device);

    VkBuffer handle() const { return buffer_; }
    VkDeviceSize size() const { return size_; }

private:
    uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VkBuffer buffer_ = VK_NULL_HANDLE;
    VkDeviceMemory memory_ = VK_NULL_HANDLE;
    VkDeviceSize size_ = 0;
    VkMemoryPropertyFlags memoryProperties_ = 0;
};

} // namespace recorz::gpu
