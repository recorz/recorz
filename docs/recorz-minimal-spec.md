# Recorz Minimal — VR Cube Specification

**Version:** 0.2  
**Date:** June 11, 2026  
**Status:** Draft  
**Purpose:** Define the smallest possible implementation that produces a visible cube in VR while staying aligned with Recorz’s core architectural principles.

---

## 1. Overview

This document defines **Recorz Minimal**, a deliberately small vertical slice whose only goal is to render a simple cube in a VR headset.

The purpose of this slice is to validate the fundamental VR loop (OpenXR + Vulkan) in a way that does **not** compromise the long-term architectural goals of Recorz. It should serve as a solid foundation that can evolve into the full engine rather than becoming technical debt.

## 2. Goals

- Render a visible 3D cube in a VR headset.
- Establish a clean, compositor-driven frame loop.
- Create a minimal but **extensible** codebase structure.
- Prove that the basic OpenXR + Vulkan integration works reliably.
- Maintain alignment with Recorz’s core tenets (especially T1, T2, T3, and T7).

## 3. Non-Goals (Out of Scope)

- No ECS (will be added later)
- No physics
- No asset pipeline or loading system
- No foveation, post-processing, or advanced rendering
- No editor or tooling
- No performance budgeting beyond basic functionality
- No multi-object scenes or complex materials

## 4. Core Requirements

The implementation **must** satisfy the following:

| Requirement                  | Description                                      | Rationale              |
|-----------------------------|--------------------------------------------------|------------------------|
| **Compositor-driven loop**  | The main loop must be paced by `xrWaitFrame`     | Tenet T1               |
| **Stereo rendering**        | Must render to both eyes correctly               | Tenet T3               |
| **Head tracking**           | The cube must respond to head movement           | Basic VR requirement   |
| **Clean shutdown**          | Must handle session loss and HMD removal gracefully | Reliability         |
| **Extensibility**           | Code structure must allow gradual introduction of `rz_ecs`, `rz_gpu`, etc. | Long-term vision    |
| **No desktop assumptions**  | No reliance on desktop swapchain or vsync        | Tenet T7               |

## 5. High-Level Architecture

Even in this minimal version, we should avoid a completely throwaway structure.

**Authoritative design doc:** `recorz-architecture.md`

**Current Layering:**

```
Application (main)
    └── GraphicsContext (composition root)

Platform
    ├── XrVulkanBridge (enable2 bootstrap)
    └── GraphicsContext

XR Layer
    └── XrContext (instance, system, session, swapchains)

GPU Layer
    └── VkContext (adopts OpenXR-created Vulkan handles)

Rendering Layer (planned)
    └── StereoRenderer → DynamicRenderer
```

Vulkan is created through OpenXR (`XR_KHR_vulkan_enable2`). `VkContext` does not call `vkCreateInstance` / `vkCreateDevice` directly in the VR path.

**Key Design Decisions:**

- Use a very small number of files initially.
- Keep rendering logic separate from XR logic.
- Use a simple struct or class to represent the cube (this will later become an entity + components).
- Do **not** use a full ECS yet, but design the rendering path so it can later consume data from an ECS snapshot (`FramePacket`).
- Math types will be wrapped behind a thin abstraction to allow future replacement of GLM.

## 6. Technical Constraints

- **Language**: C++20
- **Graphics API**: Vulkan 1.3 with **dynamic rendering** (`VK_KHR_dynamic_rendering`)
- **XR API**: OpenXR 1.0+
- **Math**: GLM is allowed **temporarily**, but **must be wrapped** behind a thin abstraction layer in `math.hpp` to ease future removal and keep alignment with Recorz’s data-oriented goals.
- **No Exceptions** in the main runtime path.
- **Error Handling**: Keep minimal and explicit for now (simple checks + logging). Structure the code so it can be upgraded to the `rz::Status` / `RZ_TRY` system (ADR-0006) later with minimal refactoring.
- **Memory**: Avoid allocations inside the per-frame hot path where reasonably possible.

## 7. Implementation Phases

### Phase 1: OpenXR + Vulkan Foundation
- Initialize OpenXR instance and system
- Create Vulkan instance and device compatible with OpenXR (using dynamic rendering)
- Create session and swapchains
- Implement basic `xrWaitFrame` → acquire → submit → `xrEndFrame` loop
- Clear the screen to a solid color in VR

### Phase 2: Render a Cube
- Create vertex and index buffers for a cube
- Create basic vertex + fragment shaders
- Set up view and projection matrices using head pose (via thin math wrapper)
- Render the cube in both eyes using dynamic rendering
- Make the cube spin slowly (to verify the loop is running)

### Phase 3: Hardening
- Handle session state changes properly
- Add basic error checking and logging
- Ensure clean shutdown when removing the headset

## 8. Success Criteria

The implementation is considered complete when:

- A cube is clearly visible in the VR headset.
- The cube moves correctly with head tracking.
- The application runs at the headset’s native refresh rate without tearing or stuttering.
- Putting on and taking off the headset does not crash the application.
- The code is reasonably clean and commented.
- The structure allows future replacement of the hardcoded cube with ECS data and removal of temporary GLM usage.

## 9. File Structure (Recommended)

```
recorz-minimal/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── math.hpp                 # Thin wrapper around GLM (temporary)
│   ├── xr/
│   │   ├── xr_context.h
│   │   └── xr_context.cpp
│   ├── vulkan/
│   │   ├── vk_context.h
│   │   └── vk_context.cpp
│   ├── render/
│   │   ├── renderer.h
│   │   └── renderer.cpp
│   └── cube/
│       ├── cube_mesh.h
│       └── cube_mesh.cpp
├── shaders/
│   ├── cube.vert
│   └── cube.frag
└── README.md
```

## 10. Extensibility Notes

This minimal implementation should be designed with these future migrations in mind:

- The `Renderer` class should eventually become `rz_render` / `rz_gpu`.
- The cube data should be easy to replace with data coming from an ECS snapshot.
- The frame loop structure should map to the future phase pipeline (`WAIT` → `INPUT` → `SIM` → `EXTRACT` → `RECORD` → `LATCH`).
- Pose handling should be structured so it can later feed into a `PoseBus`.
- The math wrapper in `math.hpp` should make it straightforward to replace GLM with custom SIMD-friendly math later.

## 11. Decisions & Open Questions

### Decisions Made

- **Math Library**: Use **GLM temporarily** behind a thin wrapper in `math.hpp`. This allows faster progress on the minimal cube while keeping the option to remove it cleanly later.
- **Rendering Style**: Use **dynamic rendering** (`vkCmdBeginRendering`) from the beginning.
- **Error Handling**: Keep error handling minimal and explicit. Use simple checks with logging. The code structure must allow easy migration to the full `rz::Status` / `RZ_TRY` system defined in ADR-0006.

### Remaining Open Questions

- Should the thin math wrapper expose a SIMD-friendly interface from the start (in preparation for future optimization), or keep it simple scalar math for now?
- How should we handle shader compilation during development (pre-compiled SPIR-V vs runtime compilation with shaderc or similar)?
