#include "xr/xr_space.h"

#include "xr/xr_common.h"

#include <iostream>

namespace recorz::xr {

XrSpace::~XrSpace() {
    destroy();
}

bool XrSpace::create(
    XrSession session,
    const StereoViewConfiguration& viewConfiguration,
    XrReferenceSpaceType type) {
    destroy();

    if (!viewConfiguration.isValid()) {
        std::cerr << "Invalid stereo view configuration.\n";
        return false;
    }

    session_ = session;
    viewConfigurationType_ = viewConfiguration.viewConfigurationType;
    viewCapacity_ = viewConfiguration.count;
    for (uint32_t i = 0; i < viewCapacity_; ++i) {
        cachedConfigViews_[i] = viewConfiguration.views[i];
    }

    XrReferenceSpaceCreateInfo createInfo{};
    createInfo.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
    createInfo.referenceSpaceType = type;
    createInfo.poseInReferenceSpace = {{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}};

    if (!checkXr(xrCreateReferenceSpace(session_, &createInfo, &space_), "xrCreateReferenceSpace")) {
        session_ = XR_NULL_HANDLE;
        viewCapacity_ = 0;
        return false;
    }

    std::cout << "Reference space created (" << viewCapacity_ << " views).\n";
    return true;
}

void XrSpace::destroy() {
    if (space_ != XR_NULL_HANDLE) {
        xrDestroySpace(space_);
        space_ = XR_NULL_HANDLE;
    }
    session_ = XR_NULL_HANDLE;
    viewCapacity_ = 0;
}

bool XrSpace::locateViews(XrSession session, XrTime displayTime, ViewState& out) const {
    if (space_ == XR_NULL_HANDLE || viewCapacity_ == 0) {
        return false;
    }

    XrViewLocateInfo locateInfo{};
    locateInfo.type = XR_TYPE_VIEW_LOCATE_INFO;
    locateInfo.viewConfigurationType = viewConfigurationType_;
    locateInfo.displayTime = displayTime;
    locateInfo.space = space_;

    XrViewState viewState{XR_TYPE_VIEW_STATE};
    XrView views[ViewState::kMaxViews]{};
    for (uint32_t i = 0; i < viewCapacity_; ++i) {
        views[i].type = XR_TYPE_VIEW;
    }

    uint32_t locatedCount = 0;
    if (!checkXr(
            xrLocateViews(session, &locateInfo, &viewState, viewCapacity_, &locatedCount, views),
            "xrLocateViews")) {
        return false;
    }

    out.viewCount = locatedCount;
    out.flags = viewState;
    for (uint32_t i = 0; i < locatedCount; ++i) {
        out.views[i] = views[i];
        out.configViews[i] = cachedConfigViews_[i];
    }

    return true;
}

} // namespace recorz::xr
