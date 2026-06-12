#pragma once

#include "gpu/vk_context.h"
#include "platform/xr_vulkan_bridge.h"
#include "xr/xr_context.h"

#include <string>

namespace recorz::platform {

struct GraphicsInitInfo {
    std::string applicationName = "Recorz Minimal";
    VulkanCreateInfo vulkanCreateInfo{};
};

class GraphicsContext {
public:
    GraphicsContext() = default;
    ~GraphicsContext();

    GraphicsContext(const GraphicsContext&) = delete;
    GraphicsContext& operator=(const GraphicsContext&) = delete;

    bool init(const GraphicsInitInfo& initInfo = {});
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
