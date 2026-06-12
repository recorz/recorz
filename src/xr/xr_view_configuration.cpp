#include "xr/xr_view_configuration.h"

#include "xr/xr_common.h"

#include <iostream>
#include <vector>

namespace recorz::xr {
namespace {

XrViewConfigurationType selectViewConfiguration(
    XrInstance instance,
    XrSystemId system) {
    uint32_t configCount = 0;
    if (xrEnumerateViewConfigurations(instance, system, 0, &configCount, nullptr) != XR_SUCCESS ||
        configCount == 0) {
        return XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    }

    std::vector<XrViewConfigurationType> configs(configCount);
    if (xrEnumerateViewConfigurations(instance, system, configCount, &configCount, configs.data()) !=
        XR_SUCCESS) {
        return XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    }

    for (XrViewConfigurationType config : configs) {
        if (config == XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO) {
            return config;
        }
    }

    return configs[0];
}

} // namespace

bool loadStereoViewConfiguration(
    XrInstance instance,
    XrSystemId system,
    StereoViewConfiguration& out) {
    out = {};

    if (instance == XR_NULL_HANDLE || system == XR_NULL_SYSTEM_ID) {
        std::cerr << "Cannot load view configuration: invalid OpenXR instance or system.\n";
        return false;
    }

    out.viewConfigurationType = selectViewConfiguration(instance, system);

    uint32_t viewCount = 0;
    if (!checkXr(
            xrEnumerateViewConfigurationViews(
                instance,
                system,
                out.viewConfigurationType,
                0,
                &viewCount,
                nullptr),
            "xrEnumerateViewConfigurationViews (count)")) {
        return false;
    }

    if (viewCount == 0 || viewCount > StereoViewConfiguration::kMaxViews) {
        std::cerr << "Unsupported stereo view count: " << viewCount << "\n";
        return false;
    }

    for (uint32_t i = 0; i < viewCount; ++i) {
        out.views[i].type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
    }

    if (!checkXr(
            xrEnumerateViewConfigurationViews(
                instance,
                system,
                out.viewConfigurationType,
                viewCount,
                &viewCount,
                out.views),
            "xrEnumerateViewConfigurationViews")) {
        return false;
    }

    out.count = viewCount;
    std::cout << "Stereo view configuration loaded (" << viewCount << " views).\n";
    return true;
}

} // namespace recorz::xr
