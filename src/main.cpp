#include "platform/graphics_context.h"
#include "render/frame_packet.h"
#include "render/stereo_renderer.h"
#include "xr/xr_frame.h"
#include "xr/xr_session.h"
#include "xr/xr_space.h"
#include "xr/xr_swapchain_group.h"

#include <array>
#include <chrono>
#include <iostream>
#include <thread>

namespace {

constexpr int kMaxRenderedFrames = 3000;
constexpr int kFramesToLog = 5;
constexpr auto kReadyPollTimeout = std::chrono::seconds(30);

bool ensureVrResourcesReady(
    XrSession session,
    recorz::gpu::VkContext& vk,
    const recorz::xr::StereoViewConfiguration& viewConfiguration,
    recorz::xr::XrSpace& referenceSpace,
    recorz::xr::XrSwapchainGroup& swapchains,
    recorz::gpu::CommandRing& commandRing,
    recorz::render::StereoRenderer& stereoRenderer) {
    if (referenceSpace.handle() != XR_NULL_HANDLE) {
        return true;
    }

    std::cout << "Initializing VR resources (reference space, swapchains, renderer)...\n";

    if (!referenceSpace.create(session, viewConfiguration, XR_REFERENCE_SPACE_TYPE_LOCAL)) {
        std::cerr << "Failed to create reference space.\n";
        return false;
    }

    if (!swapchains.create(session, vk.device(), viewConfiguration)) {
        std::cerr << "Failed to create stereo swapchains.\n";
        return false;
    }

    if (!commandRing.create(vk.device(), vk.graphicsQueueFamily())) {
        std::cerr << "Failed to create command ring.\n";
        return false;
    }

    if (!stereoRenderer.init(vk, swapchains)) {
        std::cerr << "Failed to initialize stereo renderer.\n";
        return false;
    }

    std::cout << "VR resources ready. Starting render loop.\n";
    return true;
}

} // namespace

int main() {
    std::cout << "=== Recorz Minimal: VR Clear Color Loop ===\n\n";

    recorz::platform::GraphicsContext graphics;
    recorz::platform::GraphicsInitInfo initInfo;
    initInfo.applicationName = "Recorz Minimal";

    if (!graphics.init(initInfo)) {
        std::cerr << "Graphics initialization failed.\n";
        return 1;
    }

    recorz::xr::XrSessionRuntime sessionRuntime;
    recorz::xr::XrFrame framePacer;
    recorz::xr::XrSpace referenceSpace;
    recorz::xr::XrSwapchainGroup swapchains;
    recorz::gpu::CommandRing commandRing;
    recorz::render::StereoRenderer stereoRenderer;

    const XrSession session = graphics.xr().session();
    const XrInstance instance = graphics.xr().instance();
    recorz::gpu::VkContext& vk = graphics.vk();
    const recorz::xr::StereoViewConfiguration& viewConfiguration = graphics.xr().stereoViews();
    const XrViewConfigurationType primaryViewConfiguration = viewConfiguration.viewConfigurationType;

    std::cout << "Waiting for session to become ready...\n";

    const auto readyDeadline = std::chrono::steady_clock::now() + kReadyPollTimeout;
    while (std::chrono::steady_clock::now() < readyDeadline && !sessionRuntime.exitRequested()) {
        sessionRuntime.pollAndUpdate(instance);
        if (sessionRuntime.beginIfReady(instance, session, primaryViewConfiguration)) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    if (!sessionRuntime.isSessionBegun()) {
        std::cout << "Note: session stayed IDLE (no active OpenXR runtime / headset). "
                  << "Frame loop skipped — start SteamVR or Meta Link and retry.\n";
        return 0;
    }

    int renderedFrames = 0;
    int iterations = 0;

    while (!sessionRuntime.exitRequested() && renderedFrames < kMaxRenderedFrames) {
        ++iterations;

        if (!sessionRuntime.pollAndUpdate(instance)) {
            break;
        }

        if (!sessionRuntime.isSessionBegun()) {
            if (!sessionRuntime.beginIfReady(instance, session, primaryViewConfiguration)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
                continue;
            }
        }

        if (!ensureVrResourcesReady(
                session,
                vk,
                viewConfiguration,
                referenceSpace,
                swapchains,
                commandRing,
                stereoRenderer)) {
            return 1;
        }

        const recorz::xr::FrameWaitResult frame = framePacer.wait(session);
        if (!frame.valid) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        if (!framePacer.begin(session)) {
            break;
        }

        if (frame.shouldRender) {
            recorz::xr::ViewState views{};
            if (!referenceSpace.locateViews(session, frame.state.predictedDisplayTime, views)) {
                std::cerr << "Failed to locate views.\n";
                break;
            }

            const recorz::render::FramePacket packet =
                recorz::render::fromXrViews(frame.state.predictedDisplayTime, views);

            XrCompositionLayerProjection projectionLayer{};
            std::array<XrCompositionLayerProjectionView, recorz::xr::XrSwapchainGroup::kMaxEyes> projectionViews{};

            if (!stereoRenderer.renderFrame(
                    packet,
                    swapchains,
                    referenceSpace,
                    commandRing,
                    projectionLayer,
                    projectionViews)) {
                std::cerr << "Failed to render frame.\n";
                break;
            }

            const XrCompositionLayerBaseHeader* layers[] = {
                reinterpret_cast<const XrCompositionLayerBaseHeader*>(&projectionLayer),
            };

            if (!framePacer.end(session, layers)) {
                break;
            }

            if (renderedFrames < kFramesToLog) {
                std::cout << "Rendered frame " << renderedFrames
                          << " | displayTime=" << packet.displayTime
                          << " | views=" << packet.viewCount << "\n";
            }
            ++renderedFrames;
        } else if (!framePacer.end(session)) {
            break;
        }
    }

    stereoRenderer.shutdown(vk);
    commandRing.destroy(vk.device());
    swapchains.destroy(vk.device());

    std::cout << "\n=== Frame loop finished ===\n";
    std::cout << "Rendered " << renderedFrames << " frame(s) in " << iterations << " iteration(s).\n";
    return 0;
}
