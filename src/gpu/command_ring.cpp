#include "gpu/command_ring.h"

#include <iostream>

namespace recorz::gpu {

CommandRing::~CommandRing() {
    if (device_ != VK_NULL_HANDLE) {
        std::cerr << "CommandRing destroyed without calling destroy().\n";
    }
}

bool CommandRing::create(VkDevice device, uint32_t queueFamilyIndex) {
    destroy(device);

    device_ = device;

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndex;

    if (vkCreateCommandPool(device_, &poolInfo, nullptr, &pool_) != VK_SUCCESS) {
        std::cerr << "Failed to create command pool.\n";
        device_ = VK_NULL_HANDLE;
        return false;
    }

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pool_;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = kFramesInFlight;

    if (vkAllocateCommandBuffers(device_, &allocInfo, buffers_) != VK_SUCCESS) {
        std::cerr << "Failed to allocate command buffers.\n";
        destroy(device_);
        return false;
    }

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < kFramesInFlight; ++i) {
        if (vkCreateFence(device_, &fenceInfo, nullptr, &fences_[i]) != VK_SUCCESS) {
            std::cerr << "Failed to create fence " << i << ".\n";
            destroy(device_);
            return false;
        }
    }

    return true;
}

void CommandRing::destroy(VkDevice device) {
    if (device != VK_NULL_HANDLE && device != device_) {
        device = device_;
    }

    if (device_ == VK_NULL_HANDLE) {
        return;
    }

    vkDeviceWaitIdle(device_);

    for (uint32_t i = 0; i < kFramesInFlight; ++i) {
        if (fences_[i] != VK_NULL_HANDLE) {
            vkDestroyFence(device_, fences_[i], nullptr);
            fences_[i] = VK_NULL_HANDLE;
        }
    }

    if (pool_ != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device_, pool_, nullptr);
        pool_ = VK_NULL_HANDLE;
    }

    device_ = VK_NULL_HANDLE;
    currentFrame_ = 0;
}

VkCommandBuffer CommandRing::begin(uint32_t frameIndex) {
    if (frameIndex >= kFramesInFlight || device_ == VK_NULL_HANDLE) {
        return VK_NULL_HANDLE;
    }

    vkWaitForFences(device_, 1, &fences_[frameIndex], VK_TRUE, UINT64_MAX);
    vkResetFences(device_, 1, &fences_[frameIndex]);

    VkCommandBuffer cmd = buffers_[frameIndex];
    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) {
        std::cerr << "Failed to begin command buffer.\n";
        return VK_NULL_HANDLE;
    }

    return cmd;
}

bool CommandRing::submit(VkQueue queue, uint32_t frameIndex) {
    if (frameIndex >= kFramesInFlight) {
        return false;
    }

    VkCommandBuffer cmd = buffers_[frameIndex];
    if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
        std::cerr << "Failed to end command buffer.\n";
        return false;
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    if (vkQueueSubmit(queue, 1, &submitInfo, fences_[frameIndex]) != VK_SUCCESS) {
        std::cerr << "Failed to submit command buffer.\n";
        return false;
    }

    return true;
}

bool CommandRing::wait(uint32_t frameIndex) {
    if (frameIndex >= kFramesInFlight || device_ == VK_NULL_HANDLE) {
        return false;
    }
    return vkWaitForFences(device_, 1, &fences_[frameIndex], VK_TRUE, UINT64_MAX) == VK_SUCCESS;
}

} // namespace recorz::gpu
