#pragma once

#include "xr/xr_space.h"

#include <openxr/openxr.h>

#include <cstdint>

namespace recorz::render {

struct ViewData {
    XrPosef pose{};
    XrFovf fov{};
    uint32_t width = 0;
    uint32_t height = 0;
};

struct FramePacket {
    static constexpr uint32_t kMaxViews = 2;

    XrTime displayTime = 0;
    uint32_t viewCount = 0;
    ViewData views[kMaxViews]{};
};

inline FramePacket fromXrViews(XrTime displayTime, const recorz::xr::ViewState& viewState) {
    FramePacket packet{};
    packet.displayTime = displayTime;
    packet.viewCount = viewState.viewCount;

    for (uint32_t i = 0; i < viewState.viewCount; ++i) {
        packet.views[i].pose = viewState.views[i].pose;
        packet.views[i].fov = viewState.views[i].fov;
        packet.views[i].width = viewState.configViews[i].recommendedImageRectWidth;
        packet.views[i].height = viewState.configViews[i].recommendedImageRectHeight;
    }

    return packet;
}

} // namespace recorz::render
