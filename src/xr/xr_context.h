#pragma once

#include <openxr/openxr.h>
#include <string>
#include <vector>

namespace recorz::xr {

class XrContext {
public:
    XrContext() = default;
    ~XrContext();

    // Initialize OpenXR instance
    bool init(const std::string& appName = "Recorz Minimal");

    // Get the XR system (headset)
    bool selectSystem();

    // Create a session (graphics binding added later)
    bool createSession();

    // Basic accessors
    XrInstance getInstance() const { return instance_; }
    XrSystemId getSystemId() const { return systemId_; }
    XrSession getSession() const { return session_; }

private:
    XrInstance instance_ = XR_NULL_HANDLE;
    XrSystemId systemId_ = XR_NULL_SYSTEM_ID;
    XrSession  session_  = XR_NULL_HANDLE;

    std::vector<const char*> enabledExtensions_;
};

} // namespace recorz::xr