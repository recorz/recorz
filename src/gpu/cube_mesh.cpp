#include "gpu/cube_mesh.h"

#include <array>
#include <iostream>

namespace recorz::gpu {
namespace {

constexpr float kHalf = 0.15f;

const std::array<CubeVertex, 24> kCubeVertices = {{
    // Front (red)
    {-kHalf, -kHalf, kHalf, 0.9f, 0.2f, 0.2f},
    {kHalf, -kHalf, kHalf, 0.9f, 0.2f, 0.2f},
    {kHalf, kHalf, kHalf, 0.9f, 0.2f, 0.2f},
    {-kHalf, kHalf, kHalf, 0.9f, 0.2f, 0.2f},
    // Back (green)
    {kHalf, -kHalf, -kHalf, 0.2f, 0.85f, 0.3f},
    {-kHalf, -kHalf, -kHalf, 0.2f, 0.85f, 0.3f},
    {-kHalf, kHalf, -kHalf, 0.2f, 0.85f, 0.3f},
    {kHalf, kHalf, -kHalf, 0.2f, 0.85f, 0.3f},
    // Left (blue)
    {-kHalf, -kHalf, -kHalf, 0.2f, 0.35f, 0.95f},
    {-kHalf, -kHalf, kHalf, 0.2f, 0.35f, 0.95f},
    {-kHalf, kHalf, kHalf, 0.2f, 0.35f, 0.95f},
    {-kHalf, kHalf, -kHalf, 0.2f, 0.35f, 0.95f},
    // Right (yellow)
    {kHalf, -kHalf, kHalf, 0.95f, 0.85f, 0.2f},
    {kHalf, -kHalf, -kHalf, 0.95f, 0.85f, 0.2f},
    {kHalf, kHalf, -kHalf, 0.95f, 0.85f, 0.2f},
    {kHalf, kHalf, kHalf, 0.95f, 0.85f, 0.2f},
    // Top (cyan)
    {-kHalf, kHalf, kHalf, 0.2f, 0.85f, 0.9f},
    {kHalf, kHalf, kHalf, 0.2f, 0.85f, 0.9f},
    {kHalf, kHalf, -kHalf, 0.2f, 0.85f, 0.9f},
    {-kHalf, kHalf, -kHalf, 0.2f, 0.85f, 0.9f},
    // Bottom (magenta)
    {-kHalf, -kHalf, -kHalf, 0.85f, 0.2f, 0.85f},
    {kHalf, -kHalf, -kHalf, 0.85f, 0.2f, 0.85f},
    {kHalf, -kHalf, kHalf, 0.85f, 0.2f, 0.85f},
    {-kHalf, -kHalf, kHalf, 0.85f, 0.2f, 0.85f},
}};

const std::array<uint16_t, 36> kCubeIndices = {{
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
    8, 9, 10, 10, 11, 8,
    12, 13, 14, 14, 15, 12,
    16, 17, 18, 18, 19, 16,
    20, 21, 22, 22, 23, 20,
}};

} // namespace

bool CubeMesh::create(VkContext& vk) {
    destroy(vk.device());

    const VkDeviceSize vertexSize = sizeof(kCubeVertices);
    if (!vertexBuffer_.create(
            vk,
            vertexSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        std::cerr << "Failed to create cube vertex buffer.\n";
        return false;
    }
    if (!vertexBuffer_.upload(vk, kCubeVertices.data(), vertexSize)) {
        destroy(vk.device());
        std::cerr << "Failed to upload cube vertices.\n";
        return false;
    }

    const VkDeviceSize indexSize = sizeof(kCubeIndices);
    if (!indexBuffer_.create(
            vk,
            indexSize,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
        destroy(vk.device());
        std::cerr << "Failed to create cube index buffer.\n";
        return false;
    }
    if (!indexBuffer_.upload(vk, kCubeIndices.data(), indexSize)) {
        destroy(vk.device());
        std::cerr << "Failed to upload cube indices.\n";
        return false;
    }

    return true;
}

void CubeMesh::destroy(VkDevice device) {
    indexBuffer_.destroy(device);
    vertexBuffer_.destroy(device);
}

} // namespace recorz::gpu
