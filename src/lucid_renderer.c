#include "lucid_renderer.h"
#include "lucid_context.h"
#include "lucid_utils.h"

static void createInstance(LucidRenderer ctx);
static void createSurface(LucidRenderer ctx);
static void selectPhysicalDevice(LucidRenderer ctx);
static void selectQueueFamilies(LucidRenderer ctx);
static void createDevice(LucidRenderer ctx);
static void createSwapchain(LucidRenderer ctx);
static void createSwapchainImageViews(LucidRenderer ctx);
static void createCommandPool(LucidRenderer ctx);
static void allocateCommandBuffers(LucidRenderer ctx);
static void createSyncObjects(LucidRenderer ctx);
// static void createPipeline(LucidRenderer ctx);

static VkPresentModeKHR selectPresentMode(LucidRenderer ctx, VkPresentModeKHR preferred_present_mode);
static VkSurfaceFormatKHR selectSurfaceFormat(LucidRenderer ctx, VkSurfaceFormatKHR preferred_surface_format);
static VkExtent2D selectSurfaceExtent(LucidRenderer ctx);


LucidResult LucidCreateRenderer()
{
	if (g_lucidContext == NULL)
		return LUCID_ERROR;

	LucidRenderer ctx = &g_lucidContext->renderer;
	ctx->imageIndex = 0;
	ctx->frame = 0;

	createInstance(ctx);
	createSurface(ctx);
	selectPhysicalDevice(ctx);
	selectQueueFamilies(ctx);
	createDevice(ctx);
	createSwapchain(ctx);
	createSwapchainImageViews(ctx);
	createCommandPool(ctx);
	allocateCommandBuffers(ctx);
	createSyncObjects(ctx);

	return LUCID_SUCCESS;
}

void LucidDestroyRenderer()
{
	LucidRenderer ctx = &g_lucidContext->renderer;

	vkDeviceWaitIdle(ctx->device);

	for (uint32_t i = 0; i < ctx->swapchainImageCount; ++i)
	{
		vkDestroyImageView(ctx->device, ctx->swapchainImageViews[i], NULL);
		vkDestroySemaphore(ctx->device, ctx->submitSemaphore[i], NULL);
		vkDestroyFence(ctx->device, ctx->frameFence[i], NULL);
	}

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(ctx->device, ctx->acquireSemaphore[i], NULL);
	}

	vkDestroyFence(ctx->device, ctx->immediateFence, NULL);
	vkDestroyPipelineLayout(ctx->device, ctx->layout, NULL);
	vkDestroyPipeline(ctx->device, ctx->pipeline, NULL);
	vkDestroyCommandPool(ctx->device, ctx->commandPool, NULL);
	vkDestroyCommandPool(ctx->device, ctx->immediateCommandPool, NULL);
	vkDestroySwapchainKHR(ctx->device, ctx->swapchain, NULL);
	vkDestroyDevice(ctx->device, NULL);
	vkDestroySurfaceKHR(ctx->instance, ctx->surface, NULL);
	vkDestroyInstance(ctx->instance, NULL);
}

static void createInstance(LucidRenderer ctx)
{
	VkApplicationInfo application_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.apiVersion = VK_API_VERSION_1_3
	};

	const char* extensions[] = {
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		VK_KHR_SURFACE_EXTENSION_NAME
	};

	const char* layers[] = {
		"VK_LAYER_KHRONOS_validation"
	};

	VkInstanceCreateInfo instance_create_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &application_info,
		.enabledExtensionCount = 2,
		.ppEnabledExtensionNames = extensions,
		.enabledLayerCount = 1,
		.ppEnabledLayerNames = layers,
	};

	vkCreateInstance(&instance_create_info, NULL, &ctx->instance);
}

void createSurface(LucidRenderer ctx)
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	if (hInstance == NULL)
		return;

	HWND hwnd = GetActiveWindow();
	if (hwnd == NULL)
		return;

	VkWin32SurfaceCreateInfoKHR surface_create_info = {
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.hinstance = hInstance,
		.hwnd = hwnd
	};

	vkCreateWin32SurfaceKHR(ctx->instance, &surface_create_info, NULL, &ctx->surface);
}

