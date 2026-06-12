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
# Source Files
# ============================================
add_executable(recorz-minimal
    src/main.cpp
    src/gpu/command_ring.cpp
    src/gpu/dynamic_renderer.cpp
    src/gpu/vk_context.cpp
    src/gpu/vk_swapchain_images.cpp
    src/platform/graphics_context.cpp
    src/platform/xr_vulkan_bridge.cpp
    src/render/stereo_renderer.cpp
    src/xr/xr_context.cpp
    src/xr/xr_frame.cpp
    src/xr/xr_session.cpp
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
)

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
)

# ============================================
# Status Messages
# ============================================
message(STATUS "Recorz Minimal configured with C++23")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "OpenXR fetched from: ${openxr_SOURCE_DIR}")
