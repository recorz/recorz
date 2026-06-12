#include "xr/xr_context.h"
#include "vulkan/vk_context.h"
#include <iostream>

int main() {
    recorz::vulkan::VkContext vk;
    recorz::xr::XrContext xr;

    if (!vk.init()) {
        std::cerr << "Failed to initialize Vulkan.\n";
        return 1;
    }

    if (!xr.init("Recorz Minimal")) {
        return 1;
    }

    if (!xr.selectSystem()) {
        return 1;
    }

    if (!xr.createSession(vk)) {
        return 1;
    }

    std::cout << "OpenXR + Vulkan session created successfully!\n";
    return 0;
}