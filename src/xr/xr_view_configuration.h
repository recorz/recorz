#pragma once

#include <openxr/openxr.h>

#include <cstdint>

namespace recorz::xr {

struct StereoViewConfiguration {
    static constexpr uint32_t kMaxViews = 2;

    XrViewConfigurationType viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    uint32_t count = 0;
    XrViewConfigurationView views[kMaxViews]{};

    bool isValid() const { return count > 0 && count <= kMaxViews; }
};

bool loadStereoViewConfiguration(
    XrInstance instance,
    XrSystemId system,
    StereoViewConfiguration& out);

} // namespace recorz::xr
