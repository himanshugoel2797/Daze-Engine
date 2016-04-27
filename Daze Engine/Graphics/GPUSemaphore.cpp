#include "stdafx.h"
#include "GPUSemaphore.h"

using namespace Daze::Graphics;

GPUSemaphore::GPUSemaphore(GraphicsContext *c)
{
	isReady = false;
	context = c;
	semaphore = VK_NULL_HANDLE;

	VkSemaphoreCreateInfo create_info;
	create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;

	VkResult err = vkCreateSemaphore(c->device, &create_info, NULL, &semaphore);
	if (err == VK_SUCCESS)isReady = true;
}

bool GPUSemaphore::IsReady(void)
{
	return isReady;
}

void GPUSemaphore::Reset(void)
{
	if (isReady && context != NULL)
	{
		vkDestroySemaphore(context->device, semaphore, NULL);
		isReady = false;
		semaphore = VK_NULL_HANDLE;
	}

	if (!isReady && context != NULL)
	{
		VkSemaphoreCreateInfo create_info;
		create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		create_info.pNext = NULL;
		create_info.flags = 0;

		VkResult err = vkCreateSemaphore(context->device, &create_info, NULL, &semaphore);
		if (err == VK_SUCCESS)isReady = true;
	}
}

GPUSemaphore::~GPUSemaphore()
{
	if (isReady && context != NULL) 
	{
		vkDestroySemaphore(context->device, semaphore, NULL);

		isReady = false;
		context = NULL;
		semaphore = VK_NULL_HANDLE;
	}
}
