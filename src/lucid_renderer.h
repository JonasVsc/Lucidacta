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
	VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore acquireSemaphore[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore submitSemaphore[5];
	VkFence frameFence[5];
	uint32_t imageIndex;
	uint32_t frame;

	VkCommandPool immediateCommandPool;
	VkCommandBuffer immediateCommandBuffer;
	VkFence immediateFence;

} LucidRenderer_T;
typedef struct LucidRenderer_T* LucidRenderer;

typedef struct LucidVertex {
	float position[3];
	float uv_x;
	float normal[3];
	float uv_y;
	float color[4];
} LucidVertex;

typedef struct LucidPushConstants {
	float world_matrix[16];
	VkDeviceAddress vertex_address;
} LucidPushConstants;

typedef struct LucidMesh_T {
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexMemory;
	VkDeviceAddress vertexAddress;
	VkBuffer indexBuffer;
	VkDeviceMemory indexMemory;
	uint32_t indexCount;
} LucidMesh_T;
typedef struct LucidMesh_T* LucidMesh;

LucidResult LucidCreateRenderer();
void LucidDestroyRenderer();

#endif // LUCID_RENDERER_H_