#pragma once

#include "app/vr_frame_loop.h"
#include "app/vr_session_resources.h"
#include "platform/graphics_bootstrap.h"
#include "xr/xr_frame.h"
#include "xr/xr_session.h"

#include <string>

namespace recorz::app {

struct VrAppConfig {
    std::string applicationName = "Recorz Minimal";
    int maxRenderedFrames = 3000;
    int framesToLog = 5;
};

class VrApplication {
public:
    VrApplication() = default;
    ~VrApplication();

    VrApplication(const VrApplication&) = delete;
    VrApplication& operator=(const VrApplication&) = delete;

    bool init(const VrAppConfig& config = {});
    int run();
    void shutdown();

private:
    VrAppConfig config_;
    platform::GraphicsBootstrap bootstrap_;
    xr::XrSessionRuntime session_;
    xr::XrFrame framePacer_;
    VrSessionResources resources_;
    bool initialized_ = false;
};

} // namespace recorz::app
