#pragma once
#include "GraphicsContext.h"
#include "CommandBuffer.h"

namespace Daze {
	namespace Graphics {
		enum class GPUQueue_Type {
			Graphics = (1 << 0),
			Transfer = (1 << 1),
			Compute = (1 << 2),
			Sparse = (1 << 3)
		};

		class GPUQueue
		{
			friend class GraphicsContext;
		public:
			GPUQueue(int familyIndex,
				int queueIndex,
				GraphicsContext *c);

			void SubmitBuffers(CommandBuffer *bufs, int buf_cnt);

			~GPUQueue();
		private:
			int familyIndex, queueIndex;
			GPUQueue_Type queueType;
			VkQueue queue;
			GraphicsContext *context;
		};
	}
}

