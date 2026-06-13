#pragma once

#include "gpu/buffer.h"
#include "gpu/vk_context.h"

namespace recorz::gpu {

struct CubeVertex {
    float px, py, pz;
    float r, g, b;
};

class CubeMesh {
public:
    static constexpr uint32_t kIndexCount = 36;

    bool create(VkContext& vk);
    void destroy(VkDevice device);

    VkBuffer vertexBuffer() const { return vertexBuffer_.handle(); }
    VkBuffer indexBuffer() const { return indexBuffer_.handle(); }

private:
    Buffer vertexBuffer_;
    Buffer indexBuffer_;
};

} // namespace recorz::gpu
