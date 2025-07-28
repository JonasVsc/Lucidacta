#ifndef LUCID_CONTEXT_H_
#define LUCID_CONTEXT_H_

#include "lucid_result.h"
#include "lucid_window.h"
#include "lucid_renderer.h"

#include <stdbool.h>

typedef struct LucidContext_T
{
	LucidWindow_T window;
	LucidRenderer_T renderer;
} LucidContext_T;
typedef struct LucidContext_T* LucidContext;

extern LucidContext g_lucidContext;

LucidResult LucidInit();
void LucidShutdown();

static inline bool LucidWindowShouldClose() { return g_lucidContext->window.close; }
static inline void LucidRequestWindowClose() { g_lucidContext->window.close = true; }


static inline void LucidBeginFrame()
{
	vkWaitForFences(g_lucidContext->renderer.device, 1, &g_lucidContext->renderer.frameFence[g_lucidContext->renderer.frame], VK_TRUE, UINT64_MAX);
	vkResetFences(g_lucidContext->renderer.device, 1, &g_lucidContext->renderer.frameFence[g_lucidContext->renderer.frame]);

	if (vkAcquireNextImageKHR(g_lucidContext->renderer.device, g_lucidContext->renderer.swapchain, UINT64_MAX, g_lucidContext->renderer.acquireSemaphore[g_lucidContext->renderer.frame], VK_NULL_HANDLE, &g_lucidContext->renderer.imageIndex) != VK_SUCCESS)
	{
		// TODO: HANDLE RESIZE/MINIMIZE
		return;
	}

	vkResetCommandBuffer(g_lucidContext->renderer.commandBuffers[g_lucidContext->renderer.frame], 0);
	VkCommandBufferBeginInfo cmd_begin = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	vkBeginCommandBuffer(g_lucidContext->renderer.commandBuffers[g_lucidContext->renderer.frame], &cmd_begin);

	VkImageMemoryBarrier image_barrier_write = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = VK_PIPELINE_STAGE_NONE,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = g_lucidContext->renderer.swapchainImages[g_lucidContext->renderer.imageIndex],
		.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
	};

	vkCmdPipelineBarrier(
		g_lucidContext->renderer.commandBuffers[g_lucidContext->renderer.frame],
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		0, NULL,
		0, NULL,
		1, &image_barrier_write
	);

	VkClearValue clear_color = { {{0.1f, 0.1f, 0.1f, 1.0f}} };

	VkRenderingAttachmentInfo color_attachment = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = g_lucidContext->renderer.swapchainImageViews[g_lucidContext->renderer.imageIndex],
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue = clear_color,
	};

	VkRect2D render_area = {
		.offset = {0},
		.extent = g_lucidContext->renderer.swapchainExtent
	};

	VkRenderingInfo render_info = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea = render_area,
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_attachment,
	};

	vkCmdBeginRendering(g_lucidContext->renderer.commandBuffers[g_lucidContext->renderer.frame], &render_info);

	VkViewport viewport = {
		.width = (float)g_lucidContext->renderer.swapchainExtent.width,
		.height = (float)g_lucidContext->renderer.swapchainExtent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};

	VkRect2D scissor = {
		.offset = {0, 0},
		.extent = g_lucidContext->renderer.swapchainExtent
	};

	vkCmdSetViewport(g_lucidContext->renderer.commandBuffers[g_lucidContext->renderer.frame], 0, 1, &viewport);
	vkCmdSetScissor(g_lucidContext->renderer.commandBuffers[g_lucidContext->renderer.frame], 0, 1, &scissor);

	// vkCmdBindPipeline(g_lucidContext->renderer.commandBuffers[g_lucidContext->renderer.frame], VK_PIPELINE_BIND_POINT_GRAPHICS, g_lucidContext->renderer.pipeline);
}

static inline void LucidEndFrame()
{
	vkCmdEndRendering(g_lucidContext->renderer.commandBuffers[g_lucidContext->renderer.frame]);

	VkImageMemoryBarrier image_barrier_present = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.dstAccessMask = 0,
		.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = g_lucidContext->renderer.swapchainImages[g_lucidContext->renderer.imageIndex],
		.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
	};

	vkCmdPipelineBarrier(
		g_lucidContext->renderer.commandBuffers[g_lucidContext->renderer.frame],
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0,
		0, NULL,
		0, NULL,
		1, &image_barrier_present
	);

	vkEndCommandBuffer(g_lucidContext->renderer.commandBuffers[g_lucidContext->renderer.frame]);

	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore wait_semaphores[] = { g_lucidContext->renderer.acquireSemaphore[g_lucidContext->renderer.frame] };
	VkSemaphore signal_semaphores[] = { g_lucidContext->renderer.submitSemaphore[g_lucidContext->renderer.frame] };

	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = wait_semaphores,
		.pWaitDstStageMask = wait_stages,
		.commandBufferCount = 1,
		.pCommandBuffers = &g_lucidContext->renderer.commandBuffers[g_lucidContext->renderer.frame],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = signal_semaphores
	};

	vkQueueSubmit(g_lucidContext->renderer.graphicsQueue, 1, &submit_info, g_lucidContext->renderer.frameFence[g_lucidContext->renderer.frame]);

	VkSwapchainKHR swapchains[] = { g_lucidContext->renderer.swapchain };

	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = signal_semaphores,
		.swapchainCount = 1,
		.pSwapchains = swapchains,
		.pImageIndices = &g_lucidContext->renderer.imageIndex
	};

	VkResult present_result = vkQueuePresentKHR(g_lucidContext->renderer.graphicsQueue, &present_info);
	if (present_result != VK_SUCCESS)
	{
		// TODO: HANDLE RESIZE/MINIMIZE
		return;
	}

	g_lucidContext->renderer.frame = (g_lucidContext->renderer.frame + 1) % MAX_FRAMES_IN_FLIGHT;
}
#endif // LUCID_CONTEXT_H_