#include "app/vr_session_resources.h"

#include <iostream>

namespace recorz::app {

bool VrSessionResources::isReady() const {
    return referenceSpace_.handle() != XR_NULL_HANDLE;
}

bool VrSessionResources::create(const VrSessionContext& ctx) {
    if (isReady()) {
        return true;
    }

    if (ctx.session == XR_NULL_HANDLE || ctx.vk == nullptr || ctx.viewConfiguration == nullptr) {
        std::cerr << "Invalid session context for VR resource creation.\n";
        return false;
    }

    std::cout << "Initializing VR resources (reference space, swapchains, renderer)...\n";

    if (!referenceSpace_.create(ctx.session, *ctx.viewConfiguration, XR_REFERENCE_SPACE_TYPE_LOCAL)) {
        std::cerr << "Failed to create reference space.\n";
        return false;
    }

    if (!swapchains_.create(ctx.session, ctx.vk->device(), *ctx.viewConfiguration)) {
        std::cerr << "Failed to create stereo swapchains.\n";
        referenceSpace_.destroy();
        return false;
    }

    if (!commandRing_.create(ctx.vk->device(), ctx.vk->graphicsQueueFamily())) {
        std::cerr << "Failed to create command ring.\n";
        swapchains_.destroy(ctx.vk->device());
        referenceSpace_.destroy();
        return false;
    }

    if (!stereoRenderer_.init(*ctx.vk, swapchains_)) {
        std::cerr << "Failed to initialize stereo renderer.\n";
        commandRing_.destroy(ctx.vk->device());
        swapchains_.destroy(ctx.vk->device());
        referenceSpace_.destroy();
        return false;
    }

    std::cout << "VR resources ready. Starting render loop.\n";
    return true;
}

void VrSessionResources::destroy(gpu::VkContext& vk) {
    if (!isReady()) {
        return;
    }

    stereoRenderer_.shutdown(vk);
    commandRing_.destroy(vk.device());
    swapchains_.destroy(vk.device());
    referenceSpace_.destroy();
}

} // namespace recorz::app
