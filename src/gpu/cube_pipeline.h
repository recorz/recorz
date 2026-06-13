#pragma once

#include "gpu/cube_mesh.h"
#include "gpu/shader_module.h"
#include "gpu/vk_context.h"

#include <vulkan/vulkan.h>

#include <string>

namespace recorz::gpu {

class CubePipeline {
public:
    static constexpr VkDeviceSize kMvpPushConstantSize = 64;

    bool create(VkContext& vk, VkFormat colorFormat, const std::string& shaderDir);
    void destroy(VkDevice device);

    void bind(VkCommandBuffer commandBuffer) const;
    void pushMvp(VkCommandBuffer commandBuffer, const float* mvp) const;
    void draw(VkCommandBuffer commandBuffer, const CubeMesh& mesh) const;

    bool isReady() const { return pipeline_ != VK_NULL_HANDLE; }

private:
    ShaderModule vertShader_;
    ShaderModule fragShader_;
    VkPipelineLayout pipelineLayout_ = VK_NULL_HANDLE;
    VkPipeline pipeline_ = VK_NULL_HANDLE;
};

} // namespace recorz::gpu
