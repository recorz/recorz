#include "app/vr_frame_loop.h"

#include "render/frame_packet.h"

#include <array>
#include <iostream>
#include <thread>

namespace recorz::app {

VrFrameLoop::VrFrameLoop(
    platform::GraphicsBootstrap& bootstrap,
    xr::XrSessionRuntime& session,
    xr::XrFrame& framePacer,
    VrSessionResources& resources,
    FrameLoopConfig config)
    : bootstrap_(bootstrap)
    , session_(session)
    , framePacer_(framePacer)
    , resources_(resources)
    , config_(config) {}

bool VrFrameLoop::waitForSessionBegin(std::chrono::seconds timeout) {
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

bool VrFrameLoop::ensureResourcesReady() {
    VrSessionContext ctx{
        .session = bootstrap_.xr().session(),
        .vk = &bootstrap_.vk(),
        .viewConfiguration = &bootstrap_.xr().stereoViews(),
    };
    return resources_.create(ctx);
}

TickResult VrFrameLoop::tick() {
    ++iterations_;

    const XrInstance instance = bootstrap_.xr().instance();
    const XrSession session = bootstrap_.xr().session();
    const XrViewConfigurationType primaryViewConfiguration =
        bootstrap_.xr().stereoViews().viewConfigurationType;

    if (!session_.pollAndUpdate(instance)) {
        return {.status = TickStatus::Exit};
    }

    if (!session_.isSessionBegun()) {
        if (!session_.beginIfReady(instance, session, primaryViewConfiguration)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
            return {.status = TickStatus::Continue};
        }
    }

    if (!ensureResourcesReady()) {
        return {.status = TickStatus::Error};
    }

    const xr::FrameWaitResult frame = framePacer_.wait(session);
    if (!frame.valid) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        return {.status = TickStatus::Continue};
    }

    if (!framePacer_.begin(session)) {
        return {.status = TickStatus::Exit};
    }

    if (frame.shouldRender) {
        xr::ViewState views{};
        if (!resources_.referenceSpace().locateViews(session, frame.state.predictedDisplayTime, views)) {
            std::cerr << "Failed to locate views.\n";
            return {.status = TickStatus::Exit};
        }

        const render::FramePacket packet =
            render::fromXrViews(frame.state.predictedDisplayTime, views);

        XrCompositionLayerProjection projectionLayer{};
        std::array<XrCompositionLayerProjectionView, xr::XrSwapchainGroup::kMaxEyes> projectionViews{};

        if (!resources_.renderer().renderFrame(
                packet,
                resources_.swapchains(),
                resources_.referenceSpace(),
                resources_.commandRing(),
                projectionLayer,
                projectionViews)) {
            std::cerr << "Failed to render frame.\n";
            return {.status = TickStatus::Exit};
        }

        const XrCompositionLayerBaseHeader* layers[] = {
            reinterpret_cast<const XrCompositionLayerBaseHeader*>(&projectionLayer),
        };

        if (!framePacer_.end(session, layers)) {
            return {.status = TickStatus::Exit};
        }

        if (renderedFrames_ < config_.framesToLog) {
            std::cout << "Rendered frame " << renderedFrames_
                      << " | displayTime=" << packet.displayTime
                      << " | views=" << packet.viewCount << "\n";
        }
        ++renderedFrames_;
        return {.status = TickStatus::Rendered};
    }

    if (!framePacer_.end(session)) {
        return {.status = TickStatus::Exit};
    }

    return {.status = TickStatus::Continue};
}

} // namespace recorz::app
