#include "gpu/buffer.h"

#include "gpu/vk_context.h"

#include <cstring>
#include <iostream>

namespace recorz::gpu {

Buffer::~Buffer() {
    if (buffer_ != VK_NULL_HANDLE) {
        std::cerr << "Buffer destroyed without calling destroy().\n";
    }
}

uint32_t Buffer::findMemoryType(
    VkPhysicalDevice physicalDevice,
    uint32_t typeFilter,
    VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        if ((typeFilter & (1u << i)) &&
            (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    return UINT32_MAX;
}

bool Buffer::create(
    VkContext& vk,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties) {
    destroy(vk.device());

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(vk.device(), &bufferInfo, nullptr, &buffer_) != VK_SUCCESS) {
        std::cerr << "Failed to create buffer.\n";
        return false;
    }

    VkMemoryRequirements memoryRequirements{};
    vkGetBufferMemoryRequirements(vk.device(), buffer_, &memoryRequirements);

    const uint32_t memoryTypeIndex =
        findMemoryType(vk.physicalDevice(), memoryRequirements.memoryTypeBits, properties);
    if (memoryTypeIndex == UINT32_MAX) {
        std::cerr << "Failed to find suitable memory type for buffer.\n";
        destroy(vk.device());
        return false;
    }

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    if (vkAllocateMemory(vk.device(), &allocInfo, nullptr, &memory_) != VK_SUCCESS) {
        std::cerr << "Failed to allocate buffer memory.\n";
        destroy(vk.device());
        return false;
    }

    vkBindBufferMemory(vk.device(), buffer_, memory_, 0);
    size_ = size;
    memoryProperties_ = properties;
    return true;
}

bool Buffer::upload(VkContext& vk, const void* data, VkDeviceSize size) {
    if (buffer_ == VK_NULL_HANDLE || size > size_) {
        return false;
    }

    if ((memoryProperties_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0) {
        void* mapped = nullptr;
        if (vkMapMemory(vk.device(), memory_, 0, size, 0, &mapped) != VK_SUCCESS) {
            return false;
        }
        std::memcpy(mapped, data, static_cast<size_t>(size));
        vkUnmapMemory(vk.device(), memory_);
        return true;
    }

    Buffer staging{};
    if (!staging.create(
            vk,
            size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        return false;
    }
    if (!staging.upload(vk, data, size)) {
        staging.destroy(vk.device());
        return false;
    }

    VkCommandPool pool = VK_NULL_HANDLE;
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = vk.graphicsQueueFamily();
    if (vkCreateCommandPool(vk.device(), &poolInfo, nullptr, &pool) != VK_SUCCESS) {
        staging.destroy(vk.device());
        return false;
    }

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    if (vkAllocateCommandBuffers(vk.device(), &allocInfo, &commandBuffer) != VK_SUCCESS) {
        vkDestroyCommandPool(vk.device(), pool, nullptr);
        staging.destroy(vk.device());
        return false;
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, staging.handle(), buffer_, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkFence fence = VK_NULL_HANDLE;
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence(vk.device(), &fenceInfo, nullptr, &fence);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vkQueueSubmit(vk.graphicsQueue(), 1, &submitInfo, fence);
    vkWaitForFences(vk.device(), 1, &fence, VK_TRUE, UINT64_MAX);

    vkDestroyFence(vk.device(), fence, nullptr);
    vkFreeCommandBuffers(vk.device(), pool, 1, &commandBuffer);
    vkDestroyCommandPool(vk.device(), pool, nullptr);
    staging.destroy(vk.device());
    return true;
}

void Buffer::destroy(VkDevice device) {
    if (device == VK_NULL_HANDLE) {
        return;
    }
    if (buffer_ != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, buffer_, nullptr);
        buffer_ = VK_NULL_HANDLE;
    }
    if (memory_ != VK_NULL_HANDLE) {
        vkFreeMemory(device, memory_, nullptr);
        memory_ = VK_NULL_HANDLE;
    }
    size_ = 0;
    memoryProperties_ = 0;
}

} // namespace recorz::gpu
