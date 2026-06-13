#include "render/frame_packet.h"

#include "xr/xr_space.h"

namespace recorz::render {

FramePacket fromXrViewState(XrTime displayTime, const xr::ViewState& viewState) {
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
