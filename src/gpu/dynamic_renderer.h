#pragma once

#include "gpu/vk_swapchain_images.h"

#include <vulkan/vulkan.h>

namespace recorz::gpu {

struct ClearColor {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

class DynamicRenderer {
public:
    void clearColor(
        VkCommandBuffer commandBuffer,
        VkSwapchainImages& swapchainImages,
        uint32_t imageIndex,
        const ClearColor& color) const;
};

} // namespace recorz::gpu
