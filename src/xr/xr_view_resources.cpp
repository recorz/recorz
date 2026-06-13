#include "xr/xr_view_resources.h"

#include <iostream>

namespace recorz::xr {

bool XrViewResources::isReady() const {
    return referenceSpace_.handle() != XR_NULL_HANDLE;
}

bool XrViewResources::create(XrSession session, const StereoViewConfiguration& viewConfiguration) {
    if (isReady()) {
        return true;
    }

    if (session == XR_NULL_HANDLE) {
        std::cerr << "Invalid session for XR view resource creation.\n";
        return false;
    }

    std::cout << "Initializing XR view resources (reference space, swapchains)...\n";

    if (!referenceSpace_.create(session, viewConfiguration, XR_REFERENCE_SPACE_TYPE_LOCAL)) {
        std::cerr << "Failed to create reference space.\n";
        return false;
    }

    if (!swapchains_.create(session, viewConfiguration)) {
        std::cerr << "Failed to create stereo swapchains.\n";
        referenceSpace_.destroy();
        return false;
    }

    std::cout << "XR view resources ready.\n";
    return true;
}

void XrViewResources::destroy() {
    if (!isReady()) {
        return;
    }

    swapchains_.destroy();
    referenceSpace_.destroy();
}

} // namespace recorz::xr
