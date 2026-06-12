# Recorz Minimal — Implementation Task List

**Version:** 0.2  
**Date:** June 11, 2026  
**Related Spec:** `recorz-minimal-spec.md`

This task list breaks down the Recorz Minimal VR Cube into actionable, agent-friendly tasks. Tasks are ordered for logical implementation and early validation.

**Key Decisions Applied:**
- Use **dynamic rendering** from the start.
- Use **GLM temporarily** behind a thin wrapper in `math.hpp`.
- Keep **error handling minimal** but structured for easy upgrade to `rz::Status` / `RZ_TRY` later.

---

## Phase 1: Foundation (OpenXR + Vulkan Loop)

### Task 1.1: Project Setup
- Create CMake project with C++20 standard.
- Add dependencies: Vulkan SDK headers, OpenXR loader.
- Set up basic folder structure as defined in the spec (include `math.hpp` placeholder).
- Create a minimal `main.cpp` that prints "Hello VR".

**Done when:** Project compiles and runs a basic console application.

### Task 1.2: OpenXR Context (Basic Initialization)
- Create `xr/xr_context.h` and `xr_context.cpp`.
- Implement OpenXR instance creation.
- Enumerate and select a system (headset).
- Create a basic session (graphics binding will come later).

**Done when:** OpenXR instance and system are successfully created without errors.

### Task 1.3: Vulkan Context (Compatible with OpenXR + Dynamic Rendering)
- Create `vulkan/vk_context.h` and `vk_context.cpp`.
- Create Vulkan instance with required extensions for OpenXR **and dynamic rendering**.
- Create logical device and queue.
- Ensure the device is compatible with the OpenXR graphics requirements.

**Done when:** Vulkan device supporting dynamic rendering is created successfully.

### Task 1.4: OpenXR Swapchain Creation
- Extend `XrContext` to create session with Vulkan graphics binding.
- Create stereo swapchains (one per eye) using `xrCreateSwapchain`.
- Implement swapchain image acquisition.

**Done when:** Swapchains are created and images can be acquired.

### Task 1.5: Basic Compositor-Paced Loop
- Implement the main loop using `xrWaitFrame`, `xrBeginFrame`, and `xrEndFrame`.
- Submit an empty frame (or simple clear using dynamic rendering) to the compositor.
- Handle basic session state (e.g., `XR_SESSION_STATE_FOCUSED`).

**Done when:** A blank (or solid color) view appears in the VR headset and updates at the correct refresh rate.

---

## Phase 2: Render a Cube

### Task 2.1: Math Wrapper (Temporary GLM)
- Create `math.hpp` as a thin wrapper around GLM.
- Define basic types: `Vec3`, `Mat4`, `Quat` (or at least the ones needed for view/projection matrices).
- Implement or expose: perspective matrix, look-at/view matrix, and basic multiplication.

**Done when:** Math types can be used cleanly without directly including GLM everywhere.

### Task 2.2: Cube Mesh Data
- Create `cube/cube_mesh.h` and `cube_mesh.cpp`.
- Define hardcoded vertex positions, colors (or UVs), and indices for a unit cube.
- Make the data easy to upload to Vulkan buffers later.

**Done when:** Cube geometry is defined cleanly in code.

### Task 2.3: Basic Shaders
- Create `shaders/cube.vert` and `shaders/cube.frag`.
- Vertex shader should accept position and output transformed position.
- Fragment shader should output a solid color (e.g., white or colored faces).
- Compile shaders to SPIR-V.

**Done when:** Shaders compile successfully.

### Task 2.4: Vulkan Renderer with Dynamic Rendering
- In `render/renderer.h` and `renderer.cpp`, set up:
  - Vertex and index buffers for the cube.
  - Push constants or descriptor sets for MVP matrix.
  - Graphics pipeline compatible with dynamic rendering.
- Implement command buffer recording using `vkCmdBeginRendering` / `vkCmdEndRendering` to draw the cube.

**Done when:** The renderer can record commands to draw the cube using dynamic rendering.

### Task 2.5: Integrate Head Pose
- In the main loop, call `xrLocateViews` to get head pose.
- Compute view and projection matrices (using the math wrapper).
- Pass the matrices to the renderer (via push constants).

**Done when:** The cube appears in world space and moves correctly when you move your head.

### Task 2.6: Submit Rendered Image to OpenXR
- Modify the frame loop to:
  1. Wait for frame (`xrWaitFrame`)
  2. Acquire swapchain image
  3. Record and submit Vulkan commands (using dynamic rendering) to render the cube
  4. Release swapchain image
  5. Submit composition layer(s) via `xrEndFrame`

**Done when:** A cube is visible in the VR headset.

### Task 2.7: Add Simple Animation
- Make the cube slowly rotate on the Y axis.
- Verify that the animation is smooth and the frame rate is stable.

**Done when:** The cube spins smoothly in VR.

---

## Phase 3: Hardening & Polish

### Task 3.1: Session State Management
- Properly handle `XR_SESSION_STATE_READY`, `STOPPING`, `LOSS_PENDING`, etc.
- Implement clean session begin/end logic.

**Done when:** Putting on and taking off the headset does not crash the app.

### Task 3.2: Basic Error Handling & Logging
- Add simple logging (e.g. using `fmt` or `spdlog`).
- Add basic explicit error checking around OpenXR and Vulkan calls.
- Keep checks simple and upgradeable to future `rz::Status` system.

**Done when:** Errors are reported clearly instead of silent failures or crashes.

### Task 3.3: Clean Shutdown
- Ensure all resources (Vulkan, OpenXR, swapchains) are properly destroyed.
- Test repeated launch/quit cycles.

**Done when:** Application exits cleanly without validation errors or crashes.

### Task 3.4: Documentation
- Update `README.md` with build instructions and how to run.
- Add comments in key files explaining the flow, especially around the math wrapper and dynamic rendering.

**Done when:** Another developer (or AI) can build and run the project with minimal friction.

---

## Stretch / Future-Proofing Tasks (Optional)

- Make the math wrapper interface more SIMD-friendly in preparation for later optimization.
- Begin separating "application logic" from "rendering logic" more clearly.
- Add a simple way to change cube properties via code (preparation for future editor integration).
- Document how the current renderer could later consume data from an ECS snapshot.

---

## Overall Completion Criteria

The project is complete when **all tasks in Phase 1 + Phase 2 + Phase 3** are done and the success criteria from the spec are met:

- A spinning cube is visible in VR.
- Head tracking works.
- The application is stable when using/removing the headset.
- Dynamic rendering is in use.
- GLM is isolated behind a wrapper.
- Error handling is minimal but clean and upgradeable.
- The code structure supports future evolution toward full Recorz.

---

**Suggested Workflow for AI Agent + Human**

1. Human reviews and approves tasks (especially math wrapper approach).
2. Agent implements one task (or small logical group) at a time.
3. After key tasks (especially 2.4–2.6), test in VR.
4. Human reviews code quality and alignment with spec before advancing phases.
5. Update task status (`[ ]` → `[x]`) as work progresses.
