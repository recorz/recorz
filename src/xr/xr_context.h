#pragma once

#include <vulkan/vulkan.h>
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <string>

namespace recorz::xr {

class XrContext {
public:
    XrContext() = default;
    ~XrContext();

    // OpenXR instance, XR system, and Vulkan instance/device (via OpenXR helpers).
    bool init(const std::string& appName = "Recorz Minimal");

    // xrCreateSession with XrGraphicsBindingVulkanKHR.
    bool createSession();

    bool createSwapchains();
    bool beginFrame();
    bool endFrame();

    XrInstance getInstance() const { return instance_; }
    XrSystemId getSystemId() const { return systemId_; }
    XrSession  getSession()  const { return session_; }

    VkInstance       getVulkanInstance() const { return vulkanInstance_; }
    VkPhysicalDevice getPhysicalDevice() const { return physicalDevice_; }
    VkDevice         getDevice() const { return device_; }
    VkQueue          getGraphicsQueue() const { return graphicsQueue_; }
    uint32_t         getGraphicsQueueFamily() const { return graphicsQueueFamily_; }

private:
    XrInstance instance_ = XR_NULL_HANDLE;
    XrSystemId systemId_ = XR_NULL_SYSTEM_ID;
    XrSession  session_  = XR_NULL_HANDLE;

    VkInstance       vulkanInstance_ = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkDevice         device_         = VK_NULL_HANDLE;
    VkQueue          graphicsQueue_  = VK_NULL_HANDLE;
    uint32_t         graphicsQueueFamily_ = UINT32_MAX;

    XrFrameState frameState_{};
};

} // namespace recorz::xr
