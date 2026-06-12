#pragma once

#include <vulkan/vulkan.h>
#include <openxr/openxr_platform.h>

namespace recorz::gpu {

struct AdoptedDevice {
    VkInstance       instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice         device = VK_NULL_HANDLE;
    uint32_t         graphicsQueueFamily = UINT32_MAX;
    VkQueue          graphicsQueue = VK_NULL_HANDLE;
};

class VkContext {
public:
    VkContext() = default;
    ~VkContext();

    VkContext(const VkContext&) = delete;
    VkContext& operator=(const VkContext&) = delete;

    // Take ownership of handles created via OpenXR enable2 helpers.
    bool adopt(AdoptedDevice device);

    void shutdown();

    XrGraphicsBindingVulkanKHR graphicsBinding() const;

    VkInstance       instance() const { return instance_; }
    VkPhysicalDevice physicalDevice() const { return physicalDevice_; }
    VkDevice         device() const { return device_; }
    VkQueue          graphicsQueue() const { return graphicsQueue_; }
    uint32_t         graphicsQueueFamily() const { return graphicsQueueFamily_; }
    bool             isInitialized() const { return device_ != VK_NULL_HANDLE; }

private:
    VkInstance       instance_ = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkDevice         device_ = VK_NULL_HANDLE;
    VkQueue          graphicsQueue_ = VK_NULL_HANDLE;
    uint32_t         graphicsQueueFamily_ = UINT32_MAX;
};

} // namespace recorz::gpu
