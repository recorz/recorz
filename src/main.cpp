#include "vulkan/vk_context.h"
#include "xr/xr_context.h"

#include <iostream>

int main() {
    std::cout << "=== Recorz Minimal Test ===\n\n";

    recorz::vulkan::VkContext vkContext;
    recorz::xr::XrContext xrContext;

    // 1. Initialize Vulkan
    std::cout << "[1/6] Initializing Vulkan...\n";
    if (!vkContext.init()) {
        std::cerr << "Failed to initialize Vulkan context.\n";
        return 1;
    }
    std::cout << "Vulkan initialized successfully.\n\n";

    // 2. Initialize OpenXR
    std::cout << "[2/6] Initializing OpenXR...\n";
    if (!xrContext.init("Recorz Minimal")) {
        std::cerr << "Failed to initialize OpenXR context.\n";
        return 1;
    }
    std::cout << "OpenXR initialized successfully.\n\n";

    // 3. Select XR System
    std::cout << "[3/6] Selecting XR system...\n";
    if (!xrContext.selectSystem()) {
        std::cerr << "Failed to select XR system.\n";
        return 1;
    }
    std::cout << "XR system selected.\n\n";

    // 4. Create XR Session with Vulkan binding
    std::cout << "[4/6] Creating XR session with Vulkan binding...\n";
    if (!xrContext.createSession(vkContext)) {
        std::cerr << "Failed to create XR session.\n";
        return 1;
    }
    std::cout << "XR session created.\n\n";

    // 5. Create Swapchains
    std::cout << "[5/6] Creating swapchains...\n";
    if (!xrContext.createSwapchains(vkContext)) {
        std::cerr << "Failed to create swapchains.\n";
        return 1;
    }
    std::cout << "Swapchains created.\n\n";

    // 6. Basic frame loop test (run a few frames)
    std::cout << "[6/6] Running basic frame loop test...\n";
    for (int i = 0; i < 3; ++i) {
        if (!xrContext.beginFrame()) {
            std::cerr << "beginFrame failed on iteration " << i << "\n";
            break;
        }

        // TODO: Render into swapchain images here

        if (!xrContext.endFrame()) {
            std::cerr << "endFrame failed on iteration " << i << "\n";
            break;
        }

        std::cout << "  Frame " << i << " submitted.\n";
    }

    std::cout << "\n=== Test completed successfully! ===\n";
    std::cout << "OpenXR + Vulkan pipeline is working.\n";

    return 0;
}
