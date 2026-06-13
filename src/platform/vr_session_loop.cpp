#include "platform/vr_session_loop.h"

#include "render/frame_packet.h"
#include "gpu/stereo_swapchain_images.h"

#include <array>
#include <iostream>
#include <thread>

namespace recorz::platform {

VrSessionLoop::VrSessionLoop(GraphicsBootstrap& bootstrap)
    : bootstrap_(bootstrap) {}

VrSessionLoop::~VrSessionLoop() {
    shutdown();
}

int VrSessionLoop::run(const VrSessionLoopConfig& config) {
    if (!waitForSessionBegin(config.sessionBeginTimeout)) {
        std::cout << "Note: session stayed IDLE (no active OpenXR runtime / headset). "
                  << "Frame loop skipped — start SteamVR or Meta Link and retry.\n";
        return 0;
    }

    while (!session_.exitRequested() && renderedFrames_ < config.maxRenderedFrames) {
        const LoopTickResult result = tick(config);
        if (result.status == LoopTickStatus::Error) {
            return 1;
        }
        if (result.status == LoopTickStatus::Exit) {
            break;
        }
    }

    std::cout << "\n=== Frame loop finished ===\n";
    std::cout << "Rendered " << renderedFrames_ << " frame(s) in "
              << iterations_ << " iteration(s).\n";
    return 0;
}

void VrSessionLoop::shutdown() {
    if (renderContext_.isReady()) {
        renderContext_.destroy(bootstrap_.vk());
    }
    if (viewResources_.isReady()) {
        viewResources_.destroy();
    }
}

bool VrSessionLoop::waitForSessionBegin(std::chrono::seconds timeout) {
    const XrInstance instance = bootstrap_.xr().instance();
    const XrSession session = bootstrap_.xr().session();
    const XrViewConfigurationType primaryViewConfiguration =
        bootstrap_.xr().stereoViews().viewConfigurationType;

    std::cout << "Waiting for session to become ready...\n";

    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline && !session_.exitRequested()) {
        session_.pollAndUpdate(instance);
        if (session_.beginIfReady(instance, session, primaryViewConfiguration)) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return session_.isSessionBegun();
}

bool VrSessionLoop::ensureRuntimeReady() {
    if (!viewResources_.isReady()) {
        if (!viewResources_.create(
                bootstrap_.xr().session(),
                bootstrap_.xr().stereoViews())) {
            return false;
        }
    }

    if (!renderContext_.isReady()) {
        if (!renderContext_.create(bootstrap_.vk(), viewResources_.swapchains())) {
            viewResources_.destroy();
            return false;
        }
    }

    return true;
}

LoopTickResult VrSessionLoop::tick(const VrSessionLoopConfig& config) {
    ++iterations_;

    const XrInstance instance = bootstrap_.xr().instance();
    const XrSession session = bootstrap_.xr().session();
    const XrViewConfigurationType primaryViewConfiguration =
        bootstrap_.xr().stereoViews().viewConfigurationType;

    if (!session_.pollAndUpdate(instance)) {
        return {.status = LoopTickStatus::Exit};
    }

    if (!session_.isSessionBegun()) {
        if (!session_.beginIfReady(instance, session, primaryViewConfiguration)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
            return {.status = LoopTickStatus::Continue};
        }
    }

    if (!ensureRuntimeReady()) {
        std::cerr << "Failed to initialize view or render resources.\n";
        return {.status = LoopTickStatus::Error};
    }

    const xr::FrameWaitResult frame = framePacer_.wait(session);
    if (!frame.valid) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        return {.status = LoopTickStatus::Continue};
    }

    if (!framePacer_.begin(session)) {
        return {.status = LoopTickStatus::Exit};
    }

    if (frame.shouldRender) {
        xr::ViewState views{};
        if (!viewResources_.referenceSpace().locateViews(
                session, frame.state.predictedDisplayTime, views)) {
            std::cerr << "Failed to locate views.\n";
            return {.status = LoopTickStatus::Exit};
        }

        const render::FramePacket packet =
            render::fromXrViewState(frame.state.predictedDisplayTime, views);

        XrCompositionLayerProjection projectionLayer{};
        std::array<XrCompositionLayerProjectionView, gpu::StereoSwapchainImages::kMaxEyes> projectionViews{};

        if (!renderContext_.renderFrame(packet, viewResources_, projectionLayer, projectionViews)) {
            std::cerr << "Failed to render frame.\n";
            return {.status = LoopTickStatus::Exit};
        }

        const XrCompositionLayerBaseHeader* layers[] = {
            reinterpret_cast<const XrCompositionLayerBaseHeader*>(&projectionLayer),
        };

        if (!framePacer_.end(session, layers)) {
            return {.status = LoopTickStatus::Exit};
        }

        if (renderedFrames_ < config.framesToLog) {
            std::cout << "Rendered frame " << renderedFrames_
                      << " | displayTime=" << packet.displayTime
                      << " | views=" << packet.viewCount << "\n";
        }
        ++renderedFrames_;
        return {.status = LoopTickStatus::Rendered};
    }

    if (!framePacer_.end(session)) {
        return {.status = LoopTickStatus::Exit};
    }

    return {.status = LoopTickStatus::Continue};
}

} // namespace recorz::platform
