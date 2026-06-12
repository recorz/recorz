#include "xr/xr_frame.h"

#include "xr/xr_common.h"

namespace recorz::xr {

FrameWaitResult XrFrame::wait(XrSession session) {
    FrameWaitResult result{};

    XrFrameWaitInfo waitInfo{};
    waitInfo.type = XR_TYPE_FRAME_WAIT_INFO;

    result.state.type = XR_TYPE_FRAME_STATE;
    const XrResult waitResult = xrWaitFrame(session, &waitInfo, &result.state);
    if (waitResult == XR_ERROR_SESSION_NOT_RUNNING) {
        return result;
    }
    if (!checkXr(waitResult, "xrWaitFrame")) {
        return result;
    }

    result.valid = true;
    result.shouldRender = result.state.shouldRender == XR_TRUE;
    displayTime_ = result.state.predictedDisplayTime;
    return result;
}

bool XrFrame::begin(XrSession session) {
    XrFrameBeginInfo beginInfo{};
    beginInfo.type = XR_TYPE_FRAME_BEGIN_INFO;
    return checkXr(xrBeginFrame(session, &beginInfo), "xrBeginFrame");
}

bool XrFrame::end(XrSession session, std::span<const XrCompositionLayerBaseHeader* const> layers) {
    XrFrameEndInfo endInfo{};
    endInfo.type = XR_TYPE_FRAME_END_INFO;
    endInfo.displayTime = displayTime_;
    endInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    endInfo.layerCount = static_cast<uint32_t>(layers.size());
    endInfo.layers = layers.empty() ? nullptr : layers.data();
    return checkXr(xrEndFrame(session, &endInfo), "xrEndFrame");
}

} // namespace recorz::xr
