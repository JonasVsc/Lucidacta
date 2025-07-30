#ifndef LUCID_CONTEXT_H_
#define LUCID_CONTEXT_H_

#include "lucid_result.h"
#include "lucid_window.h"
#include "lucid_renderer.h"
#include "lucid_gui.h"
#include <stdbool.h>
#include <stdint.h>

#define MAX_MESHES 100

typedef struct LucidProfileInfo
{
	LARGE_INTEGER frequency;
	LARGE_INTEGER startPerformanceCounterValue, endPerformanceCounterValue;


	double startupElapsedTime;

} LucidProfileInfo;

typedef struct LucidContext_T
{
	LucidWindow_T window;
	LucidRenderer_T renderer;
	LucidProfileInfo profile;


	LucidMesh_T meshes[MAX_MESHES];
	uint32_t meshCount;


} LucidContext_T;
typedef struct LucidContext_T* LucidContext;


extern LucidContext g_lucidContext;

LucidResult LucidInit();
void LucidInitProfileTools();
void LucidShutdown();

static inline bool LucidWindowShouldClose();
static inline void LucidRequestWindowClose();
static inline void LucidBeginFrame();
static inline void LucidEndFrame();
static inline void LucidDrawMeshes();
static inline bool LucidWindowShouldClose() { return g_lucidContext->window.close; }
static inline void LucidRequestWindowClose() { g_lucidContext->window.close = true; }
/// <summary>
/// Start performance counter
/// </summary>
static inline void LucidStartPerformanceCounter();
/// <summary>
/// End performance counter
/// </summary>
/// <returns>elapsed time ( microseconds )</returns>
static inline double LucidEndPerformanceCounter();
static inline void LucidShowRendererPerformanceCounter();


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

	vkCmdBindPipeline(g_lucidContext->renderer.commandBuffers[g_lucidContext->renderer.frame], VK_PIPELINE_BIND_POINT_GRAPHICS, g_lucidContext->renderer.pipeline);
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

static inline void LucidDrawMeshes()
{
	for (uint32_t i = 0; i < g_lucidContext->meshCount; ++i)
	{
		LucidPushConstants push = { 0 };
		push.vertex_address = g_lucidContext->meshes[i].vertexAddress;

		vkCmdPushConstants(g_lucidContext->renderer.commandBuffers[g_lucidContext->renderer.frame], g_lucidContext->renderer.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(LucidPushConstants), &push);
		vkCmdBindIndexBuffer(g_lucidContext->renderer.commandBuffers[g_lucidContext->renderer.frame], g_lucidContext->meshes[i].indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(g_lucidContext->renderer.commandBuffers[g_lucidContext->renderer.frame], g_lucidContext->meshes[i].indexCount, 1, 0, 0, 0);
	}
}

inline void LucidStartPerformanceCounter()
{
	QueryPerformanceCounter(&g_lucidContext->profile.startPerformanceCounterValue);
}

inline double LucidEndPerformanceCounter()
{
	QueryPerformanceCounter(&g_lucidContext->profile.endPerformanceCounterValue);
	return (double)(g_lucidContext->profile.endPerformanceCounterValue.QuadPart - g_lucidContext->profile.startPerformanceCounterValue.QuadPart) * 1000000.0 / g_lucidContext->profile.frequency.QuadPart;

}
static inline void LucidShowRendererPerformanceCounter()
{
	ImVec2 outerSize = { 0.0f, 0.0f };
	igBegin("Profiling Tools", NULL, 0);
	if (igBeginTable("##renderer_performance_counter", 2, ImGuiTableFlags_Borders, outerSize, 1))
	{
			// TABLE HEAD // 
			igTableNextRow(0, 0);
			igTableNextColumn();
			igText("STAGE");
			igTableNextColumn();
			igText("ELAPSED TIME");

			// TABLE BODY //
			igTableNextRow(0, 0);
			igTableNextColumn();
			igText("Begin command");
			igTableNextColumn();
			igText("0");
			igTableNextRow(0, 0);
			igTableNextColumn();
			igText("Record command");
			igTableNextColumn();
			igText("0");
			igTableNextRow(0, 0);
			igTableNextColumn();
			igText("Submit command");
			igTableNextColumn();
			igText("0");
			igTableNextRow(0, 0);
			igTableNextColumn();
			igText("Present");
			igTableNextColumn();
			igText("0");
		igEndTable();
	}
	igEnd();
}


#endif // LUCID_CONTEXT_H_