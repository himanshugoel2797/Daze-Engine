#pragma once

#include "GPUQueue.h"
#include "GPUSemaphore.h"

namespace Daze {
	namespace Graphics {
		class GraphicsContext
		{
			friend class CommandPool;
			friend class GPUSemaphore;
			friend class GPUQueue;

		public:
			GraphicsContext(const char *appName,
				uint32_t appVersion);

			void InitializeGraphics(void *pA,
				void *pB);
			void ResizeDisplay(uint32_t width, uint32_t height);

			void HandleError(VkResult err);
			void SwapBuffers(void);

			void Free(void);
			~GraphicsContext();
		private:

			void InitializeSwapChain(uint32_t width, uint32_t height);

			void *winInfoA, *winInfoB;

			bool initialized;
			char appName[256];
			VkApplicationInfo appInfo;
			VkInstance instance;
			VkPhysicalDevice activePhysicalDevice;	//For now only deal with commanding one device
			VkDevice device;
			VkSurfaceKHR surface;
			VkSwapchainKHR swapChain;
			VkPresentModeKHR present_mode;
			GPUQueue *presentQueue;
			GPUSemaphore *present_img_semaphores, *rendering_done_semaphores;
			GPUSemaphore *present_img_ready_semaphore, *rendering_done_semaphore;
			int presentQueueIndex;
		};
	}
}

