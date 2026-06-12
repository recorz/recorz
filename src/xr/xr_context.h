#pragma once

#include <vulkan/vulkan.h>
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include "xr/xr_view_configuration.h"

#include <string>

namespace recorz::xr {

class XrContext {
public:
    XrContext() = default;
    ~XrContext();

    XrContext(const XrContext&) = delete;
    XrContext& operator=(const XrContext&) = delete;

    bool createInstance(const std::string& appName = "Recorz Minimal");
    bool selectSystem(XrFormFactor formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY);
    bool createSession(const XrGraphicsBindingVulkanKHR& graphicsBinding);
    bool loadStereoViews();
    void destroySession();
    void shutdown();

    XrInstance instance() const { return instance_; }
    XrSystemId system() const { return systemId_; }
    XrSession  session() const { return session_; }
    const StereoViewConfiguration& stereoViews() const { return stereoViews_; }

private:
    XrInstance instance_ = XR_NULL_HANDLE;
    XrSystemId systemId_ = XR_NULL_SYSTEM_ID;
    XrSession  session_  = XR_NULL_HANDLE;
    StereoViewConfiguration stereoViews_{};
};

} // namespace recorz::xr
