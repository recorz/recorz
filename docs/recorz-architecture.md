# Recorz — Graphics & Platform Architecture

**Version:** 0.1  
**Date:** June 12, 2026  
**Status:** Active  
**Related:** `recorz-minimal-spec.md`, `recorz-minimal-tasks.md`

---

## 1. Core Principle: Orchestrate, Don't Merge

OpenXR and Vulkan are integrated via **XR_KHR_vulkan_enable2**. Vulkan instance and device must be created through OpenXR helpers (`xrCreateVulkanInstanceKHR`, `xrCreateVulkanDeviceKHR`), not with raw `vkCreateInstance` / `vkCreateDevice` before session creation.

Responsibilities are split across layers:

| Layer | Owns | Does Not Own |
|-------|------|--------------|
| **XrContext** | OpenXR instance, system, session, spaces, frame pacing, swapchains, composition | Vulkan objects, command buffers, pipelines |
| **VkContext** | Vulkan instance, device, queues, GPU resources (future) | Session lifecycle, compositor timing, poses |
| **GraphicsContext** | Init order, wiring, session binding | Game logic, draw calls |
| **XrVulkanBridge** | enable2 creation glue only | Long-lived state |

---

## 2. Module Layout

```
src/
├── app/                          # (future) MinimalApp, main loop
├── platform/
│   ├── graphics_context.hpp/cpp  # composition root
│   └── xr_vulkan_bridge.hpp/cpp  # enable2 bootstrap only
├── xr/
│   ├── xr_context.hpp/cpp        # OpenXR lifecycle
│   ├── xr_swapchain.hpp/cpp      # per-eye swapchain (XR side)
│   ├── xr_frame.hpp/cpp          # (planned) wait/begin/end frame
│   └── xr_space.hpp/cpp          # (planned) reference spaces, locate views
├── gpu/
│   ├── vk_context.hpp/cpp        # Vulkan device ownership
│   ├── vk_swapchain_images.hpp   # (planned) VkImageView + layout tracking
│   ├── command_ring.hpp          # (planned) per-frame command buffers
│   └── dynamic_renderer.hpp      # (planned) vkCmdBeginRendering
├── render/
│   ├── frame_packet.hpp          # (planned) POD snapshot for renderer
│   └── stereo_renderer.hpp       # (planned) records both eyes
└── math.hpp
```

Namespaces: `recorz::xr`, `recorz::gpu`, `recorz::platform`, `recorz::render`.

---

## 3. Class Relationships

```
Application (main)
    └── GraphicsContext
            ├── XrContext
            ├── VkContext
            └── XrVulkanBridge (static bootstrap)

XrVulkanBridge::createVulkanForOpenXR(xr, vk)
    → xrGetVulkanGraphicsRequirements2KHR
    → xrCreateVulkanInstanceKHR
    → xrGetVulkanGraphicsDevice2KHR
    → xrCreateVulkanDeviceKHR
    → VkContext::adopt(...)

GraphicsContext::init()
    → XrContext::createInstance
    → XrContext::selectSystem
    → XrVulkanBridge::createVulkanForOpenXR
    → XrContext::createSession(vk.graphicsBinding())
```

### XrContext (OpenXR only)

- `createInstance` — `xrCreateInstance` with `XR_KHR_vulkan_enable2`
- `selectSystem` — `xrGetSystem` for HMD form factor
- `createSession` — receives `XrGraphicsBindingVulkanKHR` from outside
- `destroySession` / `shutdown` — session and instance teardown

### VkContext (Vulkan only)

- `adopt(AdoptedDevice)` — takes ownership of enable2-created handles
- `graphicsBinding()` — builds `XrGraphicsBindingVulkanKHR` for session creation
- `shutdown` — destroys device and instance

### XrVulkanBridge

Single responsibility: run the enable2 sequence using an `XrContext` that already has instance + system. This is the **only** module that includes both OpenXR and Vulkan during initialization.

### GraphicsContext

Composition root for the minimal app. Owns `XrContext` and `VkContext`, coordinates startup/shutdown order:

1. Create OpenXR instance and system
2. Bootstrap Vulkan via bridge
3. Create session with graphics binding from `VkContext`

---

## 4. Vulkan Enable2 Initialization Flow

```
xrCreateInstance          (XR_KHR_vulkan_enable2)
    ↓
xrGetSystem               (HMD form factor)
    ↓
xrGetVulkanGraphicsRequirements2KHR
    ↓
xrCreateVulkanInstanceKHR (VkInstanceCreateInfo, API version clamped)
    ↓
xrGetVulkanGraphicsDevice2KHR
    ↓
xrCreateVulkanDeviceKHR   (dynamic rendering + sync2 extensions)
    ↓
xrCreateSession           (XrGraphicsBindingVulkanKHR)
```

**Critical rules:**

