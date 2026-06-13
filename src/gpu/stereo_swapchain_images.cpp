#include "gpu/stereo_swapchain_images.h"

#include "xr/xr_swapchain_group.h"

#include <iostream>
#include <vector>

namespace recorz::gpu {

StereoSwapchainImages::~StereoSwapchainImages() {
    if (eyeCount_ > 0) {
        std::cerr << "StereoSwapchainImages destroyed without calling destroy().\n";
    }
}

bool StereoSwapchainImages::create(VkDevice device, const xr::XrSwapchainGroup& xrSwapchains) {
    destroy(device);

    if (device == VK_NULL_HANDLE || xrSwapchains.eyeCount() == 0) {
        std::cerr << "Invalid device or XR swapchains for stereo swapchain images.\n";
        return false;
    }

    eyeCount_ = xrSwapchains.eyeCount();
    const VkFormat format = static_cast<VkFormat>(xrSwapchains.format());

    for (uint32_t eye = 0; eye < eyeCount_; ++eye) {
        const xr::XrSwapchain& xrSwapchain = xrSwapchains.eye(eye);
        const auto& xrImages = xrSwapchain.getImages();

        std::vector<VkImage> vkImages;
        vkImages.reserve(xrImages.size());
        for (const auto& xrImage : xrImages) {
            vkImages.push_back(xrImage.image);
        }

        const uint32_t width = xrSwapchain.width();
        const uint32_t height = xrSwapchain.height();
        const VkExtent2D extent{width, height};

        if (!images_[eye].create(
                device,
                vkImages.data(),
                static_cast<uint32_t>(vkImages.size()),
                format,
                extent)) {
            destroy(device);
            return false;
        }
    }

    return true;
}

void StereoSwapchainImages::destroy(VkDevice device) {
    for (uint32_t eye = 0; eye < eyeCount_; ++eye) {
        images_[eye].destroy(device);
    }
    eyeCount_ = 0;
}

VkSwapchainImages& StereoSwapchainImages::eye(uint32_t index) {
    return images_[index];
}

const VkSwapchainImages& StereoSwapchainImages::eye(uint32_t index) const {
    return images_[index];
}

} // namespace recorz::gpu
