#include "xr/xr_context.h"
#include <iostream>

int main() {
    recorz::xr::XrContext xr;

    if (!xr.init("Recorz Minimal")) return 1;
    if (!xr.selectSystem()) return 1;
    if (!xr.createSession()) return 1;

    std::cout << "OpenXR initialized successfully!\n";
    return 0;
}