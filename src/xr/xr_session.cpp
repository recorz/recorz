#include "xr/xr_session.h"

#include "xr/xr_common.h"

#include <chrono>
#include <thread>

namespace recorz::xr {
namespace {

SessionState toSessionState(XrSessionState state) {
    switch (state) {
    case XR_SESSION_STATE_IDLE: return SessionState::Idle;
    case XR_SESSION_STATE_READY: return SessionState::Ready;
    case XR_SESSION_STATE_SYNCHRONIZED: return SessionState::Synchronized;
    case XR_SESSION_STATE_VISIBLE: return SessionState::Visible;
    case XR_SESSION_STATE_FOCUSED: return SessionState::Focused;
    case XR_SESSION_STATE_STOPPING: return SessionState::Stopping;
    case XR_SESSION_STATE_LOSS_PENDING: return SessionState::LossPending;
    case XR_SESSION_STATE_EXITING: return SessionState::Exiting;
    default: return SessionState::Unknown;
    }
}

const char* sessionStateName(SessionState state) {
    switch (state) {
    case SessionState::Idle: return "Idle";
    case SessionState::Ready: return "Ready";
    case SessionState::Synchronized: return "Synchronized";
    case SessionState::Visible: return "Visible";
    case SessionState::Focused: return "Focused";
    case SessionState::Stopping: return "Stopping";
    case SessionState::LossPending: return "LossPending";
    case SessionState::Exiting: return "Exiting";
    default: return "Unknown";
    }
}

} // namespace

bool XrSessionRuntime::pollAndUpdate(XrInstance instance) {
    XrEventDataBuffer event{XR_TYPE_EVENT_DATA_BUFFER};

    while (xrPollEvent(instance, &event) == XR_SUCCESS) {
        switch (event.type) {
        case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
            const auto& stateEvent = *reinterpret_cast<const XrEventDataSessionStateChanged*>(&event);
            state_ = toSessionState(stateEvent.state);
            std::cout << "Session state: " << sessionStateName(state_) << "\n";

            if (state_ == SessionState::Exiting) {
                exitRequested_ = true;
            }
            if (state_ == SessionState::LossPending) {
                sessionBegun_ = false;
            }
            break;
        }
        case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
            exitRequested_ = true;
            break;
        default:
            break;
        }

        event = {XR_TYPE_EVENT_DATA_BUFFER};
    }

    return !exitRequested_;
}

bool XrSessionRuntime::beginIfReady(
    XrInstance instance,
    XrSession session,
    XrViewConfigurationType primaryViewConfiguration) {
    if (sessionBegun_) {
        return true;
    }
    if (state_ != SessionState::Ready) {
        return false;
    }

    XrSessionBeginInfo beginInfo{};
    beginInfo.type = XR_TYPE_SESSION_BEGIN_INFO;
    beginInfo.primaryViewConfigurationType = primaryViewConfiguration;

    if (!checkXr(xrBeginSession(session, &beginInfo), "xrBeginSession")) {
        return false;
    }

    sessionBegun_ = true;
    std::cout << "OpenXR session running (stereo).\n";

    // xrBeginSession queues a state-change event; drain it before the frame loop.
    for (int attempt = 0; attempt < 500 && !canPaceFrames(); ++attempt) {
        pollAndUpdate(instance);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return true;
}

bool XrSessionRuntime::isSessionRunning() const {
    return sessionBegun_ && (state_ == SessionState::Synchronized ||
                              state_ == SessionState::Visible ||
                              state_ == SessionState::Focused);
}

bool XrSessionRuntime::canPaceFrames() const {
    return sessionBegun_ && (state_ == SessionState::Synchronized ||
                             state_ == SessionState::Visible ||
                             state_ == SessionState::Focused);
}

} // namespace recorz::xr
