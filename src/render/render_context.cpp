#include "render/render_context.h"

#include "xr/xr_view_resources.h"

#include <iostream>

namespace recorz::render {

bool RenderContext::isReady() const {
    return ready_;
}

bool RenderContext::create(gpu::VkContext& vk, const xr::XrSwapchainGroup& xrSwapchains) {
    if (isReady()) {
        return true;
    }

    if (!vk.isInitialized() || xrSwapchains.eyeCount() == 0) {
        std::cerr << "Vulkan or XR swapchains not ready for render context creation.\n";
        return false;
    }

    std::cout << "Initializing render context (command ring, swapchain images, renderer)...\n";

    if (!commandRing_.create(vk.device(), vk.graphicsQueueFamily())) {
        std::cerr << "Failed to create command ring.\n";
        return false;
    }

    if (!swapchainImages_.create(vk.device(), xrSwapchains)) {
        std::cerr << "Failed to create stereo swapchain images.\n";
        commandRing_.destroy(vk.device());
        return false;
    }

    if (!stereoRenderer_.init(vk)) {
        std::cerr << "Failed to initialize stereo renderer.\n";
        swapchainImages_.destroy(vk.device());
        commandRing_.destroy(vk.device());
        return false;
    }

    vk_ = &vk;
    ready_ = true;
    std::cout << "Render context ready.\n";
    return true;
}

void RenderContext::destroy(gpu::VkContext& vk) {
    if (!isReady()) {
        return;
    }

    stereoRenderer_.shutdown(vk);
    swapchainImages_.destroy(vk.device());
    commandRing_.destroy(vk.device());
    vk_ = nullptr;
    ready_ = false;
}

bool RenderContext::renderFrame(
    const FramePacket& packet,
    xr::XrViewResources& viewResources,
    XrCompositionLayerProjection& outLayer,
    std::array<XrCompositionLayerProjectionView, gpu::StereoSwapchainImages::kMaxEyes>& outViews) {
    if (!isReady() || vk_ == nullptr) {
        return false;
    }

    return stereoRenderer_.renderFrame(
        packet,
        viewResources.swapchains(),
        viewResources.referenceSpace(),
        swapchainImages_,
        commandRing_,
        outLayer,
        outViews);
}

} // namespace recorz::render
