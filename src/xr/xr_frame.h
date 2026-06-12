#pragma once

#include <openxr/openxr.h>

#include <cstdint>
#include <span>

namespace recorz::xr {

struct FrameWaitResult {
    XrFrameState state{};
    bool shouldRender = false;
    bool valid = false;
};

class XrFrame {
public:
    FrameWaitResult wait(XrSession session);
    bool begin(XrSession session);
    bool end(XrSession session, std::span<const XrCompositionLayerBaseHeader* const> layers = {});

private:
    XrTime displayTime_ = 0;
};

} // namespace recorz::xr
