#include "render/stereo_renderer.h"

#include "math.hpp"
#include "render/view_matrices.h"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace recorz::render {
namespace {

const gpu::ClearColor kBackgroundColor{0.04f, 0.05f, 0.08f, 1.0f};

} // namespace

bool StereoRenderer::init(
    gpu::VkContext& vk,
    const xr::XrSwapchainGroup& xrSwapchains,
    const std::string& shaderDir) {
    if (initialized_) {
        return true;
    }

    if (!vk.isInitialized() || xrSwapchains.eyeCount() == 0) {
        std::cerr << "StereoRenderer init: Vulkan or swapchains not ready.\n";
        return false;
    }

    const VkFormat colorFormat = static_cast<VkFormat>(xrSwapchains.format());

    if (!cubeMesh_.create(vk)) {
        std::cerr << "Failed to create cube mesh.\n";
        return false;
    }

    if (!cubePipeline_.create(vk, colorFormat, shaderDir)) {
        std::cerr << "Failed to create cube pipeline.\n";
        cubeMesh_.destroy(vk.device());
        return false;
    }

    vk_ = &vk;
    initialized_ = true;
    return true;
}

bool StereoRenderer::renderFrame(
    const FramePacket& packet,
    xr::XrSwapchainGroup& xrSwapchains,
    xr::XrSpace& referenceSpace,
    gpu::StereoSwapchainImages& swapchainImages,
    gpu::CommandRing& commandRing,
    XrCompositionLayerProjection& outLayer,
    std::array<XrCompositionLayerProjectionView, gpu::StereoSwapchainImages::kMaxEyes>& outViews) {
    if (!initialized_ || vk_ == nullptr || !cubePipeline_.isReady()) {
        return false;
    }

    const uint32_t frameIndex = commandRing.currentFrame();
    VkCommandBuffer commandBuffer = commandRing.begin(frameIndex);
    if (commandBuffer == VK_NULL_HANDLE) {
        return false;
    }

    uint32_t imageIndices[gpu::StereoSwapchainImages::kMaxEyes]{};

    for (uint32_t eye = 0; eye < xrSwapchains.eyeCount(); ++eye) {
        xr::XrSwapchain& xrSwapchain = xrSwapchains.eye(eye);
        if (!xrSwapchain.acquireImage(&imageIndices[eye])) {
            std::cerr << "Failed to acquire swapchain image for eye " << eye << ".\n";
            return false;
        }
        if (!xrSwapchain.waitImage()) {
            std::cerr << "Failed to wait on swapchain image for eye " << eye << ".\n";
            return false;
        }

        gpu::VkSwapchainImages& gpuImages = swapchainImages.eye(eye);
        renderer_.beginColorPass(commandBuffer, gpuImages, imageIndices[eye], kBackgroundColor);

        const math::Mat4 mvp = mvpForView(packet.views[eye], packet.displayTime);
        cubePipeline_.bind(commandBuffer);
        cubePipeline_.pushMvp(commandBuffer, glm::value_ptr(mvp));
        cubePipeline_.draw(commandBuffer, cubeMesh_);

        renderer_.endColorPass(commandBuffer, gpuImages, imageIndices[eye]);
    }

    if (!commandRing.submit(vk_->graphicsQueue(), frameIndex)) {
        return false;
    }
    if (!commandRing.wait(frameIndex)) {
        return false;
    }

    for (uint32_t eye = 0; eye < xrSwapchains.eyeCount(); ++eye) {
        if (!xrSwapchains.eye(eye).releaseImage()) {
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
        view.subImage.swapchain = xrSwapchains.eye(eye).getHandle();
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
    cubePipeline_.destroy(vk.device());
    cubeMesh_.destroy(vk.device());
    vk_ = nullptr;
    initialized_ = false;
}

} // namespace recorz::render
