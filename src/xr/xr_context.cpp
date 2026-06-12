#include "xr/xr_context.h"
#include "vulkan/vk_context.h"

#include <openxr/openxr_platform.h>
#include <iostream>
#include <cstring>

namespace recorz::xr {

XrContext::~XrContext() {
    if (session_ != XR_NULL_HANDLE) {
        xrDestroySession(session_);
    }
    if (instance_ != XR_NULL_HANDLE) {
        xrDestroyInstance(instance_);
    }
}

bool XrContext::init(const std::string& appName) {
    XrApplicationInfo appInfo{};
    appInfo.apiVersion = XR_CURRENT_API_VERSION;
    std::strncpy(appInfo.applicationName, appName.c_str(), XR_MAX_APPLICATION_NAME_SIZE - 1);
    appInfo.applicationVersion = 1;
    std::strncpy(appInfo.engineName, "Recorz", XR_MAX_ENGINE_NAME_SIZE - 1);
    appInfo.engineVersion = 1;

    const char* extension = XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME;

    XrInstanceCreateInfo createInfo{};
    createInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;
    createInfo.applicationInfo = appInfo;
    createInfo.enabledExtensionCount = 1;
    createInfo.enabledExtensionNames = &extension;

    XrResult result = xrCreateInstance(&createInfo, &instance_);
    if (result != XR_SUCCESS) {
        std::cerr << "Failed to create OpenXR instance. Error: " << result << "\n";
        return false;
    }

    std::cout << "OpenXR instance created successfully.\n";
    return true;
}

bool XrContext::selectSystem() {
    if (instance_ == XR_NULL_HANDLE) {
        std::cerr << "Instance not initialized.\n";
        return false;
    }

    XrSystemGetInfo systemInfo{};
    systemInfo.type = XR_TYPE_SYSTEM_GET_INFO;
    systemInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

    XrResult result = xrGetSystem(instance_, &systemInfo, &systemId_);
    if (result != XR_SUCCESS) {
        std::cerr << "Failed to get XR system. Error: " << result << "\n";
        return false;
    }

    std::cout << "XR system selected successfully.\n";
    return true;
}

bool XrContext::createSession(const recorz::vulkan::VkContext& vkContext) {
    if (instance_ == XR_NULL_HANDLE || systemId_ == XR_NULL_SYSTEM_ID) {
        std::cerr << "Instance or system not ready.\n";
        return false;
    }

    XrGraphicsBindingVulkanKHR graphicsBinding{};
    graphicsBinding.type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR;
    graphicsBinding.instance = vkContext.getInstance();
    graphicsBinding.physicalDevice = vkContext.getPhysicalDevice();
    graphicsBinding.device = vkContext.getDevice();
    graphicsBinding.queueFamilyIndex = vkContext.getGraphicsQueueFamily();
    graphicsBinding.queueIndex = 0;

    XrSessionCreateInfo sessionInfo{};
    sessionInfo.type = XR_TYPE_SESSION_CREATE_INFO;
    sessionInfo.systemId = systemId_;
    sessionInfo.next = &graphicsBinding;

    XrResult result = xrCreateSession(instance_, &sessionInfo, &session_);
    if (result != XR_SUCCESS) {
        std::cerr << "Failed to create XR session. Error: " << result << "\n";
        return false;
    }

    std::cout << "XR session created successfully with Vulkan graphics binding.\n";
    return true;
}

bool XrContext::createSwapchains(const recorz::vulkan::VkContext& vkContext) {
    if (session_ == XR_NULL_HANDLE) {
        std::cerr << "Session not created yet.\n";
        return false;
    }

    // TODO: Query recommended format and size from xrEnumerateViewConfigurationViews
    int64_t colorFormat = VK_FORMAT_R8G8B8A8_SRGB;
    uint32_t width = 1440;
    uint32_t height = 1600;

    if (!leftSwapchain_.create(session_, colorFormat, width, height)) {
        return false;
    }

    if (!rightSwapchain_.create(session_, colorFormat, width, height)) {
        return false;
    }

    std::cout << "Swapchains created for both eyes.\n";
    return true;
}

bool XrContext::beginFrame() {
    XrFrameWaitInfo waitInfo{};
    waitInfo.type = XR_TYPE_FRAME_WAIT_INFO;

    XrResult result = xrWaitFrame(session_, &waitInfo, &frameState_);
    if (result != XR_SUCCESS) return false;

    XrFrameBeginInfo beginInfo{};
    beginInfo.type = XR_TYPE_FRAME_BEGIN_INFO;

    result = xrBeginFrame(session_, &beginInfo);
    return result == XR_SUCCESS;
}

bool XrContext::endFrame() {
    // TODO: Properly fill projection views using xrLocateViews
    XrCompositionLayerProjectionView layerViews[2] = {};

    XrCompositionLayerProjection layer{};
    layer.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION;
    layer.space = XR_NULL_HANDLE; // Should use a reference space
    layer.viewCount = 2;
    layer.views = layerViews;

    const XrCompositionLayerBaseHeader* layers[1] = {
        reinterpret_cast<const XrCompositionLayerBaseHeader*>(&layer)
    };

    XrFrameEndInfo endInfo{};
    endInfo.type = XR_TYPE_FRAME_END_INFO;
    endInfo.displayTime = frameState_.predictedDisplayTime;
    endInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    endInfo.layerCount = 1;
    endInfo.layers = layers;

    XrResult result = xrEndFrame(session_, &endInfo);
    return result == XR_SUCCESS;
}

} // namespace recorz::xr
