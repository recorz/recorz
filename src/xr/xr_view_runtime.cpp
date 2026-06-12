#include "xr/xr_view_runtime.h"

#include <iostream>

namespace recorz::xr {

bool XrViewRuntime::isReady() const {
    return referenceSpace_.handle() != XR_NULL_HANDLE;
}

bool XrViewRuntime::create(
    XrSession session,
    VkDevice device,
    const StereoViewConfiguration& viewConfiguration) {
    if (isReady()) {
        return true;
    }

    if (session == XR_NULL_HANDLE || device == VK_NULL_HANDLE) {
        std::cerr << "Invalid handles for XR view runtime creation.\n";
        return false;
    }

    std::cout << "Initializing XR view runtime (reference space, swapchains)...\n";

    if (!referenceSpace_.create(session, viewConfiguration, XR_REFERENCE_SPACE_TYPE_LOCAL)) {
        std::cerr << "Failed to create reference space.\n";
        return false;
    }

    if (!swapchains_.create(session, device, viewConfiguration)) {
        std::cerr << "Failed to create stereo swapchains.\n";
        referenceSpace_.destroy();
        return false;
    }

    std::cout << "XR view runtime ready.\n";
    return true;
}

void XrViewRuntime::destroy(VkDevice device) {
    if (!isReady()) {
        return;
    }

    swapchains_.destroy(device);
    referenceSpace_.destroy();
}

} // namespace recorz::xr
