#include "render/stereo_renderer.h"

#include <iostream>

namespace recorz::render {
namespace {

const gpu::ClearColor kEyeColors[xr::XrSwapchainGroup::kMaxEyes] = {
    {0.05f, 0.08f, 0.35f, 1.0f},
    {0.35f, 0.08f, 0.08f, 1.0f},
};

} // namespace

bool StereoRenderer::init(gpu::VkContext& vk, xr::XrSwapchainGroup& swapchains) {
    if (initialized_) {
        return true;
    }

    if (!vk.isInitialized() || swapchains.eyeCount() == 0) {
        std::cerr << "StereoRenderer init: Vulkan or swapchains not ready.\n";
        return false;
    }

    vk_ = &vk;
    initialized_ = true;
    return true;
}

bool StereoRenderer::renderFrame(
    const FramePacket& packet,
    xr::XrSwapchainGroup& swapchains,
    xr::XrSpace& referenceSpace,
    gpu::CommandRing& commandRing,
    XrCompositionLayerProjection& outLayer,
    std::array<XrCompositionLayerProjectionView, xr::XrSwapchainGroup::kMaxEyes>& outViews) {
    if (!initialized_ || vk_ == nullptr) {
        return false;
    }

    const uint32_t frameIndex = commandRing.currentFrame();
    VkCommandBuffer commandBuffer = commandRing.begin(frameIndex);
    if (commandBuffer == VK_NULL_HANDLE) {
        return false;
    }

    uint32_t imageIndices[xr::XrSwapchainGroup::kMaxEyes]{};

    for (uint32_t eye = 0; eye < swapchains.eyeCount(); ++eye) {
        xr::XrSwapchain& xrSwapchain = swapchains.eye(eye);
        if (!xrSwapchain.acquireImage(&imageIndices[eye])) {
            std::cerr << "Failed to acquire swapchain image for eye " << eye << ".\n";
            return false;
        }
        if (!xrSwapchain.waitImage()) {
            std::cerr << "Failed to wait on swapchain image for eye " << eye << ".\n";
            return false;
        }

        gpu::VkSwapchainImages& gpuImages = swapchains.gpuImages(eye);
        renderer_.clearColor(commandBuffer, gpuImages, imageIndices[eye], kEyeColors[eye]);
    }

    if (!commandRing.submit(vk_->graphicsQueue(), frameIndex)) {
        return false;
    }
    if (!commandRing.wait(frameIndex)) {
        return false;
    }

    for (uint32_t eye = 0; eye < swapchains.eyeCount(); ++eye) {
        if (!swapchains.eye(eye).releaseImage()) {
            std::cerr << "Failed to release swapchain image for eye " << eye << ".\n";
            return false;
        }
    }

    for (uint32_t eye = 0; eye < packet.viewCount; ++eye) {
        XrCompositionLayerProjectionView& view = outViews[eye];
        view = {};
        view.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
        view.pose = packet.views[eye].pose;
        view.fov = packet.views[eye].fov;
        view.subImage.swapchain = swapchains.eye(eye).getHandle();
        view.subImage.imageRect.offset = {0, 0};
        view.subImage.imageRect.extent = {
            static_cast<int32_t>(packet.views[eye].width),
            static_cast<int32_t>(packet.views[eye].height),
        };
        view.subImage.imageArrayIndex = 0;
    }

    outLayer = {};
    outLayer.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION;
    outLayer.space = referenceSpace.handle();
    outLayer.viewCount = packet.viewCount;
    outLayer.views = outViews.data();

    commandRing.advance();
    return true;
}

void StereoRenderer::shutdown(gpu::VkContext& vk) {
    if (initialized_ && vk.isInitialized()) {
        vkDeviceWaitIdle(vk.device());
    }
    vk_ = nullptr;
    initialized_ = false;
}

} // namespace recorz::render
