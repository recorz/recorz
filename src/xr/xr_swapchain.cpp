#include "xr/xr_swapchain.h"

#include <openxr/openxr_platform.h>
#include <iostream>

namespace recorz::xr {

XrSwapchain::~XrSwapchain() {
    destroy();
}

bool XrSwapchain::create(XrSession session, int64_t format, uint32_t width, uint32_t height, uint32_t sampleCount) {
    session_ = session;

    XrSwapchainCreateInfo createInfo{};
    createInfo.type = XR_TYPE_SWAPCHAIN_CREATE_INFO;
    createInfo.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT;
    createInfo.format = format;
    createInfo.sampleCount = sampleCount;
    createInfo.width = width;
    createInfo.height = height;
    createInfo.faceCount = 1;
    createInfo.arraySize = 1;
    createInfo.mipCount = 1;

    XrResult result = xrCreateSwapchain(session_, &createInfo, &handle_);
    if (result != XR_SUCCESS) {
        std::cerr << "Failed to create swapchain. Error: " << result << "\n";
        return false;
    }

    // Enumerate swapchain images
    uint32_t imageCount = 0;
    xrEnumerateSwapchainImages(handle_, 0, &imageCount, nullptr);

    images_.resize(imageCount);
    for (auto& img : images_) {
        img.type = XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR;
    }

    result = xrEnumerateSwapchainImages(
        handle_,
        imageCount,
        &imageCount,
        reinterpret_cast<XrSwapchainImageBaseHeader*>(images_.data())
    );

    if (result != XR_SUCCESS) {
        std::cerr << "Failed to enumerate swapchain images. Error: " << result << "\n";
        return false;
    }

    std::cout << "Swapchain created with " << imageCount << " images.\n";
    return true;
}

void XrSwapchain::destroy() {
    if (handle_ != XR_NULL_HANDLE) {
        xrDestroySwapchain(handle_);
        handle_ = XR_NULL_HANDLE;
    }
    images_.clear();
}

bool XrSwapchain::acquireImage(uint32_t* imageIndex) {
    XrSwapchainImageAcquireInfo acquireInfo{};
    acquireInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO;

    XrResult result = xrAcquireSwapchainImage(handle_, &acquireInfo, imageIndex);
    return result == XR_SUCCESS;
}

bool XrSwapchain::waitImage(XrDuration timeout) {
    XrSwapchainImageWaitInfo waitInfo{};
    waitInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO;
    waitInfo.timeout = timeout;

    XrResult result = xrWaitSwapchainImage(handle_, &waitInfo);
    return result == XR_SUCCESS;
}

bool XrSwapchain::releaseImage() {
    XrSwapchainImageReleaseInfo releaseInfo{};
    releaseInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO;

    XrResult result = xrReleaseSwapchainImage(handle_, &releaseInfo);
    return result == XR_SUCCESS;
}

} // namespace recorz::xr
