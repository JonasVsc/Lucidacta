#include "lucid_gui.h"
#include "lucid_context.h"



#define IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE (8)

static void checkVkResult(VkResult err)
{
	if (err == VK_SUCCESS)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

static int ImGui_ImplWin32_CreateVkSurface(ImGuiViewport* viewport, ImU64 vk_instance, const void* vk_allocator, ImU64* out_vk_surface)
{

	VkWin32SurfaceCreateInfoKHR createInfo = { 0 };
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = (HWND)viewport->PlatformHandleRaw;
	createInfo.hinstance = GetModuleHandle(NULL);
	return (int)vkCreateWin32SurfaceKHR((VkInstance)vk_instance, &createInfo, (VkAllocationCallbacks*)vk_allocator, (VkSurfaceKHR*)out_vk_surface);
}

void LucidSetupImGui()
{
	LucidRenderer renderer = &g_lucidContext->renderer;

	igCreateContext(NULL);
	ImGuiIO* io = igGetIO_Nil(); (void)io;
	io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	igStyleColorsDark(NULL);

	ImGui_ImplVulkan_InitInfo initInfo = {
		.ApiVersion = VK_API_VERSION_1_4,
		.Instance = renderer->instance,
		.PhysicalDevice = renderer->physicalDevice,
		.Device = renderer->device,
		.QueueFamily = renderer->graphicsQueueFamily,
		.Queue = renderer->graphicsQueue,
		.MinImageCount = 2,
		.ImageCount = renderer->swapchainImageCount,
		.MSAASamples = VK_SAMPLE_COUNT_1_BIT,
		.DescriptorPoolSize = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE,
		.UseDynamicRendering = true,
		.PipelineRenderingCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats = &renderer->swapchainFormat.format
		},
		.CheckVkResultFn = checkVkResult
	};

	ImGuiPlatformIO* platform = igGetPlatformIO_Nil();
	platform->Platform_CreateVkSurface = ImGui_ImplWin32_CreateVkSurface;

	ImGui_ImplWin32_Init(g_lucidContext->window.handle);
	ImGui_ImplVulkan_Init(&initInfo);
}

void LucidGuiShutdown()
{
	vkDeviceWaitIdle(g_lucidContext->renderer.device);

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplWin32_Shutdown();
	igDestroyContext(NULL);
}
