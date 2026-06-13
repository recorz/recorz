cmake_minimum_required(VERSION 3.28)
project(recorz-minimal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================
# Performance / Build Settings
# ============================================
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ============================================
# Vulkan SDK (installed separately)
# ============================================
find_package(Vulkan REQUIRED)

# ============================================
# Fetch OpenXR SDK (loader + headers only)
# ============================================
include(FetchContent)

set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_CONFORMANCE_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_API_LAYERS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    openxr
    GIT_REPOSITORY https://github.com/KhronosGroup/OpenXR-SDK.git
    GIT_TAG        release-1.1.41
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(openxr)

# ============================================
# GLM (header-only math, wrapped by math.hpp)
# ============================================
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(glm)

# ============================================
# Shaders (SPIR-V via glslc)
# ============================================
find_program(GLSLC_EXECUTABLE glslc HINTS "$ENV{VULKAN_SDK}/Bin" "$ENV{VULKAN_SDK}/bin")
if(NOT GLSLC_EXECUTABLE)
    message(FATAL_ERROR "glslc not found. Install the Vulkan SDK.")
endif()

set(RECORZ_SHADER_DIR ${CMAKE_BINARY_DIR}/shaders)
string(REPLACE "\\" "/" RECORZ_SHADER_DIR "${RECORZ_SHADER_DIR}")
file(MAKE_DIRECTORY ${RECORZ_SHADER_DIR})

set(RECORZ_SHADERS
    ${CMAKE_SOURCE_DIR}/shaders/cube.vert
    ${CMAKE_SOURCE_DIR}/shaders/cube.frag
)

set(RECORZ_SHADER_SPV)
foreach(SHADER ${RECORZ_SHADERS})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SPV_OUTPUT ${RECORZ_SHADER_DIR}/${SHADER_NAME}.spv)
    add_custom_command(
        OUTPUT ${SPV_OUTPUT}
        COMMAND ${GLSLC_EXECUTABLE} ${SHADER} -o ${SPV_OUTPUT}
        DEPENDS ${SHADER}
        COMMENT "Compiling shader ${SHADER_NAME}"
    )
    list(APPEND RECORZ_SHADER_SPV ${SPV_OUTPUT})
endforeach()

add_custom_target(recorz_shaders DEPENDS ${RECORZ_SHADER_SPV})

# ============================================
# Source Files
# ============================================
add_executable(recorz-minimal
    src/main.cpp
    src/gpu/buffer.cpp
    src/gpu/command_ring.cpp
    src/gpu/cube_mesh.cpp
    src/gpu/cube_pipeline.cpp
    src/gpu/dynamic_renderer.cpp
    src/gpu/shader_module.cpp
    src/gpu/stereo_swapchain_images.cpp
    src/gpu/vk_context.cpp
    src/gpu/vk_swapchain_images.cpp
    src/platform/graphics_bootstrap.cpp
    src/platform/vr_session_loop.cpp
    src/platform/xr_vulkan_bridge.cpp
    src/render/frame_packet.cpp
    src/render/render_context.cpp
    src/render/stereo_renderer.cpp
    src/render/view_matrices.cpp
    src/xr/xr_context.cpp
    src/xr/xr_frame.cpp
    src/xr/xr_session.cpp
    src/xr/xr_view_resources.cpp
    src/xr/xr_space.cpp
    src/xr/xr_swapchain.cpp
    src/xr/xr_swapchain_group.cpp
    src/xr/xr_view_configuration.cpp
)

target_include_directories(recorz-minimal PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(recorz-minimal PRIVATE
    XR_USE_GRAPHICS_API_VULKAN
    RECORZ_SHADER_DIR="${RECORZ_SHADER_DIR}"
)

add_dependencies(recorz-minimal recorz_shaders)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND MSVC)
    # clang-cl (LLVM front-end, MSVC linker/ABI)
    target_compile_options(recorz-minimal PRIVATE
        $<$<CONFIG:Debug>:/Od>
        $<$<CONFIG:Debug>:/Zi>
        $<$<NOT:$<CONFIG:Debug>>:/O2>
        $<$<NOT:$<CONFIG:Debug>>:/clang:-march=native>
    )
elseif(MSVC)
    target_compile_options(recorz-minimal PRIVATE
        $<$<CONFIG:Debug>:/Od>
        $<$<CONFIG:Debug>:/Zi>
        $<$<NOT:$<CONFIG:Debug>>:/O2>
    )
else()
    target_compile_options(recorz-minimal PRIVATE
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Debug>:-g3>
        $<$<NOT:$<CONFIG:Debug>>:-O3>
        $<$<NOT:$<CONFIG:Debug>>:-DNDEBUG>
        $<$<NOT:$<CONFIG:Debug>>:-march=native>
    )
endif()

target_link_libraries(recorz-minimal PRIVATE
    Vulkan::Vulkan
    OpenXR::openxr_loader
    glm::glm
)

# ============================================
# Status Messages
# ============================================
message(STATUS "Recorz Minimal configured with C++23")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "OpenXR fetched from: ${openxr_SOURCE_DIR}")
