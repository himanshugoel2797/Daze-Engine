#include "stdafx.h"
#include "CommandPool.h"

using namespace Daze::Graphics;

CommandPool::CommandPool(int queueFamilyIndex,
	bool transient,
	bool individualReset,
	GraphicsContext *c)
{
	initialized = false;
	CommandPool::individualReset = individualReset;

	context = c;

	VkCommandPoolCreateInfo cmd_pool_info;
	cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_info.pNext = NULL;

	cmd_pool_info.flags = 0;
	cmd_pool_info.flags |= transient ? VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : 0;
	cmd_pool_info.flags |= individualReset ? VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : 0;

	cmd_pool_info.queueFamilyIndex = queueFamilyIndex;

	VkResult err = vkCreateCommandPool(c->device, &cmd_pool_info, NULL, &pool);

	if (err == VK_SUCCESS)
	{
		initialized = true;
	}
}

bool CommandPool::IsReady(void)
{
	bool free = lock.try_lock();
	if (free)lock.unlock();

	return initialized && free;
}

bool CommandPool::Reset(bool fullReset)
{
	if (!initialized)return false;
	lock.lock();
	VkResult err = vkResetCommandPool(context->device, pool,
		fullReset ? VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT : 0);
	lock.unlock();

	if (err == VK_SUCCESS) {
		for (auto it = buffers.begin(); it != buffers.end(); ++it)
		{
			(*it)->reset = true;
		}
		return true;
	}
	else return false;
}

void CommandPool::AllocCommandBuffer(int cnt, bool primary, CommandBuffer **cmd_bufs)
{
	if (!initialized) return;


	CommandBuffer *bufs = new CommandBuffer[cnt];
	if (bufs == NULL)
	{
		cmd_bufs = NULL;
		return;
	}

	VkCommandBuffer *cmdBufs = new VkCommandBuffer[cnt];
	if (cmdBufs == NULL)
	{
		cmd_bufs = NULL;
		delete[] bufs;
		return;
	}

	lock.lock();
	VkCommandBufferAllocateInfo alloc_info;
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.pNext = NULL;

	alloc_info.commandBufferCount = cnt;
	alloc_info.commandPool = pool;
	alloc_info.level = primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;

	VkResult err = vkAllocateCommandBuffers(context->device, &alloc_info, cmdBufs);
	lock.unlock();

	if (err != VK_SUCCESS)
	{
		cmd_bufs = NULL;
		delete[] bufs;
		delete[] cmdBufs;
		return;
	}

	for (int i = 0; i < cnt; i++)
	{
		bufs[i].parent = this;
		bufs[i].buffer = &cmdBufs[i];
		bufs[i].canSelfReset = individualReset;
		bufs[i].isSecondary = !primary;

	}
	buffers.push_back(bufs);

	*cmd_bufs = bufs;
}

void CommandPool::FreeCommandBuffer(CommandBuffer *cmd_buf)
{
	if (!initialized)return;
	if (cmd_buf->buffer == NULL)return;
	lock.lock();
	vkFreeCommandBuffers(context->device, pool, 1, cmd_buf->buffer);
	lock.unlock();
	delete cmd_buf->buffer;
	cmd_buf->buffer = NULL;
}

CommandPool::~CommandPool()
{
	//Delete all the associated buffers
	if (!initialized)return;
	lock.lock();
	for (auto it = buffers.begin(); it != buffers.end(); ++it)
	{
		delete[] *it;
	}
	buffers.clear();
	vkDestroyCommandPool(context->device, pool, NULL);
	lock.unlock();

	initialized = false;
}
