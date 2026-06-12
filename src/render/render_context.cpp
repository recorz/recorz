#include "render/render_context.h"

#include <iostream>

namespace recorz::render {

bool RenderContext::isReady() const {
    return ready_;
}

bool RenderContext::create(gpu::VkContext& vk, xr::XrSwapchainGroup& swapchains) {
    if (isReady()) {
        return true;
    }

    if (!vk.isInitialized() || swapchains.eyeCount() == 0) {
        std::cerr << "Vulkan or swapchains not ready for render context creation.\n";
        return false;
    }

    std::cout << "Initializing render context (command ring, stereo renderer)...\n";

    if (!commandRing_.create(vk.device(), vk.graphicsQueueFamily())) {
        std::cerr << "Failed to create command ring.\n";
        return false;
    }

    if (!stereoRenderer_.init(vk, swapchains)) {
        std::cerr << "Failed to initialize stereo renderer.\n";
        commandRing_.destroy(vk.device());
        return false;
    }

    ready_ = true;
    std::cout << "Render context ready.\n";
    return true;
}

void RenderContext::destroy(gpu::VkContext& vk) {
    if (!isReady()) {
        return;
    }

    stereoRenderer_.shutdown(vk);
    commandRing_.destroy(vk.device());
    ready_ = false;
}

} // namespace recorz::render
