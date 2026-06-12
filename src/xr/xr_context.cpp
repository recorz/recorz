#include "xr/xr_context.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>

namespace recorz::xr {
namespace {

bool checkXr(XrResult result, const char* operation) {
    if (result == XR_SUCCESS) {
        return true;
    }
    std::cerr << operation << " failed (XrResult " << result << ").\n";
    return false;
}

template<typename T>
T loadXrFunction(XrInstance instance, const char* name) {
    PFN_xrVoidFunction function = nullptr;
    if (xrGetInstanceProcAddr(instance, name, &function) != XR_SUCCESS || function == nullptr) {
        std::cerr << "Failed to load OpenXR function: " << name << "\n";
        return nullptr;
    }
    return reinterpret_cast<T>(function);
}

void copyXrString(char* destination, size_t capacity, const std::string& value) {
    if (capacity == 0) {
        return;
    }
    std::snprintf(destination, capacity, "%s", value.c_str());
}

uint32_t clampVulkanApiVersion(uint32_t desired, XrVersion minVersion, XrVersion maxVersion) {
    desired = std::max(desired, static_cast<uint32_t>(minVersion));
    desired = std::min(desired, static_cast<uint32_t>(maxVersion));
    return desired;
}

uint32_t findGraphicsQueueFamily(VkPhysicalDevice physicalDevice) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            return i;
        }
    }
    return UINT32_MAX;
}

} // namespace

XrContext::~XrContext() {
    if (session_ != XR_NULL_HANDLE) {
        xrDestroySession(session_);
        session_ = XR_NULL_HANDLE;
    }
    if (device_ != VK_NULL_HANDLE) {
        vkDestroyDevice(device_, nullptr);
        device_ = VK_NULL_HANDLE;
    }
    if (vulkanInstance_ != VK_NULL_HANDLE) {
        vkDestroyInstance(vulkanInstance_, nullptr);
        vulkanInstance_ = VK_NULL_HANDLE;
    }
    if (instance_ != XR_NULL_HANDLE) {
        xrDestroyInstance(instance_);
        instance_ = XR_NULL_HANDLE;
    }
}

