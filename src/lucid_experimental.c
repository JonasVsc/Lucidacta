#include "lucid_experimental.h"
#include "lucid_context.h"

#include "lucid_vk_utils.h"

void LucidCreateTriangle()
{
	LucidMesh triangleMesh = &g_lucidContext->meshes[g_lucidContext->meshCount++];

	LucidVertex vertices[] = {
		{.position = { 0.0f, -0.5f, 0.0f }, .color = { 1.0f, 0.0f, 0.0f, 1.0f } },
		{.position = { 0.5f,  0.5f, 0.0f }, .color = { 0.0f, 1.0f, 0.0f, 1.0f } },
		{.position = {-0.5f,  0.5f, 0.0f }, .color = { 0.0f, 0.0f, 1.0f, 1.0f } },
	};

	uint32_t indices[] = {
		0, 1, 2
	};

	size_t verticesSize = sizeof(vertices);
	size_t indicesSize = sizeof(indices);

	// VERTEX BUFFER
	triangleMesh->vertexBuffer = LucidCreateBuffer(g_lucidContext->renderer.device, verticesSize, 
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
	triangleMesh->vertexMemory = LucidAllocateMemory(g_lucidContext->renderer.physicalDevice, g_lucidContext->renderer.device, triangleMesh->vertexBuffer,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT);

	VkBufferDeviceAddressInfo vertexAddressInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = triangleMesh->vertexBuffer
	};

	triangleMesh->vertexAddress = vkGetBufferDeviceAddress(g_lucidContext->renderer.device, &vertexAddressInfo);

	// INDEX BUFFER
	triangleMesh->indexBuffer = LucidCreateBuffer(g_lucidContext->renderer.device, indicesSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	triangleMesh->indexMemory = LucidAllocateMemory(g_lucidContext->renderer.physicalDevice, g_lucidContext->renderer.device, triangleMesh->indexBuffer,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);

	// STAGING BUFFER
	struct Staging {
		VkBuffer buffer;
		VkDeviceMemory memory;
	} staging = { 0 };

	staging.buffer = LucidCreateBuffer(g_lucidContext->renderer.device, verticesSize + indicesSize, 
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	staging.memory = LucidAllocateMemory(g_lucidContext->renderer.physicalDevice, g_lucidContext->renderer.device, staging.buffer,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0);

	void* data;
	vkMapMemory(g_lucidContext->renderer.device, staging.memory, 0, (VkDeviceSize)(verticesSize + indicesSize), 0, &data);
	memcpy(data, vertices, verticesSize);
	memcpy((char*)data + verticesSize, indices, indicesSize);
	vkUnmapMemory(g_lucidContext->renderer.device, staging.memory);

	// COPY BUFFER TO BUFFER
	LucidBeginCommandBuffer(g_lucidContext->renderer.device, g_lucidContext->renderer.immediateCommandBuffer, g_lucidContext->renderer.immediateFence);
	LucidCopyBufferToBuffer(g_lucidContext->renderer.immediateCommandBuffer, staging.buffer, triangleMesh->vertexBuffer, verticesSize, 0);
	LucidCopyBufferToBuffer(g_lucidContext->renderer.immediateCommandBuffer, staging.buffer, triangleMesh->indexBuffer, indicesSize, verticesSize);
	LucidSubmitCommandBuffer(g_lucidContext->renderer.device, g_lucidContext->renderer.immediateCommandBuffer, g_lucidContext->renderer.graphicsQueue, g_lucidContext->renderer.immediateFence);

	vkDestroyBuffer(g_lucidContext->renderer.device, staging.buffer, NULL);
	vkFreeMemory(g_lucidContext->renderer.device, staging.memory, NULL);

	triangleMesh->indexCount = 3;
}
