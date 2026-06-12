#include "gpu/vk_context.h"

namespace recorz::gpu {

VkContext::~VkContext() {
    shutdown();
}

bool VkContext::adopt(AdoptedDevice device) {
    if (device.instance == VK_NULL_HANDLE || device.device == VK_NULL_HANDLE ||
        device.graphicsQueueFamily == UINT32_MAX) {
        return false;
    }

    shutdown();

    instance_ = device.instance;
    physicalDevice_ = device.physicalDevice;
    device_ = device.device;
    graphicsQueueFamily_ = device.graphicsQueueFamily;
    graphicsQueue_ = device.graphicsQueue;

    if (graphicsQueue_ == VK_NULL_HANDLE) {
        vkGetDeviceQueue(device_, graphicsQueueFamily_, 0, &graphicsQueue_);
    }

    return true;
}

void VkContext::shutdown() {
    if (device_ != VK_NULL_HANDLE) {
        vkDestroyDevice(device_, nullptr);
        device_ = VK_NULL_HANDLE;
        graphicsQueue_ = VK_NULL_HANDLE;
    }
    if (instance_ != VK_NULL_HANDLE) {
        vkDestroyInstance(instance_, nullptr);
        instance_ = VK_NULL_HANDLE;
    }
    physicalDevice_ = VK_NULL_HANDLE;
    graphicsQueueFamily_ = UINT32_MAX;
}

XrGraphicsBindingVulkanKHR VkContext::graphicsBinding() const {
    XrGraphicsBindingVulkanKHR binding{};
    binding.type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR;
    binding.instance = instance_;
    binding.physicalDevice = physicalDevice_;
    binding.device = device_;
    binding.queueFamilyIndex = graphicsQueueFamily_;
    binding.queueIndex = 0;
    return binding;
}

} // namespace recorz::gpu
