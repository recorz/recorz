#pragma once

#include "xr/xr_view_configuration.h"

#include <openxr/openxr.h>

#include <cstdint>

namespace recorz::xr {

struct ViewState {
    static constexpr uint32_t kMaxViews = 2;

    uint32_t viewCount = 0;
    XrView views[kMaxViews]{};
    XrViewConfigurationView configViews[kMaxViews]{};
    XrViewState flags{};
};

class XrSpace {
public:
    XrSpace() = default;
    ~XrSpace();

    XrSpace(const XrSpace&) = delete;
    XrSpace& operator=(const XrSpace&) = delete;

    bool create(
        XrSession session,
        const StereoViewConfiguration& viewConfiguration,
        XrReferenceSpaceType type = XR_REFERENCE_SPACE_TYPE_LOCAL);

    void destroy();

    bool locateViews(XrSession session, XrTime displayTime, ViewState& out) const;

    ::XrSpace handle() const { return space_; }

private:
    ::XrSpace space_ = XR_NULL_HANDLE;
    XrSession session_ = XR_NULL_HANDLE;
    XrViewConfigurationType viewConfigurationType_ = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    uint32_t viewCapacity_ = 0;
    XrViewConfigurationView cachedConfigViews_[ViewState::kMaxViews]{};
};

} // namespace recorz::xr
