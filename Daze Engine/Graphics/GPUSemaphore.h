#pragma once
#include "GraphicsContext.h"

namespace Daze {
	namespace Graphics {

		class GPUSemaphore
		{
			friend class GraphicsContext;
			friend class GPUQueue;
		public:
			GPUSemaphore(GraphicsContext *c);
			~GPUSemaphore();
			void Reset(void);
			bool IsReady(void);

		private:
			GraphicsContext *context;
			VkSemaphore semaphore;
			bool isReady;
		};
	}
}
