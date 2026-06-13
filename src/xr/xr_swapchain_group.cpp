#include "xr/xr_swapchain_group.h"

#include "xr/xr_common.h"

#include <algorithm>
#include <iostream>
#include <vector>

namespace recorz::xr {
namespace {

int64_t selectSwapchainFormat(XrSession session) {
    uint32_t formatCount = 0;
    if (xrEnumerateSwapchainFormats(session, 0, &formatCount, nullptr) != XR_SUCCESS || formatCount == 0) {
        return static_cast<int64_t>(VK_FORMAT_R8G8B8A8_SRGB);
    }

    std::vector<int64_t> formats(formatCount);
    if (xrEnumerateSwapchainFormats(session, formatCount, &formatCount, formats.data()) != XR_SUCCESS) {
        return static_cast<int64_t>(VK_FORMAT_R8G8B8A8_SRGB);
    }

    const int64_t preferred[] = {
        static_cast<int64_t>(VK_FORMAT_R8G8B8A8_SRGB),
        static_cast<int64_t>(VK_FORMAT_B8G8R8A8_SRGB),
        static_cast<int64_t>(VK_FORMAT_R8G8B8A8_UNORM),
        static_cast<int64_t>(VK_FORMAT_B8G8R8A8_UNORM),
    };

    for (int64_t candidate : preferred) {
        if (std::find(formats.begin(), formats.end(), candidate) != formats.end()) {
            return candidate;
        }
    }

    return formats[0];
}

} // namespace

XrSwapchainGroup::~XrSwapchainGroup() {
    if (eyeCount_ > 0) {
        std::cerr << "XrSwapchainGroup destroyed without calling destroy().\n";
    }
}

bool XrSwapchainGroup::create(XrSession session, const StereoViewConfiguration& viewConfiguration) {
    destroy();

    if (!viewConfiguration.isValid()) {
        std::cerr << "Invalid stereo view configuration for swapchains.\n";
        return false;
    }

    format_ = selectSwapchainFormat(session);
    eyeCount_ = viewConfiguration.count;

    for (uint32_t i = 0; i < eyeCount_; ++i) {
        const uint32_t width = viewConfiguration.views[i].recommendedImageRectWidth;
        const uint32_t height = viewConfiguration.views[i].recommendedImageRectHeight;
        const uint32_t sampleCount = viewConfiguration.views[i].recommendedSwapchainSampleCount;

        if (!swapchains_[i].create(session, format_, width, height, sampleCount)) {
            destroy();
            return false;
        }

        std::cout << "Eye " << i << " swapchain: " << width << "x" << height
                  << " (" << swapchains_[i].getImages().size() << " images)\n";
    }

    return true;
}

void XrSwapchainGroup::destroy() {
    for (uint32_t i = 0; i < eyeCount_; ++i) {
        swapchains_[i].destroy();
    }
    eyeCount_ = 0;
    format_ = 0;
}

XrSwapchain& XrSwapchainGroup::eye(uint32_t index) {
    return swapchains_[index];
}

const XrSwapchain& XrSwapchainGroup::eye(uint32_t index) const {
    return swapchains_[index];
}

} // namespace recorz::xr
