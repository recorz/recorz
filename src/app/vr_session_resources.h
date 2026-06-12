#pragma once

#include "gpu/command_ring.h"
#include "gpu/vk_context.h"
#include "render/stereo_renderer.h"
#include "xr/xr_space.h"
#include "xr/xr_swapchain_group.h"
#include "xr/xr_view_configuration.h"

#include <openxr/openxr.h>

namespace recorz::app {

struct VrSessionContext {
    XrSession session = XR_NULL_HANDLE;
    gpu::VkContext* vk = nullptr;
    const xr::StereoViewConfiguration* viewConfiguration = nullptr;
};

// Resources created after xrBeginSession; torn down before session destroy.
class VrSessionResources {
public:
    bool isReady() const;
    bool create(const VrSessionContext& ctx);
    void destroy(gpu::VkContext& vk);

    xr::XrSpace& referenceSpace() { return referenceSpace_; }
    const xr::XrSpace& referenceSpace() const { return referenceSpace_; }

    xr::XrSwapchainGroup& swapchains() { return swapchains_; }
    const xr::XrSwapchainGroup& swapchains() const { return swapchains_; }

    gpu::CommandRing& commandRing() { return commandRing_; }
    render::StereoRenderer& renderer() { return stereoRenderer_; }

private:
    xr::XrSpace referenceSpace_;
    xr::XrSwapchainGroup swapchains_;
    gpu::CommandRing commandRing_;
    render::StereoRenderer stereoRenderer_;
};

} // namespace recorz::app
