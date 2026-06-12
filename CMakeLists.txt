cmake_minimum_required(VERSION 3.28)
project(recorz-minimal LANGUAGES CXX)

# ============================================
# Performance-oriented defaults
# ============================================
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable good optimization and debug settings
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-O0 -g3)
else()
    add_compile_options(-O3 -DNDEBUG -march=native)
endif()

# ============================================
# Source files
# ============================================
add_executable(recorz-minimal
    src/main.cpp
)

target_include_directories(recorz-minimal PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ============================================
# Future dependencies (commented for now)
# ============================================
# find_package(Vulkan REQUIRED)
# find_package(OpenXR REQUIRED)

# target_link_libraries(recorz-minimal PRIVATE
#     Vulkan::Vulkan
#     OpenXR::openxr_loader
# )

# ============================================
# Output
# ============================================
message(STATUS "Recorz Minimal configured with C++23")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
