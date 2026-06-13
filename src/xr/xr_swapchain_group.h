#pragma once

#include "xr/xr_swapchain.h"
#include "xr/xr_view_configuration.h"

#include <openxr/openxr.h>

#include <cstdint>

namespace recorz::xr {

class XrSwapchainGroup {
public:
    static constexpr uint32_t kMaxEyes = 2;

    XrSwapchainGroup() = default;
    ~XrSwapchainGroup();

    XrSwapchainGroup(const XrSwapchainGroup&) = delete;
    XrSwapchainGroup& operator=(const XrSwapchainGroup&) = delete;

    bool create(XrSession session, const StereoViewConfiguration& viewConfiguration);
    void destroy();

    uint32_t eyeCount() const { return eyeCount_; }
    int64_t format() const { return format_; }

    XrSwapchain& eye(uint32_t index);
    const XrSwapchain& eye(uint32_t index) const;

private:
    uint32_t eyeCount_ = 0;
    int64_t format_ = 0;
    XrSwapchain swapchains_[kMaxEyes]{};
};

} // namespace recorz::xr
