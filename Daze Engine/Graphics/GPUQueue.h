#pragma once
#include "GraphicsContext.h"

namespace Daze {
	enum class GPUQueue_Type {
		Graphics = (1 << 0),
		Transfer = (1 << 1),
		Compute = (1 << 2),
		Sparse = (1 << 3)
	};

	class GPUQueue
	{
	public:
		GPUQueue(int familyIndex, 
				 int queueIndex,
				 GraphicsContext *c);

		~GPUQueue();
	private:
		int familyIndex, queueIndex;
		GPUQueue_Type queueType;
		VkQueue queue;
	};
}