static void selectPhysicalDevice(LucidRenderer ctx)
{
	uint32_t physical_device_count = { 0 };
	vkEnumeratePhysicalDevices(ctx->instance, &physical_device_count, NULL);

	if (physical_device_count <= 0)
		return;

	VkPhysicalDevice physical_devices[10] = { 0 };
	vkEnumeratePhysicalDevices(ctx->instance, &physical_device_count, physical_devices);

	for (uint32_t i = 0; i < physical_device_count; ++i)
	{
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physical_devices[i], &properties);

		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			ctx->physicalDevice = physical_devices[i];
			return;
		}
	}

	ctx->physicalDevice = physical_devices[0];
}

static void selectQueueFamilies(LucidRenderer ctx)
{
	uint32_t queue_family_property_count = { 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(ctx->physicalDevice, &queue_family_property_count, NULL);

	VkQueueFamilyProperties queue_family_properties[20] = { 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(ctx->physicalDevice, &queue_family_property_count, queue_family_properties);

	for (uint32_t i = 0; i < queue_family_property_count; ++i)
	{
		if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			ctx->graphicsQueueFamily = i;
			return;
		}
	}
}

static void createDevice(LucidRenderer ctx)
{
	float queue_priority = 1.0f;
	VkDeviceQueueCreateInfo graphics_queue_create_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = ctx->graphicsQueueFamily,
		.queueCount = 1,
		.pQueuePriorities = &queue_priority
	};

	const char* extensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
	};

	VkPhysicalDeviceBufferDeviceAddressFeatures buffer_device_address_features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
		.bufferDeviceAddress = VK_TRUE
	};

	VkPhysicalDeviceVulkan13Features features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
		.pNext = &buffer_device_address_features,
		.dynamicRendering = VK_TRUE,
	};

	VkDeviceCreateInfo device_create_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &features,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &graphics_queue_create_info,
		.enabledExtensionCount = 2,
		.ppEnabledExtensionNames = extensions
	};

	vkCreateDevice(ctx->physicalDevice, &device_create_info, NULL, &ctx->device);
	vkGetDeviceQueue(ctx->device, ctx->graphicsQueueFamily, 0, &ctx->graphicsQueue);
}

static void createSwapchain(LucidRenderer ctx)
{
	VkSurfaceFormatKHR preffered_format = {
		.format = VK_FORMAT_B8G8R8A8_SRGB,
		.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR
	};

	ctx->swapchainExtent = selectSurfaceExtent(ctx);
	ctx->swapchainFormat = selectSurfaceFormat(ctx, preffered_format);
	VkPresentModeKHR present_mode = selectPresentMode(ctx, VK_PRESENT_MODE_FIFO_KHR);

	VkSurfaceCapabilitiesKHR capabilities = { 0 };
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx->physicalDevice, ctx->surface, &capabilities);

	uint32_t swapchain_image_count = capabilities.minImageCount + 1;
	if (swapchain_image_count > capabilities.maxImageCount)
	{
		swapchain_image_count -= 1;
	}

	VkSwapchainCreateInfoKHR swapchain_create_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = ctx->surface,
		.minImageCount = capabilities.minImageCount,
		.imageFormat = ctx->swapchainFormat.format,
		.imageColorSpace = ctx->swapchainFormat.colorSpace,
		.imageExtent = ctx->swapchainExtent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = present_mode,
		.oldSwapchain = VK_NULL_HANDLE,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE
	};

	ctx->swapchainImageCount = swapchain_image_count;

	vkCreateSwapchainKHR(ctx->device, &swapchain_create_info, NULL, &ctx->swapchain);

	vkGetSwapchainImagesKHR(ctx->device, ctx->swapchain, &ctx->swapchainImageCount, NULL);
	vkGetSwapchainImagesKHR(ctx->device, ctx->swapchain, &ctx->swapchainImageCount, ctx->swapchainImages);
}

static void createSwapchainImageViews(LucidRenderer ctx)
{
	for (uint32_t i = 0; i < ctx->swapchainImageCount; ++i)
	{
		VkImageViewCreateInfo image_view_create_info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = ctx->swapchainImages[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = ctx->swapchainFormat.format,
			.components = {
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY
			},
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		vkCreateImageView(ctx->device, &image_view_create_info, NULL, &ctx->swapchainImageViews[i]);
	}

}

static void createCommandPool(LucidRenderer ctx)
{
	VkCommandPoolCreateInfo coomand_pool_create_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = ctx->graphicsQueueFamily
	};

	vkCreateCommandPool(ctx->device, &coomand_pool_create_info, NULL, &ctx->commandPool);
	vkCreateCommandPool(ctx->device, &coomand_pool_create_info, NULL, &ctx->immediateCommandPool);
}

