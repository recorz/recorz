#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace recorz::vulkan {

class VkContext {
public:
    VkContext() = default;
    ~VkContext();

    // Initialize Vulkan instance and device
    bool init();

    // Basic accessors
    VkInstance       getInstance() const { return instance_; }
    VkPhysicalDevice getPhysicalDevice() const { return physicalDevice_; }
    VkDevice         getDevice() const { return device_; }
    VkQueue          getGraphicsQueue() const { return graphicsQueue_; }
    uint32_t         getGraphicsQueueFamily() const { return graphicsQueueFamily_; }

private:
    bool createInstance();
    bool pickPhysicalDevice();
    bool createLogicalDevice();

    VkInstance       instance_       = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkDevice         device_         = VK_NULL_HANDLE;
    VkQueue          graphicsQueue_  = VK_NULL_HANDLE;

    uint32_t graphicsQueueFamily_ = UINT32_MAX;

    std::vector<const char*> instanceExtensions_;
    std::vector<const char*> deviceExtensions_;
};

} // namespace recorz::vulkan
