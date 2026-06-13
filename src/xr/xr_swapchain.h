#pragma once

#include <vulkan/vulkan.h>
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <vector>

namespace recorz::xr {

class XrSwapchain {
public:
    XrSwapchain() = default;
    ~XrSwapchain();

    bool create(XrSession session, int64_t format, uint32_t width, uint32_t height, uint32_t sampleCount = 1);
    void destroy();

    ::XrSwapchain getHandle() const { return handle_; }
    const std::vector<XrSwapchainImageVulkanKHR>& getImages() const { return images_; }
    uint32_t width() const { return width_; }
    uint32_t height() const { return height_; }

    bool acquireImage(uint32_t* imageIndex);
    bool waitImage(XrDuration timeout = XR_INFINITE_DURATION);
    bool releaseImage();

private:
    ::XrSwapchain handle_ = XR_NULL_HANDLE;
    std::vector<XrSwapchainImageVulkanKHR> images_;
    XrSession session_ = XR_NULL_HANDLE;
    uint32_t width_ = 0;
    uint32_t height_ = 0;
};

} // namespace recorz::xr
