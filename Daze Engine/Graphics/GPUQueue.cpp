#include "stdafx.h"
#include "GPUQueue.h"

using namespace Daze::Graphics;

GPUQueue::GPUQueue(int familyIndex, int queueIndex, GraphicsContext *c)
{
	context = c;
	GPUQueue::familyIndex = familyIndex;
	GPUQueue::queueIndex = queueIndex;

	vkGetDeviceQueue(context->device, familyIndex, queueIndex, &queue);
}

void GPUQueue::SubmitBuffers(CommandBuffer *bufs, int buf_cnt)
{
	context->rendering_done_semaphore->Reset();

	VkSubmitInfo submit_info;
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = NULL;

	VkCommandBuffer *cmd_bufs = new VkCommandBuffer[buf_cnt];

	for (int i = 0; i < buf_cnt; i++)
	{
		cmd_bufs[i] = *bufs[i].buffer;
	}

	submit_info.commandBufferCount = buf_cnt;
	submit_info.pCommandBuffers = cmd_bufs;
	submit_info.pWaitSemaphores = &context->present_img_ready_semaphore->semaphore;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &context->rendering_done_semaphore->semaphore;
	submit_info.signalSemaphoreCount = 1;

	vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);

	delete[] cmd_bufs;
}


GPUQueue::~GPUQueue()
{
}
