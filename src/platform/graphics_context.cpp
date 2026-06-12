#include "platform/graphics_context.h"

#include <iostream>

namespace recorz::platform {

GraphicsContext::~GraphicsContext() {
    shutdown();
}

bool GraphicsContext::init(const GraphicsInitInfo& initInfo) {
    if (initialized_) {
        return true;
    }

    if (!xr_.createInstance(initInfo.applicationName)) {
        return false;
    }

    if (!xr_.selectSystem()) {
        xr_.shutdown();
        return false;
    }

    VulkanCreateInfo vulkanCreateInfo = initInfo.vulkanCreateInfo;
    if (vulkanCreateInfo.applicationName.empty()) {
        vulkanCreateInfo.applicationName = initInfo.applicationName;
    }

    if (!XrVulkanBridge::createVulkanForOpenXR(xr_, vk_, vulkanCreateInfo)) {
        xr_.shutdown();
        return false;
    }

    if (!xr_.createSession(vk_.graphicsBinding())) {
        vk_.shutdown();
        xr_.shutdown();
        return false;
    }

    if (!xr_.loadStereoViews()) {
        xr_.destroySession();
        vk_.shutdown();
        xr_.shutdown();
        return false;
    }

    initialized_ = true;
    return true;
}

void GraphicsContext::shutdown() {
    if (!initialized_) {
        return;
    }

    xr_.destroySession();
    vk_.shutdown();
    xr_.shutdown();
    initialized_ = false;
}

} // namespace recorz::platform
