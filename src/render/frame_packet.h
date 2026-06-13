#pragma once

#include <openxr/openxr.h>

#include <cstdint>

namespace recorz::xr {
struct ViewState;
}

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

FramePacket fromXrViewState(XrTime displayTime, const xr::ViewState& viewState);

} // namespace recorz::render
