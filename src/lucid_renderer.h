#ifndef LUCID_RENDERER_H_
#define LUCID_RENDERER_H_

#include "lucid_result.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct LucidRenderer_T
{
	VkInstance instance;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue graphicsQueue;
	uint32_t graphicsQueueFamily;
	VkSwapchainKHR swapchain;
	VkSurfaceFormatKHR swapchainFormat;
	VkExtent2D swapchainExtent;
	VkImage swapchainImages[5];
	VkImageView swapchainImageViews[5];
	uint32_t swapchainImageCount;
	VkPipeline pipeline;
	VkPipelineLayout layout;
	VkCommandPool commandPool;
	VkCommandPool immediateCommandPool;
	VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
	VkCommandBuffer immediateCommandBuffer;
	VkSemaphore acquireSemaphore[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore submitSemaphore[5];
	VkFence frameFence[5];
	VkFence immediateFence;
	uint32_t imageIndex;
	uint32_t frame;

} LucidRenderer_T;
typedef struct LucidRenderer_T* LucidRenderer;

LucidResult LucidCreateRenderer();
void LucidDestroyRenderer();

#endif // LUCID_RENDERER_H_