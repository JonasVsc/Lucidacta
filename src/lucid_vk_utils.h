#ifndef LUCID_VK_UTILS_H_
#define LUCID_VK_UTILS_H_

#include <vulkan/vulkan.h>

VkBuffer LucidCreateBuffer(VkDevice device, size_t size, VkBufferUsageFlags usage);

VkDeviceMemory LucidAllocateMemory(VkPhysicalDevice physical_device, VkDevice device, VkBuffer buffer, VkMemoryPropertyFlags properties, VkMemoryAllocateFlags flags);

VkResult LucidBeginCommandBuffer(VkDevice device, VkCommandBuffer cmd, VkFence fence);

VkResult LucidSubmitCommandBuffer(VkDevice device, VkCommandBuffer cmd, VkQueue queue, VkFence fence);

void LucidCopyBufferToBuffer(VkCommandBuffer cmd, VkBuffer src, VkBuffer dst, size_t size, size_t srcOffset);

VkShaderModule LucidCreateShaderModule(VkDevice device, const char* path);

#endif // LUCID_VK_UTILS_H_