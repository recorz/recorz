#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

namespace recorz::gpu {

class VkSwapchainImages {
public:
    VkSwapchainImages() = default;
    ~VkSwapchainImages();

    VkSwapchainImages(const VkSwapchainImages&) = delete;
    VkSwapchainImages& operator=(const VkSwapchainImages&) = delete;

    bool create(VkDevice device, const VkImage* images, uint32_t imageCount, VkFormat format, VkExtent2D extent);
    void destroy(VkDevice device);

    uint32_t imageCount() const { return static_cast<uint32_t>(images_.size()); }
    VkImage image(uint32_t index) const { return images_[index]; }
    VkImageView view(uint32_t index) const { return views_[index]; }
    VkFormat format() const { return format_; }
    VkExtent2D extent() const { return extent_; }

    VkImageLayout layout(uint32_t index) const { return layouts_[index]; }
    void setLayout(uint32_t index, VkImageLayout layout) { layouts_[index] = layout; }

private:
    VkFormat format_ = VK_FORMAT_UNDEFINED;
    VkExtent2D extent_{};
    std::vector<VkImage> images_;
    std::vector<VkImageView> views_;
    std::vector<VkImageLayout> layouts_;
};

} // namespace recorz::gpu
