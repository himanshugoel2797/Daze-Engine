#pragma once
#include "GraphicsContext.h"
#include "CommandBuffer.h"

#include <mutex>
#include <memory>
#include <list>

namespace Daze {
	namespace Graphics {
		class CommandPool
		{
			friend class CommandBuffer;
		public:
			//friend class CommandBuffer;
			CommandPool(int				queueFamilyIndex,
						bool			transient,
						bool			individualReset,
						GraphicsContext *c);

			~CommandPool();

			bool
				IsReady(void);

			bool
				Reset(bool fullReset);

			void
				AllocCommandBuffer(int			   cnt,
			   					   bool			   primary,
								   CommandBuffer  **cmd_bufs);

		private:
			void
				FreeCommandBuffer(CommandBuffer *cmd_buf);

			bool initialized, individualReset;
			std::mutex lock;
			VkCommandPool pool;
			GraphicsContext *context;
			std::list<CommandBuffer*> buffers;

		};
	}
}