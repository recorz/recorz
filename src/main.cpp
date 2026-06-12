#include "platform/graphics_bootstrap.h"
#include "platform/vr_session_loop.h"

#include <iostream>

int main() {
    std::cout << "=== Recorz Minimal: VR Clear Color Loop ===\n\n";

    recorz::platform::GraphicsBootstrap bootstrap;
    recorz::platform::BootstrapInitInfo bootstrapInfo;
    bootstrapInfo.applicationName = "Recorz Minimal";

    if (!bootstrap.init(bootstrapInfo)) {
        std::cerr << "Graphics initialization failed.\n";
        return 1;
    }

    recorz::platform::VrSessionLoopConfig loopConfig{
        .maxRenderedFrames = 3000,
        .framesToLog = 5,
    };

    int exitCode = 0;
    {
        recorz::platform::VrSessionLoop sessionLoop(bootstrap);
        exitCode = sessionLoop.run(loopConfig);
    }

    bootstrap.shutdown();
    return exitCode;
}
