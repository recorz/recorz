#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace recorz::gpu {

class ShaderModule {
public:
    ShaderModule() = default;
    ~ShaderModule();

    ShaderModule(const ShaderModule&) = delete;
    ShaderModule& operator=(const ShaderModule&) = delete;

    bool loadFromFile(VkDevice device, const std::string& path);
    void destroy(VkDevice device);

    VkShaderModule handle() const { return module_; }

private:
    VkShaderModule module_ = VK_NULL_HANDLE;
};

std::vector<char> readBinaryFile(const std::string& path);

} // namespace recorz::gpu
