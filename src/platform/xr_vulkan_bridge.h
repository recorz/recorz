#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <string>

namespace recorz::gpu { class VkContext; }
namespace recorz::xr { class XrContext; }

namespace recorz::platform {

struct VulkanCreateInfo {
    std::string applicationName = "Recorz Minimal";
    uint32_t desiredApiVersion = VK_API_VERSION_1_2;
};

class XrVulkanBridge {
public:
    static bool createVulkanForOpenXR(
        const xr::XrContext& xr,
        gpu::VkContext& vk,
        const VulkanCreateInfo& createInfo = {});
};

} // namespace recorz::platform
