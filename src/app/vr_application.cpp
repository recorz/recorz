#include "app/vr_application.h"

#include <chrono>
#include <iostream>

namespace recorz::app {

VrApplication::~VrApplication() {
    shutdown();
}

bool VrApplication::init(const VrAppConfig& config) {
    if (initialized_) {
        return true;
    }

    config_ = config;

    platform::BootstrapInitInfo bootstrapInfo;
    bootstrapInfo.applicationName = config_.applicationName;

    if (!bootstrap_.init(bootstrapInfo)) {
        std::cerr << "Graphics initialization failed.\n";
        return false;
    }

    initialized_ = true;
    return true;
}

int VrApplication::run() {
    if (!initialized_) {
        std::cerr << "VrApplication::run called before init.\n";
        return 1;
    }

    FrameLoopConfig loopConfig{
        .maxRenderedFrames = config_.maxRenderedFrames,
        .framesToLog = config_.framesToLog,
    };

    VrFrameLoop frameLoop(bootstrap_, session_, framePacer_, resources_, loopConfig);

    if (!frameLoop.waitForSessionBegin(std::chrono::seconds(30))) {
        std::cout << "Note: session stayed IDLE (no active OpenXR runtime / headset). "
                  << "Frame loop skipped — start SteamVR or Meta Link and retry.\n";
        return 0;
    }

    while (!session_.exitRequested() && frameLoop.renderedFrames() < config_.maxRenderedFrames) {
        const TickResult result = frameLoop.tick();
        if (result.status == TickStatus::Error) {
            return 1;
        }
        if (result.status == TickStatus::Exit) {
            break;
        }
    }

    std::cout << "\n=== Frame loop finished ===\n";
    std::cout << "Rendered " << frameLoop.renderedFrames() << " frame(s) in "
              << frameLoop.iterations() << " iteration(s).\n";
    return 0;
}

void VrApplication::shutdown() {
    if (!initialized_) {
        return;
    }

    if (resources_.isReady()) {
        resources_.destroy(bootstrap_.vk());
    }

    bootstrap_.shutdown();
    initialized_ = false;
}

} // namespace recorz::app
