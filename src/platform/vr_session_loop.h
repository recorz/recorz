#pragma once

#include "platform/graphics_bootstrap.h"
#include "render/render_context.h"
#include "xr/xr_frame.h"
#include "xr/xr_session.h"
#include "xr/xr_view_runtime.h"

#include <chrono>
#include <cstdint>

namespace recorz::platform {

struct VrSessionLoopConfig {
    int maxRenderedFrames = 3000;
    int framesToLog = 5;
    std::chrono::seconds sessionBeginTimeout{30};
};

enum class LoopTickStatus {
    Continue,
    Rendered,
    Exit,
    Error,
};

struct LoopTickResult {
    LoopTickStatus status = LoopTickStatus::Continue;
};

// Runtime orchestrator: session events, compositor pacing, view locate, stereo render.
class VrSessionLoop {
public:
    explicit VrSessionLoop(GraphicsBootstrap& bootstrap);
    ~VrSessionLoop();

    VrSessionLoop(const VrSessionLoop&) = delete;
    VrSessionLoop& operator=(const VrSessionLoop&) = delete;

    int run(const VrSessionLoopConfig& config = {});
    void shutdown();

    int renderedFrames() const { return renderedFrames_; }
    int iterations() const { return iterations_; }

private:
    bool waitForSessionBegin(std::chrono::seconds timeout);
    bool ensureRuntimeReady();
    LoopTickResult tick(const VrSessionLoopConfig& config);

    GraphicsBootstrap& bootstrap_;
    xr::XrSessionRuntime session_;
    xr::XrFrame framePacer_;
    xr::XrViewRuntime viewRuntime_;
    render::RenderContext renderContext_;

    int renderedFrames_ = 0;
    int iterations_ = 0;
};

} // namespace recorz::platform
