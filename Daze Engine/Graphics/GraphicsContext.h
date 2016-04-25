#pragma once

namespace Daze {
	class GraphicsContext
	{
		friend class GPUQueue;

	public:
		GraphicsContext(const char *appName,
			uint32_t appVersion);

		void InitializeGraphics(void *pA,
			void *pB);

		void HandleError(VkResult err);

		void Free(void);
		~GraphicsContext();
	private:
		bool initialized;
		char appName[256];
		VkApplicationInfo appInfo;
		VkInstance instance;
		VkPhysicalDevice activePhysicalDevice;	//For now only deal with commanding one device
		VkDevice device;
		VkSurfaceKHR surface;
		int presentQueueIndex;
	};
}

