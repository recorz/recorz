#pragma once

#include "gpu/vk_context.h"
#include "platform/xr_vulkan_bridge.h"
#include "xr/xr_context.h"

#include <string>

namespace recorz::platform {

struct BootstrapInitInfo {
    std::string applicationName = "Recorz Minimal";
    VulkanCreateInfo vulkanCreateInfo{};
};

// Startup-only composition root: OpenXR instance, Vulkan enable2, session creation.
// Does not own frame loop, swapchains, or rendering resources.
class GraphicsBootstrap {
public:
    GraphicsBootstrap() = default;
    ~GraphicsBootstrap();

    GraphicsBootstrap(const GraphicsBootstrap&) = delete;
    GraphicsBootstrap& operator=(const GraphicsBootstrap&) = delete;

    bool init(const BootstrapInitInfo& initInfo = {});
    void shutdown();

    xr::XrContext& xr() { return xr_; }
    const xr::XrContext& xr() const { return xr_; }

    gpu::VkContext& vk() { return vk_; }
    const gpu::VkContext& vk() const { return vk_; }

    bool isInitialized() const { return initialized_; }

private:
    xr::XrContext xr_;
    gpu::VkContext vk_;
    bool initialized_ = false;
};

} // namespace recorz::platform
