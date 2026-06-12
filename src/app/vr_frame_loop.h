#pragma once

#include "app/vr_session_resources.h"
#include "platform/graphics_bootstrap.h"
#include "xr/xr_frame.h"
#include "xr/xr_session.h"

#include <chrono>
#include <cstdint>

namespace recorz::app {

enum class TickStatus {
    Continue,
    Rendered,
    Exit,
    Error,
};

struct TickResult {
    TickStatus status = TickStatus::Continue;
};

struct FrameLoopConfig {
    int maxRenderedFrames = 3000;
    int framesToLog = 5;
};

class VrFrameLoop {
public:
    VrFrameLoop(
        platform::GraphicsBootstrap& bootstrap,
        xr::XrSessionRuntime& session,
        xr::XrFrame& framePacer,
        VrSessionResources& resources,
        FrameLoopConfig config = {});

    bool waitForSessionBegin(std::chrono::seconds timeout = std::chrono::seconds(30));
    TickResult tick();

    int renderedFrames() const { return renderedFrames_; }
    int iterations() const { return iterations_; }

private:
    bool ensureResourcesReady();

    platform::GraphicsBootstrap& bootstrap_;
    xr::XrSessionRuntime& session_;
    xr::XrFrame& framePacer_;
    VrSessionResources& resources_;
    FrameLoopConfig config_;

    int renderedFrames_ = 0;
    int iterations_ = 0;
};

} // namespace recorz::app
