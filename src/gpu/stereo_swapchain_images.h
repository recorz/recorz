#pragma once

#include "gpu/vk_swapchain_images.h"

#include <vulkan/vulkan.h>

#include <cstdint>

namespace recorz::xr {
class XrSwapchainGroup;
}

namespace recorz::gpu {

// Vulkan image views for stereo OpenXR swapchain images (imported from xr swapchains).
class StereoSwapchainImages {
public:
    static constexpr uint32_t kMaxEyes = 2;

    StereoSwapchainImages() = default;
    ~StereoSwapchainImages();

    StereoSwapchainImages(const StereoSwapchainImages&) = delete;
    StereoSwapchainImages& operator=(const StereoSwapchainImages&) = delete;

    bool create(VkDevice device, const xr::XrSwapchainGroup& xrSwapchains);
    void destroy(VkDevice device);

    uint32_t eyeCount() const { return eyeCount_; }
    VkSwapchainImages& eye(uint32_t index);
    const VkSwapchainImages& eye(uint32_t index) const;

private:
    uint32_t eyeCount_ = 0;
    VkSwapchainImages images_[kMaxEyes]{};
};

} // namespace recorz::gpu
