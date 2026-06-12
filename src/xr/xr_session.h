#pragma once

#include <openxr/openxr.h>

namespace recorz::xr {

enum class SessionState {
    Unknown,
    Idle,
    Ready,
    Synchronized,
    Visible,
    Focused,
    Stopping,
    LossPending,
    Exiting,
};

class XrSessionRuntime {
public:
    bool pollAndUpdate(XrInstance instance);
    bool beginIfReady(
        XrInstance instance,
        XrSession session,
        XrViewConfigurationType primaryViewConfiguration = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO);

    SessionState state() const { return state_; }
    bool exitRequested() const { return exitRequested_; }
    bool isSessionBegun() const { return sessionBegun_; }
    bool isSessionRunning() const;
    bool canPaceFrames() const;

private:
    SessionState state_ = SessionState::Unknown;
    bool exitRequested_ = false;
    bool sessionBegun_ = false;
};

} // namespace recorz::xr
