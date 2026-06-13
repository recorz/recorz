#pragma once

#include "xr/xr_space.h"
#include "xr/xr_swapchain_group.h"
#include "xr/xr_view_configuration.h"

#include <openxr/openxr.h>

namespace recorz::xr {

// OpenXR view resources created after xrBeginSession: reference space + stereo swapchains.
class XrViewResources {
public:
    bool isReady() const;
    bool create(XrSession session, const StereoViewConfiguration& viewConfiguration);
    void destroy();

    XrSpace& referenceSpace() { return referenceSpace_; }
    const XrSpace& referenceSpace() const { return referenceSpace_; }

    XrSwapchainGroup& swapchains() { return swapchains_; }
    const XrSwapchainGroup& swapchains() const { return swapchains_; }

private:
    XrSpace referenceSpace_;
    XrSwapchainGroup swapchains_;
};

} // namespace recorz::xr
