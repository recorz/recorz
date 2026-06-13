#pragma once

#include "gpu/command_ring.h"
#include "gpu/cube_mesh.h"
#include "gpu/cube_pipeline.h"
#include "gpu/dynamic_renderer.h"
#include "gpu/stereo_swapchain_images.h"
#include "gpu/vk_context.h"
#include "render/frame_packet.h"
#include "xr/xr_space.h"
#include "xr/xr_swapchain_group.h"

#include <openxr/openxr.h>

#include <array>
#include <cstdint>
#include <string>

namespace recorz::render {

class StereoRenderer {
public:
    bool init(gpu::VkContext& vk, const xr::XrSwapchainGroup& xrSwapchains, const std::string& shaderDir);

    bool renderFrame(
        const FramePacket& packet,
        xr::XrSwapchainGroup& xrSwapchains,
        xr::XrSpace& referenceSpace,
        gpu::StereoSwapchainImages& swapchainImages,
        gpu::CommandRing& commandRing,
        XrCompositionLayerProjection& outLayer,
        std::array<XrCompositionLayerProjectionView, gpu::StereoSwapchainImages::kMaxEyes>& outViews);

    void shutdown(gpu::VkContext& vk);

private:
    gpu::DynamicRenderer renderer_;
    gpu::CubeMesh cubeMesh_;
    gpu::CubePipeline cubePipeline_;
    gpu::VkContext* vk_ = nullptr;
    bool initialized_ = false;
};

} // namespace recorz::render
