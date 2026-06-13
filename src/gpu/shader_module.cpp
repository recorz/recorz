#include "gpu/shader_module.h"

#include <fstream>
#include <iostream>

namespace recorz::gpu {

ShaderModule::~ShaderModule() {
    if (module_ != VK_NULL_HANDLE) {
        std::cerr << "ShaderModule destroyed without calling destroy().\n";
    }
}

std::vector<char> readBinaryFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Failed to open file: " << path << "\n";
        return {};
    }

    const std::streamsize size = file.tellg();
    if (size <= 0) {
        std::cerr << "File is empty: " << path << "\n";
        return {};
    }

    std::vector<char> buffer(static_cast<size_t>(size));
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), size);
    return buffer;
}

bool ShaderModule::loadFromFile(VkDevice device, const std::string& path) {
    destroy(device);

    const std::vector<char> code = readBinaryFile(path);
    if (code.empty()) {
        return false;
    }

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    if (vkCreateShaderModule(device, &createInfo, nullptr, &module_) != VK_SUCCESS) {
        std::cerr << "Failed to create shader module from " << path << ".\n";
        return false;
    }

    return true;
}

void ShaderModule::destroy(VkDevice device) {
    if (device != VK_NULL_HANDLE && module_ != VK_NULL_HANDLE) {
        vkDestroyShaderModule(device, module_, nullptr);
        module_ = VK_NULL_HANDLE;
    }
}

} // namespace recorz::gpu
