#ifndef LUCID_PIPELINE_H_
#define LUCID_PIPELINE_H_

#include <vulkan/vulkan.h>

typedef struct LucidPipelineCreateInfo {
	VkPipelineLayout layout;
	const char* vertShader;
	const char* fragShader;
	VkFormat format;
} LucidPipelineCreateInfo;

VkPipeline LucidCreatePipeline(VkDevice device, const LucidPipelineCreateInfo* createInfo);

VkPipelineLayout LucidCreatePipelineLayout(VkDevice device);

#endif // LUCID_PIPELINE_H_