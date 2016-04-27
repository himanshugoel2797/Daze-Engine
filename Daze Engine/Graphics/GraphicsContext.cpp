#include "stdafx.h"
#include <vector>
#include <cstdlib>

#include "GraphicsContext.h"
#include "GPUSemaphore.h"

using namespace Daze::Graphics;

GraphicsContext::GraphicsContext(const char *appName,
	uint32_t appVersion)
{
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = NULL;
	appInfo.apiVersion = VK_API_VERSION_1_0;

	strcpy_s(GraphicsContext::appName, appName);
	appInfo.pApplicationName = GraphicsContext::appName;
	appInfo.applicationVersion = appVersion;

	appInfo.pEngineName = DAZE_ENGINE_NAME;
	appInfo.engineVersion = DAZE_ENGINE_VER_COMB;

	swapChain = VK_NULL_HANDLE;
	surface = VK_NULL_HANDLE;
	device = VK_NULL_HANDLE;
	activePhysicalDevice = VK_NULL_HANDLE;
	instance = VK_NULL_HANDLE;

	initialized = false;
}

void GraphicsContext::InitializeGraphics(void *pA,
	void *pB)
{
	int enabledLayerCount = 0;
	const char* enabledLayerNames[] = {
		"VK_LAYER_GOOGLE_threading",
		"VK_LAYER_GOOGLE_unique_objects",
		"VK_LAYER_LUNARG_api_dump",
		"VK_LAYER_LUNARG_device_limits",
		"VK_LAYER_LUNARG_draw_state",
		"VK_LAYER_LUNARG_image",
		"VK_LAYER_LUNARG_mem_tracker",
		"VK_LAYER_LUNARG_object_tracker",
		"VK_LAYER_LUNARG_param_checker",
		"VK_LAYER_LUNARG_screenshot",
		"VK_LAYER_LUNARG_swapchain",
		//     "VK_LAYER_LUNARG_vktrace",
	};


	VkResult err;

	std::vector<const char*> extns;
	std::vector<const char*> layers;
	std::vector<VkDeviceQueueCreateInfo> queues;

	float double_priority[] = { 1.0f, 0.0f };
	float single_priority[] = { 1.0f };

	//Enumerate layers and add all recognized layers
	{
		uint32_t layer_cnt = 0;

		err = vkEnumerateInstanceLayerProperties(&layer_cnt, NULL);
		HandleError(err);

		if (layer_cnt > 0) {
			VkLayerProperties *layer_props = new VkLayerProperties[layer_cnt];

			err = vkEnumerateInstanceLayerProperties(&layer_cnt, layer_props);
			HandleError(err);

			for (int i = 0; i < layer_cnt; i++)
			{
				//Check for any layers we recognize and add them to the list above
				for (int j = 0; j < enabledLayerCount; j++)
				{
					if (!strcmp(enabledLayerNames[j], layer_props[i].layerName))
					{
						layers.push_back(enabledLayerNames[j]);
					}
				}
			}

			delete[] layer_props;
		}
	}

	//Enumerate extensions and add all recognized extensions
	{
		uint32_t extn_cnt = 0;

		err = vkEnumerateInstanceExtensionProperties(NULL, &extn_cnt, NULL);
		HandleError(err);

		if (extn_cnt > 0)
		{
			VkExtensionProperties *extn_props = new VkExtensionProperties[extn_cnt];
			bool surface_ext_found = false;
			bool platform_surface_ext_found = false;
			bool swapchain_ext_found = false;

			err = vkEnumerateInstanceExtensionProperties(NULL, &extn_cnt, extn_props);
			HandleError(err);

			for (int i = 0; i < extn_cnt; i++)
			{
				//Search for the surface extensions
				if (!strcmp(extn_props[i].extensionName, VK_KHR_SURFACE_EXTENSION_NAME))
				{
					surface_ext_found = true;
					extns.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
				}

				//Search for the swapchain extension
				if (!strcmp(extn_props[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
				{
					swapchain_ext_found = true;
					extns.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
				}

#if defined(_WIN32)
				if (!strcmp(extn_props[i].extensionName, VK_KHR_WIN32_SURFACE_EXTENSION_NAME))
				{
					platform_surface_ext_found = true;
					extns.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
				}
#else
				if (!stcmp(extn_props[i].extensionName, VK_KHR_XCB_SURFACE_EXTENSION_NAME))
				{
					platform_surface_ext_found = true;
					extns.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
				}
#endif
			}

			delete[] extn_props;

			if (!surface_ext_found | !platform_surface_ext_found | !swapchain_ext_found)
			{
				fprintf(stderr, "Required surface extension for this platform not found! \nMake sure Vulkan is supported.");
			}
		}
	}

	//Setup the instance creation info
	VkInstanceCreateInfo inst_create_info;
	inst_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_create_info.pNext = NULL;
	inst_create_info.pApplicationInfo = &appInfo;
	inst_create_info.flags = 0;
	inst_create_info.enabledExtensionCount = extns.size();
	inst_create_info.ppEnabledExtensionNames = extns.data();
	inst_create_info.enabledLayerCount = layers.size();
	inst_create_info.ppEnabledLayerNames = layers.data();

	//Create the instance
	err = vkCreateInstance(&inst_create_info, NULL, &instance);
	HandleError(err);


	//Enumerate devices and pick
	{
		uint32_t gpu_cnt = 0;

		err = vkEnumeratePhysicalDevices(instance, &gpu_cnt, NULL);
		HandleError(err);

		if (gpu_cnt > 0)
		{
			VkPhysicalDevice *gpu_devices = new VkPhysicalDevice[gpu_cnt];
			err = vkEnumeratePhysicalDevices(instance, &gpu_cnt, gpu_devices);
			HandleError(err);

			activePhysicalDevice = gpu_devices[0];

			VkPhysicalDeviceProperties default_device_props;
			vkGetPhysicalDeviceProperties(activePhysicalDevice, &default_device_props);

			//Calculate a score for each device and select the one with the highest score
			auto score_calc = [](const VkPhysicalDeviceProperties *props) {
				int score = 0;

				if (props->deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
					score += 2;
				else if (props->deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
					score += 1;

				//Get queue family properties, update score based on how many queues are supported

				return score;
			};

			int device_score = score_calc(&default_device_props);

			for (int i = 0; i < gpu_cnt; i++)
			{
				VkPhysicalDeviceProperties cur_device_props;
				vkGetPhysicalDeviceProperties(gpu_devices[i], &cur_device_props);

				//Pick a gpu
				int cur_score = score_calc(&cur_device_props);
				if (cur_score > device_score)
				{
					device_score = cur_score;
					activePhysicalDevice = gpu_devices[i];
				}
			}

			//Retrieve device properties again to account for any changes from above and store limits
			vkGetPhysicalDeviceProperties(activePhysicalDevice, &default_device_props);

			//TODO: Store device limits

			//Determine which queues to use and store their creation info
			{
				uint32_t queue_family_cnt = 0;
				vkGetPhysicalDeviceQueueFamilyProperties(activePhysicalDevice, &queue_family_cnt, NULL);

				VkQueueFamilyProperties *queue_family_props = new VkQueueFamilyProperties[queue_family_cnt];
				vkGetPhysicalDeviceQueueFamilyProperties(activePhysicalDevice, &queue_family_cnt, queue_family_props);

				for (int j = 0; j < queue_family_cnt; j++)
				{
					//Ensure the family has queues and supports presenting
					bool canPresent = false;
#if defined(_WIN32)
					canPresent = vkGetPhysicalDeviceWin32PresentationSupportKHR(activePhysicalDevice, j);
#else
#error Implement check for presentation support
					canPresent = vkGetPhysicalDeviceXcbPresentaitonSupportKHR(activePhysicalDevice, j);
#endif

					if (queue_family_props[j].queueCount > 0) {

						//TODO: Make the system select based on queue type
						VkDeviceQueueCreateInfo create_info;

						create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
						create_info.pNext = NULL;
						create_info.flags = 0;
						create_info.queueFamilyIndex = j;
						create_info.queueCount = queue_family_props[j].queueCount > 2 ? 2 : 1;
						create_info.pQueuePriorities = queue_family_props[j].queueCount > 2 ? double_priority : single_priority;

						if (canPresent && queue_family_props[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
							presentQueueIndex = j;

						queues.push_back(create_info);
					}
				}

				delete[] queue_family_props;
			}

			delete[] gpu_devices;
		}
		else {
			fprintf(stderr, "No supported devices detected.\n");
			abort();
		}
	}

	//Declare any features we want from the device here
	VkPhysicalDeviceFeatures device_features;
	device_features.multiDrawIndirect = true;
	device_features.tessellationShader = true;


	//Now we're ready to create the logical device
	VkDeviceCreateInfo device_info;
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.pNext = NULL;
	device_info.flags = 0;

	device_info.enabledLayerCount = 0;
	device_info.ppEnabledLayerNames = NULL;
	device_info.enabledExtensionCount = 0;
	device_info.ppEnabledExtensionNames = NULL;

	device_info.pEnabledFeatures = &device_features;
	device_info.pQueueCreateInfos = queues.data();
	device_info.queueCreateInfoCount = queues.size();

	//Create the device
	err = vkCreateDevice(activePhysicalDevice, &device_info, NULL, &device);
	HandleError(err);
	initialized = true;	//The device has been created


						//Now to determine which surface format to proceed with
#if defined(_WIN32)
	VkWin32SurfaceCreateInfoKHR surface_create_info;
	surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surface_create_info.pNext = NULL;
	surface_create_info.hinstance = *(HINSTANCE*)pA;
	surface_create_info.hwnd = *(HWND*)pB;

	err = vkCreateWin32SurfaceKHR(instance, &surface_create_info, NULL, &surface);
#else
						//TODO add code for creating a linux or android surface
#endif
	HandleError(err);

	present_img_ready_semaphore = new GPUSemaphore(this);
	rendering_done_semaphore = new GPUSemaphore(this);
	presentQueue = new GPUQueue(presentQueueIndex, 0, this);

	if (present_img_ready_semaphore == NULL | rendering_done_semaphore == NULL | presentQueue == NULL) {
		fprintf(stderr, "Memory allocation error!\n");
		abort();
	}


	InitializeSwapChain(960, 540);
}

// Handle general Vulkan errors
void GraphicsContext::HandleError(VkResult err)
{
	switch (err)
	{
	case VK_SUCCESS:
		return;
	case VK_ERROR_DEVICE_LOST:
		fprintf(stderr, "An unrecoverable error has occured. Exiting...\n");
		//TODO: Free all resources
		abort();
		break;
	default:
		fprintf(stderr, "Unknown Error code: %d\n", err);
	}
}

void GraphicsContext::Free(void)
{
	if (initialized)
	{
		if (surface != VK_NULL_HANDLE)
		{
			vkDestroySurfaceKHR(instance, surface, NULL);
			surface = VK_NULL_HANDLE;
		}

		if (swapChain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(device, swapChain, NULL);
			swapChain = VK_NULL_HANDLE;
		}

		vkDestroyInstance(instance, NULL);
		initialized = false;
	}
}

GraphicsContext::~GraphicsContext()
{
	//Ensure all pointers have been freed
	Free();
}

void GraphicsContext::ResizeDisplay(uint32_t width, uint32_t height)
{
	InitializeSwapChain(width, height);
}

void GraphicsContext::InitializeSwapChain(uint32_t width, uint32_t height)
{
	VkResult err;
	VkFormat color_fmt;
	VkColorSpaceKHR color_space;
	//Select a surface format and space
	{
		uint32_t fmt_cnt = 0;
		err = vkGetPhysicalDeviceSurfaceFormatsKHR(activePhysicalDevice, surface, &fmt_cnt, NULL);
		HandleError(err);

		VkSurfaceFormatKHR *fmt_info = new VkSurfaceFormatKHR[fmt_cnt];
		err = vkGetPhysicalDeviceSurfaceFormatsKHR(activePhysicalDevice, surface, &fmt_cnt, fmt_info);
		HandleError(err);

		if (fmt_cnt == 1 && fmt_info[0].format == VK_FORMAT_UNDEFINED)
		{
			color_fmt = VK_FORMAT_B8G8R8A8_SRGB;
		}
		else {
			if (fmt_cnt < 1) {
				fprintf(stderr, "Unable to retrieve surface format info.");
				abort();
			}

			color_fmt = fmt_info[0].format;
		}
		color_space = fmt_info[0].colorSpace;

		delete[] fmt_info;
	}

	{
		uint32_t present_mode_cnt = 0;
		err = vkGetPhysicalDeviceSurfacePresentModesKHR(activePhysicalDevice, surface, &present_mode_cnt, NULL);
		HandleError(err);

		present_mode = VK_PRESENT_MODE_FIFO_KHR;

		if (present_mode_cnt > 0) {
			VkPresentModeKHR *present_modes = new VkPresentModeKHR[present_mode_cnt];

			vkGetPhysicalDeviceSurfacePresentModesKHR(activePhysicalDevice, surface, &present_mode_cnt, present_modes);
			HandleError(err);

			for (int i = 0; i < present_mode_cnt; i++)
			{
				if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
				}
			}

			delete[] present_modes;
		}
	}

	//Start building the swap chain
	VkSurfaceCapabilitiesKHR surface_cap;
	err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(activePhysicalDevice, surface, &surface_cap);
	HandleError(err);

	VkExtent2D swapchain_extent;
	if (surface_cap.currentExtent.width == -1)
	{
		swapchain_extent = { width, height };
		if (swapchain_extent.width < surface_cap.minImageExtent.width)
			swapchain_extent.width = surface_cap.maxImageExtent.width;

		if (swapchain_extent.height < surface_cap.minImageExtent.height)
			swapchain_extent.height = surface_cap.minImageExtent.height;

		if (swapchain_extent.width > surface_cap.maxImageExtent.width)
			swapchain_extent.width = surface_cap.maxImageExtent.width;

		if (swapchain_extent.height > surface_cap.maxImageExtent.height)
			swapchain_extent.height = surface_cap.maxImageExtent.height;
	}
	else {
		swapchain_extent.width = surface_cap.currentExtent.width;
		swapchain_extent.height = surface_cap.currentExtent.height;
	}


	//Create the swapchain
	VkSwapchainKHR swap_chain;
	{
		VkSwapchainCreateInfoKHR swapChain_create_info;
		swapChain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapChain_create_info.pNext = NULL;

		swapChain_create_info.oldSwapchain = swapChain;
		swapChain_create_info.flags = 0;
		swapChain_create_info.imageArrayLayers = 1;
		swapChain_create_info.presentMode = present_mode;
		swapChain_create_info.surface = surface;
		swapChain_create_info.imageExtent = swapchain_extent;
		swapChain_create_info.imageFormat = color_fmt;
		swapChain_create_info.imageColorSpace = color_space;
		swapChain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapChain_create_info.clipped = VK_TRUE;
		swapChain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapChain_create_info.queueFamilyIndexCount = 0;
		swapChain_create_info.pQueueFamilyIndices = NULL;

		swapChain_create_info.preTransform = surface_cap.currentTransform;
		if (surface_cap.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
			swapChain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

		swapChain_create_info.minImageCount = surface_cap.minImageCount + 1;
		if (swapChain_create_info.minImageCount < 3)
			swapChain_create_info.minImageCount = 3;

		if (swapChain_create_info.minImageCount > surface_cap.maxImageCount)
			swapChain_create_info.minImageCount = surface_cap.maxImageCount;

		swapChain_create_info.imageUsage = 0;

		if (surface_cap.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
			swapChain_create_info.imageUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		if (surface_cap.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			swapChain_create_info.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;


		err = vkCreateSwapchainKHR(device, &swapChain_create_info, NULL, &swap_chain);
		HandleError(err);
	}

	if (swapChain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(device, swapChain, NULL);
	}
	swapChain = swap_chain;
}

void GraphicsContext::SwapBuffers(void)
{
	if (!initialized)return;

	uint32_t image_index = 0;
	
	VkPresentInfoKHR present_info;
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.pNext = NULL;
	present_info.pImageIndices = &image_index;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &swapChain;
	present_info.waitSemaphoreCount = 0;
	present_info.pWaitSemaphores = &rendering_done_semaphore->semaphore;

	vkQueuePresentKHR(presentQueue->queue, &present_info);


	//Create a new semaphore
	present_img_ready_semaphore->Reset();
	if (!present_img_ready_semaphore->IsReady()) {
		fprintf(stderr, "Failed to create a semaphore for presentation.\n");
		abort();
	}

	VkResult err = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, present_img_ready_semaphore->semaphore, VK_NULL_HANDLE, &image_index);
	
	switch (err)
	{
	case VK_SUCCESS:
	case VK_SUBOPTIMAL_KHR:
		break;
	case VK_ERROR_OUT_OF_DATE_KHR:
		InitializeSwapChain(960, 540);
		break;
	case VK_ERROR_INITIALIZATION_FAILED:
		fprintf(stderr, "Failed to retrieve next frame!\n");
		abort();
		break;
	default:
		fprintf(stderr, "Error %d\n", err);
		break;
	}




}