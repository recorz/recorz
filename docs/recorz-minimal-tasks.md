# Recorz Minimal — Implementation Task List

**Version:** 0.3  
**Date:** June 11, 2026  
**Related Spec:** `recorz-minimal-spec.md`

**Key Decisions (Performance-First):**
- Use **C++23**
- Use **dynamic rendering** from the start
- Use **GLM temporarily** behind a thin wrapper in `math.hpp`
- Keep error handling minimal but designed to evolve into a high-performance `std::expected`-based system
- Prioritize low latency and high frame consistency

---

## Phase 1: Foundation (OpenXR + Vulkan Loop)

### Task 1.1: Project Setup (Completed)
- Create a CMake project targeting **C++23**.
- Set up the recommended folder structure.
- Create a basic `main.cpp`.
- Add a placeholder `math.hpp` (thin wrapper around GLM).
- Configure CMake with good defaults for performance and modern tooling.

**Done when:** The project configures and builds successfully with C++23.

### Task 1.2: OpenXR Context (Basic Initialization) (completed)
- Create `xr/xr_context.h` and `xr_context.cpp`.
- Implement OpenXR instance creation.
- Enumerate and select a system (headset).
- Create a basic session.

**Done when:** OpenXR instance and system are successfully created.

### Task 1.3: Vulkan Context (with Dynamic Rendering)
- Create `vulkan/vk_context.h` and `vk_context.cpp`.
- Create a Vulkan 1.3 instance and device with **dynamic rendering** support.
- Ensure compatibility with OpenXR graphics requirements.

**Done when:** A Vulkan device supporting dynamic rendering is created successfully.

### Task 1.4: OpenXR Swapchain Creation
- Create stereo swapchains using OpenXR.
- Implement image acquisition and release.

**Done when:** Swapchains are functional.

### Task 1.5: Basic Compositor-Paced Loop
- Implement the main loop driven by `xrWaitFrame`.
- Submit frames to the compositor (initially just clearing the screen using dynamic rendering).

**Done when:** A solid color view renders correctly in the VR headset at the native refresh rate.

---

## Phase 2: Render a Cube

### Task 2.1: Math Wrapper
- Implement `math.hpp` as a thin, clean wrapper around GLM.
- Expose only the types and functions needed (Vec3, Mat4, perspective, lookAt, etc.).

**Done when:** Math operations can be used without directly depending on GLM in most files.

### Task 2.2: Cube Mesh Data
- Define vertex and index data for a simple cube.

**Done when:** Cube geometry is ready to be uploaded to the GPU.

### Task 2.3: Shaders
- Write basic vertex and fragment shaders for the cube.
- Compile to SPIR-V.

**Done when:** Shaders are ready.

### Task 2.4: Renderer with Dynamic Rendering
- Implement a minimal renderer using `vkCmdBeginRendering`.
- Create vertex/index buffers and pipeline.
- Support pushing MVP matrices.

**Done when:** The renderer can draw the cube using dynamic rendering.

### Task 2.5: Head Pose Integration
- Read head pose from OpenXR.
- Compute view + projection matrices (using the math wrapper).
- Render the cube correctly in world space.

**Done when:** The cube appears in front of the player and tracks head movement.

### Task 2.6: Full Frame Submission
- Integrate rendering into the OpenXR frame loop.
- Submit the rendered image as a composition layer.

**Done when:** A cube is visible in VR.

### Task 2.7: Basic Animation
- Add simple rotation to the cube to verify the loop is running smoothly.

**Done when:** The cube spins smoothly at stable frame rate.

---

## Phase 3: Hardening

### Task 3.1: Session State Handling
- Properly manage OpenXR session states (ready, stopping, loss, etc.).

**Done when:** The app handles headset removal/reconnection gracefully.

### Task 3.2: Minimal Error Handling & Logging
- Add basic error checking and logging.
- Keep it explicit and easy to evolve into a `std::expected`-based system later.

**Done when:** Errors are visible and the app doesn’t silently fail.

### Task 3.3: Clean Shutdown
- Ensure all resources are properly released.

**Done when:** The application exits cleanly.

### Task 3.4: Documentation
- Update `README.md` and add helpful comments.

**Done when:** The project is understandable and buildable by others (or AI agents).

---

## Overall Goal

Build the smallest possible VR application that renders a cube with:
- Very low latency
- Clean compositor-driven loop
- Modern C++ (C++23 + dynamic rendering)
- A structure that can scale toward a high-performance dedicated VR engine

---

**Next Action:** Start with **Task 1.1**.
