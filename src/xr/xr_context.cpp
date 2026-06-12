#include "xr/xr_context.h"

#include "xr/xr_common.h"

#include <iostream>

namespace recorz::xr {

XrContext::~XrContext() {
    shutdown();
}

bool XrContext::createInstance(const std::string& appName) {
    if (instance_ != XR_NULL_HANDLE) {
        return true;
    }

    XrApplicationInfo appInfo{};
    appInfo.apiVersion = XR_CURRENT_API_VERSION;
    copyXrString(appInfo.applicationName, XR_MAX_APPLICATION_NAME_SIZE, appName);
    appInfo.applicationVersion = 1;
    copyXrString(appInfo.engineName, XR_MAX_ENGINE_NAME_SIZE, "Recorz");
    appInfo.engineVersion = 1;

    const char* extensions[] = {XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME};

    XrInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.applicationInfo = appInfo;
    instanceCreateInfo.enabledExtensionCount = 1;
    instanceCreateInfo.enabledExtensionNames = extensions;

    if (!checkXr(xrCreateInstance(&instanceCreateInfo, &instance_), "xrCreateInstance")) {
        return false;
    }

    std::cout << "OpenXR instance created.\n";
    return true;
}

bool XrContext::selectSystem(XrFormFactor formFactor) {
    if (instance_ == XR_NULL_HANDLE) {
        std::cerr << "OpenXR instance must be created before selecting a system.\n";
        return false;
    }

    XrSystemGetInfo systemInfo{};
    systemInfo.type = XR_TYPE_SYSTEM_GET_INFO;
    systemInfo.formFactor = formFactor;

    if (!checkXr(xrGetSystem(instance_, &systemInfo, &systemId_), "xrGetSystem")) {
        return false;
    }

    std::cout << "OpenXR system selected.\n";
    return true;
}

bool XrContext::loadStereoViews() {
    if (session_ == XR_NULL_HANDLE) {
        std::cerr << "OpenXR session must be created before loading view configuration.\n";
        return false;
    }

    return loadStereoViewConfiguration(instance_, systemId_, stereoViews_);
}

bool XrContext::createSession(const XrGraphicsBindingVulkanKHR& graphicsBinding) {
    if (instance_ == XR_NULL_HANDLE || systemId_ == XR_NULL_SYSTEM_ID) {
        std::cerr << "Cannot create session: OpenXR instance or system not ready.\n";
        return false;
    }

    if (session_ != XR_NULL_HANDLE) {
        return true;
    }

    XrSessionCreateInfo sessionCreateInfo{};
    sessionCreateInfo.type = XR_TYPE_SESSION_CREATE_INFO;
    sessionCreateInfo.systemId = systemId_;
    sessionCreateInfo.next = const_cast<XrGraphicsBindingVulkanKHR*>(&graphicsBinding);

    if (!checkXr(xrCreateSession(instance_, &sessionCreateInfo, &session_), "xrCreateSession")) {
        return false;
    }

    std::cout << "OpenXR session created.\n";
    return true;
}

void XrContext::destroySession() {
    if (session_ != XR_NULL_HANDLE) {
        xrDestroySession(session_);
        session_ = XR_NULL_HANDLE;
    }
}

void XrContext::shutdown() {
    destroySession();

    if (instance_ != XR_NULL_HANDLE) {
        xrDestroyInstance(instance_);
        instance_ = XR_NULL_HANDLE;
    }

    systemId_ = XR_NULL_SYSTEM_ID;
    stereoViews_ = {};
}

} // namespace recorz::xr
