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

// XR + Vulkan startup coordinator (enable2 instance/device/session).
// Owns XrContext and VkContext through session creation; does not own
// per-session rendering resources or the compositor frame loop.
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
