#include "xr/xr_context.h"

#include <iostream>

int main() {
    std::cout << "=== Recorz Minimal: OpenXR + Vulkan (enable2) ===\n\n";

    recorz::xr::XrContext xr;

    std::cout << "[1/2] Initializing OpenXR and Vulkan via OpenXR helpers...\n";
    if (!xr.init("Recorz Minimal")) {
        std::cerr << "Initialization failed.\n";
        return 1;
    }

    std::cout << "\n[2/2] Creating OpenXR session...\n";
    if (!xr.createSession()) {
        std::cerr << "Session creation failed.\n";
        return 1;
    }

    std::cout << "\n=== Success ===\n";
    std::cout << "OpenXR instance:  " << xr.getInstance() << "\n";
    std::cout << "Vulkan instance:  " << xr.getVulkanInstance() << "\n";
    std::cout << "Vulkan device:    " << xr.getDevice() << "\n";
    std::cout << "OpenXR session:   " << xr.getSession() << "\n";
    return 0;
}