- Always call `xrGetSystem` before graphics requirements or Vulkan creation.
- Always use `xrGetVulkanGraphicsDevice2KHR` — never pick the first enumerated GPU.
- Pass a full `VkInstanceCreateInfo` (not just `VkApplicationInfo`) to `xrCreateVulkanInstanceKHR`.

---

## 5. Compositor-Driven Frame Loop (Planned)

Frame pacing is owned by **`XrFrame`** (not `XrContext`):

```
1. Poll session events (ready / stopping / loss)
2. frame = XrFrame::wait(session)           // xrWaitFrame
3. if !frame.shouldRender → skip GPU work
4. XrFrame::begin(session)                  // xrBeginFrame
5. views = XrSpace::locateViews(...)        // per-eye pose + FOV
6. Build FramePacket { views, displayTime }
7. For each eye:
     swapchain.acquire() / wait()
     renderer.renderEye(FramePacket, eye, image)
     swapchain.release()
8. XrFrame::end(session, projection layers) // xrEndFrame
```

### Performance Guidelines

- Preallocate `XrView` and layer structs — no per-frame `vector` growth.
- Use a **command ring** (2–3 frames in flight) aligned with swapchain image count.
- Pass a **`FramePacket`** POD struct to the renderer — future ECS → GPU boundary.

```cpp
struct FramePacket {
    XrTime displayTime;
    uint32_t viewCount;
    ViewData views[2];   // pose, fov, swapchain index
};
```

---

## 6. Rendering Path (Planned)

| Component | Role |
|-----------|------|
| **XrSwapchain** | XR handle, acquire/wait/release |
| **VkSwapchainImages** | `VkImage` + `VkImageView` + layout per swapchain image |
| **DynamicRenderer** | `vkCmdBeginRendering`, clears, draws |
| **StereoRenderer** | Orchestrates both eyes from one `FramePacket` |

`DynamicRenderer` must not call OpenXR APIs — keeps GPU code testable.

---

## 7. Abstractions to Introduce

| Abstraction | Purpose | Status |
|-------------|---------|--------|
| `GraphicsContext` | Composition root | **Implemented** |
| `XrVulkanBridge` | enable2 glue | **Implemented** |
| `VkContext::adopt` | Vulkan ownership | **Implemented** |
| `FramePacket` / `ViewData` | Renderer input POD | **Implemented** |
| `XrFrame` | Compositor pacing | **Implemented** |
| `XrSpace` | Poses and reference spaces | **Implemented** |
| `XrSessionRuntime` | Event polling, `xrBeginSession` | **Implemented** |
| `Result<T>` / `std::expected` | Error handling | Planned |
| `SessionStateMachine` | Event-driven session state | Planned |

---

## 8. Data-Oriented Growth Path

When ECS arrives, `xr/` and `gpu/` modules stay unchanged:

```
ECS simulation → RenderSnapshot (SoA) → FramePacket → StereoRenderer → GPU
```

`RenderSnapshot` is built on the simulation thread. `FramePacket` holds pointers + counts into that snapshot. The renderer stays stateless per frame.

---

## 9. Implementation Roadmap

### Step 1 — Extract bridge ✅

- Move enable2 code from `XrContext` → `XrVulkanBridge`
- Strip Vulkan members from `XrContext`
- Restore `VkContext` with `adopt()` and destroy logic
- Add `GraphicsContext` as composition root

### Step 2 — Split frame responsibilities ✅

- Add `xr_frame.hpp/cpp` (`wait` / `begin` / `end`)
- Add `xr_space.hpp/cpp` (`createReferenceSpace`, `locateViews`)
- Add `xr_session.hpp/cpp` (event polling, `xrBeginSession`)
- Add `render/frame_packet.h` (POD boundary for renderer)
- Remove frame stubs from `XrContext`
- Compositor-paced loop in `main.cpp`

### Step 3 — Swapchain pipeline ✅

- `xr_swapchain_group` — stereo swapchains from view config + format enumeration
- `gpu/vk_swapchain_images` — image views per swapchain image
- Acquire/wait/release hooked into frame loop

### Step 4 — Dynamic rendering clear ✅

- `CommandRing` (2 frames in flight)
- `DynamicRenderer::clearColor` into swapchain images
- `StereoRenderer` drives both eyes (blue left / red right for stereo check)
- Submit via `xrEndFrame` projection layer

### Step 5 — Cube + head pose

- `FramePacket` with view/projection matrices
- Vertex/index buffers, SPIR-V shaders, dynamic rendering pipeline
- Cube draw in `StereoRenderer`

### Step 6 — Hardening

- `SessionStateMachine` + clean shutdown on HMD removal
- `Result<T>` on all init paths
- Compile-time strippable logging

---

## 10. What Not to Build Yet

- Full ECS — use `FramePacket` as the seam instead
- Render graph / pass system — one `DynamicRenderer` is enough for the cube
- Multi-threaded command recording — single graphics thread until profiling demands it
- Heavy `IRenderDevice` abstraction — `VkContext` + thin helpers is sufficient
