#pragma once

#include <openxr/openxr.h>
#include <vulkan/vulkan.h>
#include <string>
#include "xr/xr_swapchain.h"

namespace recorz::vulkan { class VkContext; }

namespace recorz::xr {

class XrContext {
public:
    XrContext() = default;
    ~XrContext();

    // Initialize OpenXR instance
    bool init(const std::string& appName = "Recorz Minimal");

    // Get the XR system (headset)
    bool selectSystem();

    // Create a session using Vulkan graphics binding
    bool createSession(const recorz::vulkan::VkContext& vkContext);

    // Create swapchains for both eyes
    bool createSwapchains(const recorz::vulkan::VkContext& vkContext);

    // Basic frame loop
    bool beginFrame();
    bool endFrame();

    // Accessors
    XrInstance getInstance() const { return instance_; }
    XrSystemId getSystemId() const { return systemId_; }
    XrSession  getSession()  const { return session_; }

private:
    XrInstance instance_ = XR_NULL_HANDLE;
    XrSystemId systemId_ = XR_NULL_SYSTEM_ID;
    XrSession  session_  = XR_NULL_HANDLE;

    XrSwapchain leftSwapchain_;
    XrSwapchain rightSwapchain_;

    XrFrameState frameState_{};
};

} // namespace recorz::xr
