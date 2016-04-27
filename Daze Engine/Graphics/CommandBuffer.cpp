#include "stdafx.h"
#include "CommandBuffer.h"
#include "CommandPool.h"

using namespace Daze::Graphics;

CommandBuffer::CommandBuffer(CommandPool *parent)
{
	CommandBuffer::parent = parent;
	recording = false;
}

CommandBuffer::CommandBuffer()
{
	CommandBuffer::parent = NULL;
	recording = false;
}

bool CommandBuffer::IsReady(void)
{
	bool locked = lock.try_lock();
	if (locked)lock.unlock();

	return parent != NULL && buffer != NULL && locked;
}

bool CommandBuffer::Reset(bool realloc)
{
	if (parent != NULL && buffer != NULL && canSelfReset && !recording) {
		lock.lock();
		VkResult err = vkResetCommandBuffer(*buffer, realloc ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0);
		lock.unlock();

		if (err == VK_SUCCESS) {
			reset = true;
			recording = false;
			return true;
		}
	}

	return false;
}

bool CommandBuffer::BeginRecording(CommandBufferUsage usage)
{
	if (parent != NULL && buffer != NULL && !recording)
	{
		VkCommandBufferInheritanceInfo inherit_info;
		inherit_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inherit_info.pNext = NULL;

		VkCommandBufferBeginInfo beginInfo;
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = NULL;

		beginInfo.flags = 0;
		if ((int)usage & (int)CommandBufferUsage::OneTime) {
			beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		}

		if ((int)usage & (int)CommandBufferUsage::RenderPass) {
			if (!isSecondary)return false;

			//TODO implement render pass related primitives so they can be set in the inheritance data
			beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		}

		if ((int)usage & (int)CommandBufferUsage::Simultaneous) {
			beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		}


		lock.lock();
		VkResult err = vkBeginCommandBuffer(*buffer, &beginInfo);
		lock.unlock();

		if (err == VK_SUCCESS)
		{
			recording = true;
			reset = false;
			return true;
		}
	}

	return false;
}

bool CommandBuffer::EndRecording()
{
	if (parent != NULL && buffer != NULL && recording) {
		lock.lock();
		VkResult err = vkEndCommandBuffer(*buffer);
		lock.unlock();

		if (err == VK_SUCCESS)
		{
			recording = false;
			return true;
		}
	}
	return false;
}

CommandBuffer::~CommandBuffer()
{
	if (parent != NULL && buffer != NULL)
	{
		lock.lock();
		parent->FreeCommandBuffer(this);
		lock.unlock();
	}
}
