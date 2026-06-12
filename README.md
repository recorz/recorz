# Recorz Minimal

**Status:** Early Development  
**Goal:** Build the smallest possible high-performance VR application that renders a cube.

This is the first concrete milestone toward **Recorz** — a dedicated, maximum-performance VR game engine.

## Current Objective

Render a simple spinning cube in a VR headset using:

- OpenXR
- Vulkan 1.3 with dynamic rendering
- C++23
- Clean, extensible architecture

## Project Goals (Performance First)

- Lowest possible motion-to-photon latency
- Clean compositor-driven frame loop (`xrWaitFrame`)
- Modern C++ with minimal overhead in hot paths
- Structure designed to scale into a full data-oriented VR engine

## Build Instructions

### Requirements

- CMake 3.28+
- C++23 compatible compiler (clang 17+ or GCC 13+ recommended)
- Vulkan SDK
- OpenXR SDK / runtime (e.g. SteamVR, Oculus, or Monado)

### Build Steps

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j
```

### Run

```bash
./recorz-minimal
```

> **Note:** Currently this only prints a message. OpenXR + Vulkan integration is in progress.

## Project Structure

```
recorz-minimal/
├── CMakeLists.txt
├── README.md
├── .gitignore
├── src/
│   ├── main.cpp
│   └── math.hpp          # Temporary GLM wrapper
├── shaders/              # GLSL shaders
└── build/                # Build output (gitignored)
```

## Development Philosophy

- **Performance first**: Decisions are made to minimize latency and maximize frame consistency.
- **Extensible by design**: Code is structured so it can evolve into the full Recorz engine.
- **Modern C++**: Using C++23 features where they provide clear value.
- **Dynamic rendering**: Using `vkCmdBeginRendering` from the start.

## Current Status

- [x] Project setup with C++23
- [x] Basic CMake configuration
- [x] Math wrapper skeleton (`math.hpp`)
- [ ] OpenXR initialization
- [ ] Vulkan context with dynamic rendering
- [ ] Render a cube in VR

## Next Steps

See `docs/recorz-minimal-tasks.md` for the detailed task list.

## License

MIT License
