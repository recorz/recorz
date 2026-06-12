#pragma once

#include <openxr/openxr.h>

#include <cstdio>
#include <iostream>
#include <string>

namespace recorz::xr {

inline bool checkXr(XrResult result, const char* operation) {
    if (result == XR_SUCCESS) {
        return true;
    }
    std::cerr << operation << " failed (XrResult " << result << ").\n";
    return false;
}

inline void copyXrString(char* destination, size_t capacity, const std::string& value) {
    if (capacity == 0) {
        return;
    }
    std::snprintf(destination, capacity, "%s", value.c_str());
}

} // namespace recorz::xr