static void allocateCommandBuffers(LucidRenderer ctx)
{
	VkCommandBufferAllocateInfo command_buffer_alloc_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = ctx->commandPool,
		.commandBufferCount = MAX_FRAMES_IN_FLIGHT,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY
	};

	vkAllocateCommandBuffers(ctx->device, &command_buffer_alloc_info, ctx->commandBuffers);

	command_buffer_alloc_info.commandPool = ctx->immediateCommandPool;
	command_buffer_alloc_info.commandBufferCount = 1;

	vkAllocateCommandBuffers(ctx->device, &command_buffer_alloc_info, &ctx->immediateCommandBuffer);
}

static void createSyncObjects(LucidRenderer ctx)
{
	VkSemaphoreCreateInfo semaphore_create_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};

	VkFenceCreateInfo fence_create_info = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkCreateSemaphore(ctx->device, &semaphore_create_info, NULL, &ctx->acquireSemaphore[i]);
		vkCreateFence(ctx->device, &fence_create_info, NULL, &ctx->frameFence[i]);
	}

	for (uint32_t i = 0; i < ctx->swapchainImageCount; ++i)
	{
		vkCreateSemaphore(ctx->device, &semaphore_create_info, NULL, &ctx->submitSemaphore[i]);
	}

	vkCreateFence(ctx->device, &fence_create_info, NULL, &ctx->immediateFence);
}

static VkPresentModeKHR selectPresentMode(LucidRenderer ctx, VkPresentModeKHR preferred_present_mode)
{
	uint32_t present_mode_count = { 0 };
	vkGetPhysicalDeviceSurfacePresentModesKHR(ctx->physicalDevice, ctx->surface, &present_mode_count, NULL);

	VkPresentModeKHR present_modes[8] = { 0 };
	vkGetPhysicalDeviceSurfacePresentModesKHR(ctx->physicalDevice, ctx->surface, &present_mode_count, present_modes);

	for (uint32_t i = 0; i < present_mode_count; ++i)
	{
		if (present_modes[i] == preferred_present_mode)
			return preferred_present_mode;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

static VkSurfaceFormatKHR selectSurfaceFormat(LucidRenderer ctx, VkSurfaceFormatKHR preferred_surface_format)
{
	uint32_t surface_format_count = { 0 };
	vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physicalDevice, ctx->surface, &surface_format_count, NULL);

	VkSurfaceFormatKHR surface_formats[15] = { 0 };
	vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physicalDevice, ctx->surface, &surface_format_count, surface_formats);

	VkSurfaceFormatKHR default_surface_format = {
		.format = VK_FORMAT_B8G8R8A8_SRGB,
		.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
	};

	bool supports_default = false;
	for (uint32_t i = 0; i < surface_format_count; ++i)
	{
		if (surface_formats[i].format == preferred_surface_format.format &&
			surface_formats[i].colorSpace == preferred_surface_format.colorSpace)
		{
			return preferred_surface_format;
		}

		if (surface_formats[i].format == default_surface_format.format &&
			surface_formats[i].colorSpace == default_surface_format.colorSpace)
		{
			supports_default = true;
		}
	}

	if (supports_default)
		return default_surface_format;

	return surface_formats[0];
}

VkExtent2D selectSurfaceExtent(LucidRenderer ctx)
{
	VkSurfaceCapabilitiesKHR capabilities = { 0 };
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx->physicalDevice, ctx->surface, &capabilities);

	VkExtent2D extent = { 0 };
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		extent = capabilities.currentExtent;
	}
	else
	{
		HWND hwnd = GetActiveWindow();
		RECT client_rect;
		GetClientRect(hwnd, &client_rect);
		uint32_t width = client_rect.right = client_rect.left;
		uint32_t height = client_rect.bottom - client_rect.top;

		extent.width = width;
		extent.height = height;

		extent.width = clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		extent.height = clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
	}

	return extent;
}


