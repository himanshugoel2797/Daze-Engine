#pragma once
#include <mutex>

namespace Daze {
	namespace Graphics {

		enum class CommandBufferUsage {
			OneTime = (1 << 0),
			RenderPass = (1 << 1),
			Simultaneous = (1 << 2)
		};

		class CommandBuffer
		{
			friend class CommandPool;
			friend class GPUQueue;
		public:
			bool Reset(bool realloc);

			bool BeginRecording(CommandBufferUsage usage);
			bool EndRecording();

			bool IsReady(void);

			~CommandBuffer();
		private:
			bool canSelfReset, reset, isSecondary, recording;
			CommandBuffer();
			CommandBuffer(CommandPool *parent);
			CommandPool *parent;
			VkCommandBuffer *buffer;
			std::mutex lock;
		};
	}
}