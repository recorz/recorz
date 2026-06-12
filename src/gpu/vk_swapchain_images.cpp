#include "gpu/vk_swapchain_images.h"

#include <iostream>

namespace recorz::gpu {

VkSwapchainImages::~VkSwapchainImages() {
    if (!views_.empty()) {
        std::cerr << "VkSwapchainImages destroyed without calling destroy().\n";
    }
}

bool VkSwapchainImages::create(
    VkDevice device,
    const VkImage* images,
    uint32_t imageCount,
    VkFormat format,
    VkExtent2D extent) {
    destroy(device);

    format_ = format;
    extent_ = extent;
    images_.assign(images, images + imageCount);
    layouts_.assign(imageCount, VK_IMAGE_LAYOUT_UNDEFINED);

    views_.reserve(imageCount);
    for (uint32_t i = 0; i < imageCount; ++i) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = images_[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format_;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView view = VK_NULL_HANDLE;
        if (vkCreateImageView(device, &viewInfo, nullptr, &view) != VK_SUCCESS) {
            std::cerr << "Failed to create swapchain image view " << i << ".\n";
            destroy(device);
            return false;
        }
        views_.push_back(view);
    }

    return true;
}

void VkSwapchainImages::destroy(VkDevice device) {
    for (VkImageView view : views_) {
        if (view != VK_NULL_HANDLE) {
            vkDestroyImageView(device, view, nullptr);
        }
    }
    views_.clear();
    images_.clear();
    layouts_.clear();
    format_ = VK_FORMAT_UNDEFINED;
    extent_ = {0, 0};
}

} // namespace recorz::gpu
