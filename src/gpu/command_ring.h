#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>

namespace recorz::gpu {

class CommandRing {
public:
    static constexpr uint32_t kFramesInFlight = 2;

    CommandRing() = default;
    ~CommandRing();

    CommandRing(const CommandRing&) = delete;
    CommandRing& operator=(const CommandRing&) = delete;

    bool create(VkDevice device, uint32_t queueFamilyIndex);
    void destroy(VkDevice device);

    VkCommandBuffer begin(uint32_t frameIndex);
    bool submit(VkQueue queue, uint32_t frameIndex);
    bool wait(uint32_t frameIndex);

    uint32_t currentFrame() const { return currentFrame_; }
    void advance() { currentFrame_ = (currentFrame_ + 1) % kFramesInFlight; }

private:
    VkDevice device_ = VK_NULL_HANDLE;
    VkCommandPool pool_ = VK_NULL_HANDLE;
    VkCommandBuffer buffers_[kFramesInFlight]{};
    VkFence fences_[kFramesInFlight]{};
    uint32_t currentFrame_ = 0;
};

} // namespace recorz::gpu
