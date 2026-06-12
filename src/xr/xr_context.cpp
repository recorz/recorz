#include "xr/xr_context.h"

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
    // Application info
    XrApplicationInfo appInfo{};
    appInfo.apiVersion = XR_CURRENT_API_VERSION;
    std::strncpy(appInfo.applicationName, appName.c_str(), XR_MAX_APPLICATION_NAME_SIZE - 1);
    appInfo.applicationVersion = 1;
    std::strncpy(appInfo.engineName, "Recorz", XR_MAX_ENGINE_NAME_SIZE - 1);
    appInfo.engineVersion = 1;

    // Instance create info
    XrInstanceCreateInfo createInfo{};
    createInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;
    createInfo.applicationInfo = appInfo;

    // Enable extensions (graphics binding will be added later)
    createInfo.enabledExtensionCount = 0;
    createInfo.enabledExtensionNames = nullptr;

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

bool XrContext::createSession() {
    if (instance_ == XR_NULL_HANDLE || systemId_ == XR_NULL_SYSTEM_ID) {
        std::cerr << "Instance or system not ready.\n";
        return false;
    }

    // Note: Graphics binding will be added in Task 1.3 when we integrate Vulkan
    XrSessionCreateInfo sessionInfo{};
    sessionInfo.type = XR_TYPE_SESSION_CREATE_INFO;
    sessionInfo.systemId = systemId_;
    sessionInfo.next = nullptr; // Graphics binding goes here later

    XrResult result = xrCreateSession(instance_, &sessionInfo, &session_);
    if (result != XR_SUCCESS) {
        std::cerr << "Failed to create XR session. Error: " << result << "\n";
        return false;
    }

    std::cout << "XR session created successfully.\n";
    return true;
}

} // namespace recorz::xr