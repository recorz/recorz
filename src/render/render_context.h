#pragma once

#include "gpu/command_ring.h"
#include "gpu/vk_context.h"
#include "render/stereo_renderer.h"
#include "xr/xr_swapchain_group.h"

namespace recorz::render {

// GPU rendering resources for stereo VR output (command ring + stereo renderer).
class RenderContext {
public:
    bool isReady() const;
    bool create(gpu::VkContext& vk, xr::XrSwapchainGroup& swapchains);
    void destroy(gpu::VkContext& vk);

    gpu::CommandRing& commandRing() { return commandRing_; }
    StereoRenderer& renderer() { return stereoRenderer_; }

private:
    gpu::CommandRing commandRing_;
    StereoRenderer stereoRenderer_;
    bool ready_ = false;
};

} // namespace recorz::render
