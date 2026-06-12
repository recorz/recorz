#pragma once

#include "gpu/command_ring.h"
#include "gpu/dynamic_renderer.h"
#include "gpu/vk_context.h"
#include "render/frame_packet.h"
#include "xr/xr_space.h"
#include "xr/xr_swapchain_group.h"

#include <openxr/openxr.h>

#include <array>
#include <cstdint>

namespace recorz::render {

class StereoRenderer {
public:
    bool init(gpu::VkContext& vk, xr::XrSwapchainGroup& swapchains);

    bool renderFrame(
        const FramePacket& packet,
        xr::XrSwapchainGroup& swapchains,
        xr::XrSpace& referenceSpace,
        gpu::CommandRing& commandRing,
        XrCompositionLayerProjection& outLayer,
        std::array<XrCompositionLayerProjectionView, xr::XrSwapchainGroup::kMaxEyes>& outViews);

    void shutdown(gpu::VkContext& vk);

private:
    gpu::DynamicRenderer renderer_;
    gpu::VkContext* vk_ = nullptr;
    bool initialized_ = false;
};

} // namespace recorz::render
