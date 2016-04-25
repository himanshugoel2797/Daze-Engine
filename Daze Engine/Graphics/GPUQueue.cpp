#include "stdafx.h"
#include "GPUQueue.h"

using namespace Daze;

GPUQueue::GPUQueue(int familyIndex, int queueIndex, GraphicsContext *c)
{
	GPUQueue::familyIndex = familyIndex;
	GPUQueue::queueIndex = queueIndex;

	vkGetDeviceQueue(c->device, familyIndex, queueIndex, &queue);
}


GPUQueue::~GPUQueue()
{
}
