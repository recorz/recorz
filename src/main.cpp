#include "app/vr_application.h"

#include <iostream>

int main() {
    std::cout << "=== Recorz Minimal: VR Clear Color Loop ===\n\n";

    recorz::app::VrApplication app;
    recorz::app::VrAppConfig config;
    config.applicationName = "Recorz Minimal";

    if (!app.init(config)) {
        return 1;
    }

    return app.run();
}