bool XrContext::init(const std::string& appName) {
    // 1. xrCreateInstance (XR_KHR_vulkan_enable2)
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

    // 2. xrGetSystem
    XrSystemGetInfo systemInfo{};
    systemInfo.type = XR_TYPE_SYSTEM_GET_INFO;
    systemInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

    if (!checkXr(xrGetSystem(instance_, &systemInfo, &systemId_), "xrGetSystem")) {
        return false;
    }
    std::cout << "OpenXR system selected.\n";

    auto pfnGetVulkanGraphicsRequirements2KHR = loadXrFunction<PFN_xrGetVulkanGraphicsRequirements2KHR>(
        instance_, "xrGetVulkanGraphicsRequirements2KHR");
    auto pfnCreateVulkanInstanceKHR = loadXrFunction<PFN_xrCreateVulkanInstanceKHR>(
        instance_, "xrCreateVulkanInstanceKHR");
    auto pfnGetVulkanGraphicsDevice2KHR = loadXrFunction<PFN_xrGetVulkanGraphicsDevice2KHR>(
        instance_, "xrGetVulkanGraphicsDevice2KHR");
    auto pfnCreateVulkanDeviceKHR = loadXrFunction<PFN_xrCreateVulkanDeviceKHR>(
        instance_, "xrCreateVulkanDeviceKHR");

    if (!pfnGetVulkanGraphicsRequirements2KHR || !pfnCreateVulkanInstanceKHR ||
        !pfnGetVulkanGraphicsDevice2KHR || !pfnCreateVulkanDeviceKHR) {
        return false;
    }

    // 3. xrGetVulkanGraphicsRequirements2KHR
    XrGraphicsRequirementsVulkanKHR requirements{};
    requirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR;

    if (!checkXr(
            pfnGetVulkanGraphicsRequirements2KHR(instance_, systemId_, &requirements),
            "xrGetVulkanGraphicsRequirements2KHR")) {
        return false;
    }
    std::cout << "Vulkan API version range: "
              << XR_VERSION_MAJOR(requirements.minApiVersionSupported) << "."
              << XR_VERSION_MINOR(requirements.minApiVersionSupported) << " - "
              << XR_VERSION_MAJOR(requirements.maxApiVersionSupported) << "."
              << XR_VERSION_MINOR(requirements.maxApiVersionSupported) << "\n";

    // 4. xrCreateVulkanInstanceKHR
    VkApplicationInfo vkAppInfo{};
    vkAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vkAppInfo.pApplicationName = appName.c_str();
    vkAppInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    vkAppInfo.pEngineName = "Recorz";
    vkAppInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    vkAppInfo.apiVersion = clampVulkanApiVersion(
        VK_API_VERSION_1_2,
        requirements.minApiVersionSupported,
        requirements.maxApiVersionSupported);

    VkInstanceCreateInfo vkInstanceCreateInfo{};
    vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkInstanceCreateInfo.pApplicationInfo = &vkAppInfo;

    XrVulkanInstanceCreateInfoKHR xrVulkanInstanceCreateInfo{};
    xrVulkanInstanceCreateInfo.type = XR_TYPE_VULKAN_INSTANCE_CREATE_INFO_KHR;
    xrVulkanInstanceCreateInfo.systemId = systemId_;
    xrVulkanInstanceCreateInfo.createFlags = 0;
    xrVulkanInstanceCreateInfo.pfnGetInstanceProcAddr = &vkGetInstanceProcAddr;
    xrVulkanInstanceCreateInfo.vulkanCreateInfo = &vkInstanceCreateInfo;
    xrVulkanInstanceCreateInfo.vulkanAllocator = nullptr;

    VkResult vkResult = VK_SUCCESS;
    if (!checkXr(
            pfnCreateVulkanInstanceKHR(
                instance_, &xrVulkanInstanceCreateInfo, &vulkanInstance_, &vkResult),
            "xrCreateVulkanInstanceKHR")) {
        return false;
    }
    if (vkResult != VK_SUCCESS) {
        std::cerr << "xrCreateVulkanInstanceKHR returned VkResult " << vkResult << ".\n";
        return false;
    }
    std::cout << "Vulkan instance created via OpenXR.\n";

    // 5. xrGetVulkanGraphicsDevice2KHR
    XrVulkanGraphicsDeviceGetInfoKHR deviceGetInfo{};
    deviceGetInfo.type = XR_TYPE_VULKAN_GRAPHICS_DEVICE_GET_INFO_KHR;
    deviceGetInfo.systemId = systemId_;
    deviceGetInfo.vulkanInstance = vulkanInstance_;

    if (!checkXr(
            pfnGetVulkanGraphicsDevice2KHR(instance_, &deviceGetInfo, &physicalDevice_),
            "xrGetVulkanGraphicsDevice2KHR")) {
        return false;
    }
    std::cout << "OpenXR graphics physical device selected.\n";

    graphicsQueueFamily_ = findGraphicsQueueFamily(physicalDevice_);
    if (graphicsQueueFamily_ == UINT32_MAX) {
        std::cerr << "No graphics queue family found on OpenXR physical device.\n";
        return false;
    }

    // 6. xrCreateVulkanDeviceKHR
    const char* deviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
        VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
    };

    VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
    dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
    dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

    VkPhysicalDeviceSynchronization2Features sync2Features{};
    sync2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
    sync2Features.synchronization2 = VK_TRUE;
    sync2Features.pNext = &dynamicRenderingFeatures;

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = graphicsQueueFamily_;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkDeviceCreateInfo vkDeviceCreateInfo{};
    vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    vkDeviceCreateInfo.pNext = &sync2Features;
    vkDeviceCreateInfo.queueCreateInfoCount = 1;
    vkDeviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    vkDeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(std::size(deviceExtensions));
    vkDeviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;

    XrVulkanDeviceCreateInfoKHR xrVulkanDeviceCreateInfo{};
    xrVulkanDeviceCreateInfo.type = XR_TYPE_VULKAN_DEVICE_CREATE_INFO_KHR;
    xrVulkanDeviceCreateInfo.systemId = systemId_;
    xrVulkanDeviceCreateInfo.createFlags = 0;
    xrVulkanDeviceCreateInfo.pfnGetInstanceProcAddr = &vkGetInstanceProcAddr;
    xrVulkanDeviceCreateInfo.vulkanPhysicalDevice = physicalDevice_;
    xrVulkanDeviceCreateInfo.vulkanCreateInfo = &vkDeviceCreateInfo;
    xrVulkanDeviceCreateInfo.vulkanAllocator = nullptr;

    vkResult = VK_SUCCESS;
    if (!checkXr(
            pfnCreateVulkanDeviceKHR(instance_, &xrVulkanDeviceCreateInfo, &device_, &vkResult),
            "xrCreateVulkanDeviceKHR")) {
        return false;
    }
    if (vkResult != VK_SUCCESS) {
        std::cerr << "xrCreateVulkanDeviceKHR returned VkResult " << vkResult << ".\n";
        return false;
    }

    vkGetDeviceQueue(device_, graphicsQueueFamily_, 0, &graphicsQueue_);
    std::cout << "Vulkan device created via OpenXR (queue family " << graphicsQueueFamily_ << ").\n";
    return true;
}

bool XrContext::createSession() {
    if (instance_ == XR_NULL_HANDLE || systemId_ == XR_NULL_SYSTEM_ID ||
        vulkanInstance_ == VK_NULL_HANDLE || device_ == VK_NULL_HANDLE) {
        std::cerr << "Cannot create session: OpenXR/Vulkan not initialized.\n";
        return false;
    }

    XrGraphicsBindingVulkanKHR graphicsBinding{};
    graphicsBinding.type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR;
    graphicsBinding.instance = vulkanInstance_;
    graphicsBinding.physicalDevice = physicalDevice_;
    graphicsBinding.device = device_;
    graphicsBinding.queueFamilyIndex = graphicsQueueFamily_;
    graphicsBinding.queueIndex = 0;

    XrSessionCreateInfo sessionCreateInfo{};
    sessionCreateInfo.type = XR_TYPE_SESSION_CREATE_INFO;
    sessionCreateInfo.systemId = systemId_;
    sessionCreateInfo.next = &graphicsBinding;

    if (!checkXr(xrCreateSession(instance_, &sessionCreateInfo, &session_), "xrCreateSession")) {
        return false;
    }

    std::cout << "OpenXR session created.\n";
    return true;
}

bool XrContext::createSwapchains() { return true; }
bool XrContext::beginFrame() { return true; }
bool XrContext::endFrame() { return true; }

} // namespace recorz::xr
