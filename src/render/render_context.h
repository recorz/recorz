#pragma once

#include "gpu/command_ring.h"
#include "gpu/stereo_swapchain_images.h"
#include "gpu/vk_context.h"
#include "render/frame_packet.h"
#include "render/stereo_renderer.h"

#include <openxr/openxr.h>

#include <array>
#include <cstdint>

namespace recorz::xr {
class XrViewResources;
}

namespace recorz::render {

// GPU rendering resources: command ring, swapchain image views, stereo renderer.
class RenderContext {
public:
    bool isReady() const;
    bool create(gpu::VkContext& vk, const xr::XrSwapchainGroup& xrSwapchains);
    void destroy(gpu::VkContext& vk);

    bool renderFrame(
        const FramePacket& packet,
        xr::XrViewResources& viewResources,
        XrCompositionLayerProjection& outLayer,
        std::array<XrCompositionLayerProjectionView, gpu::StereoSwapchainImages::kMaxEyes>& outViews);

private:
    gpu::CommandRing commandRing_;
    gpu::StereoSwapchainImages swapchainImages_;
    StereoRenderer stereoRenderer_;
    gpu::VkContext* vk_ = nullptr;
    bool ready_ = false;
};

} // namespace recorz::render
