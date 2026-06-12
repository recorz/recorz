#include "platform/xr_vulkan_bridge.h"

#include "gpu/vk_context.h"
#include "xr/xr_context.h"

#include <openxr/openxr_platform.h>

#include <algorithm>
#include <iostream>
#include <vector>

namespace recorz::platform {
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

bool XrVulkanBridge::createVulkanForOpenXR(
    const xr::XrContext& xr,
    gpu::VkContext& vk,
    const VulkanCreateInfo& createInfo) {
    if (xr.instance() == XR_NULL_HANDLE || xr.system() == XR_NULL_SYSTEM_ID) {
        std::cerr << "OpenXR instance and system must be created before Vulkan bootstrap.\n";
        return false;
    }

    const XrInstance instance = xr.instance();
    const XrSystemId systemId = xr.system();

    auto pfnGetVulkanGraphicsRequirements2KHR = loadXrFunction<PFN_xrGetVulkanGraphicsRequirements2KHR>(
        instance, "xrGetVulkanGraphicsRequirements2KHR");
    auto pfnCreateVulkanInstanceKHR = loadXrFunction<PFN_xrCreateVulkanInstanceKHR>(
        instance, "xrCreateVulkanInstanceKHR");
    auto pfnGetVulkanGraphicsDevice2KHR = loadXrFunction<PFN_xrGetVulkanGraphicsDevice2KHR>(
        instance, "xrGetVulkanGraphicsDevice2KHR");
    auto pfnCreateVulkanDeviceKHR = loadXrFunction<PFN_xrCreateVulkanDeviceKHR>(
        instance, "xrCreateVulkanDeviceKHR");

    if (!pfnGetVulkanGraphicsRequirements2KHR || !pfnCreateVulkanInstanceKHR ||
        !pfnGetVulkanGraphicsDevice2KHR || !pfnCreateVulkanDeviceKHR) {
        return false;
    }

    XrGraphicsRequirementsVulkanKHR requirements{};
    requirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR;

    if (!checkXr(
            pfnGetVulkanGraphicsRequirements2KHR(instance, systemId, &requirements),
            "xrGetVulkanGraphicsRequirements2KHR")) {
        return false;
    }
    std::cout << "Vulkan API version range: "
              << XR_VERSION_MAJOR(requirements.minApiVersionSupported) << "."
              << XR_VERSION_MINOR(requirements.minApiVersionSupported) << " - "
              << XR_VERSION_MAJOR(requirements.maxApiVersionSupported) << "."
              << XR_VERSION_MINOR(requirements.maxApiVersionSupported) << "\n";

    VkApplicationInfo vkAppInfo{};
    vkAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vkAppInfo.pApplicationName = createInfo.applicationName.c_str();
    vkAppInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    vkAppInfo.pEngineName = "Recorz";
    vkAppInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    vkAppInfo.apiVersion = clampVulkanApiVersion(
        createInfo.desiredApiVersion,
        requirements.minApiVersionSupported,
        requirements.maxApiVersionSupported);

    VkInstanceCreateInfo vkInstanceCreateInfo{};
    vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkInstanceCreateInfo.pApplicationInfo = &vkAppInfo;

    XrVulkanInstanceCreateInfoKHR xrVulkanInstanceCreateInfo{};
    xrVulkanInstanceCreateInfo.type = XR_TYPE_VULKAN_INSTANCE_CREATE_INFO_KHR;
    xrVulkanInstanceCreateInfo.systemId = systemId;
    xrVulkanInstanceCreateInfo.createFlags = 0;
    xrVulkanInstanceCreateInfo.pfnGetInstanceProcAddr = &vkGetInstanceProcAddr;
    xrVulkanInstanceCreateInfo.vulkanCreateInfo = &vkInstanceCreateInfo;
    xrVulkanInstanceCreateInfo.vulkanAllocator = nullptr;

    VkInstance vulkanInstance = VK_NULL_HANDLE;
    VkResult vkResult = VK_SUCCESS;
    if (!checkXr(
            pfnCreateVulkanInstanceKHR(
                instance, &xrVulkanInstanceCreateInfo, &vulkanInstance, &vkResult),
            "xrCreateVulkanInstanceKHR")) {
        return false;
    }
    if (vkResult != VK_SUCCESS) {
        std::cerr << "xrCreateVulkanInstanceKHR returned VkResult " << vkResult << ".\n";
        return false;
    }
    std::cout << "Vulkan instance created via OpenXR.\n";

    XrVulkanGraphicsDeviceGetInfoKHR deviceGetInfo{};
    deviceGetInfo.type = XR_TYPE_VULKAN_GRAPHICS_DEVICE_GET_INFO_KHR;
    deviceGetInfo.systemId = systemId;
    deviceGetInfo.vulkanInstance = vulkanInstance;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    if (!checkXr(
            pfnGetVulkanGraphicsDevice2KHR(instance, &deviceGetInfo, &physicalDevice),
            "xrGetVulkanGraphicsDevice2KHR")) {
        vkDestroyInstance(vulkanInstance, nullptr);
        return false;
    }
    std::cout << "OpenXR graphics physical device selected.\n";

    const uint32_t graphicsQueueFamily = findGraphicsQueueFamily(physicalDevice);
    if (graphicsQueueFamily == UINT32_MAX) {
        std::cerr << "No graphics queue family found on OpenXR physical device.\n";
        vkDestroyInstance(vulkanInstance, nullptr);
        return false;
    }

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
    queueCreateInfo.queueFamilyIndex = graphicsQueueFamily;
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
    xrVulkanDeviceCreateInfo.systemId = systemId;
    xrVulkanDeviceCreateInfo.createFlags = 0;
    xrVulkanDeviceCreateInfo.pfnGetInstanceProcAddr = &vkGetInstanceProcAddr;
    xrVulkanDeviceCreateInfo.vulkanPhysicalDevice = physicalDevice;
    xrVulkanDeviceCreateInfo.vulkanCreateInfo = &vkDeviceCreateInfo;
    xrVulkanDeviceCreateInfo.vulkanAllocator = nullptr;

    VkDevice device = VK_NULL_HANDLE;
    vkResult = VK_SUCCESS;
    if (!checkXr(
            pfnCreateVulkanDeviceKHR(instance, &xrVulkanDeviceCreateInfo, &device, &vkResult),
            "xrCreateVulkanDeviceKHR")) {
        vkDestroyInstance(vulkanInstance, nullptr);
        return false;
    }
    if (vkResult != VK_SUCCESS) {
        std::cerr << "xrCreateVulkanDeviceKHR returned VkResult " << vkResult << ".\n";
        vkDestroyInstance(vulkanInstance, nullptr);
        return false;
    }

    VkQueue graphicsQueue = VK_NULL_HANDLE;
    vkGetDeviceQueue(device, graphicsQueueFamily, 0, &graphicsQueue);
    std::cout << "Vulkan device created via OpenXR (queue family " << graphicsQueueFamily << ").\n";

    gpu::AdoptedDevice adopted{};
    adopted.instance = vulkanInstance;
    adopted.physicalDevice = physicalDevice;
    adopted.device = device;
    adopted.graphicsQueueFamily = graphicsQueueFamily;
    adopted.graphicsQueue = graphicsQueue;

    if (!vk.adopt(adopted)) {
        vkDestroyDevice(device, nullptr);
        vkDestroyInstance(vulkanInstance, nullptr);
        return false;
    }

    return true;
}

} // namespace recorz::platform
