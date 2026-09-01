#include <vulkan/vulkan_raii.hpp>

namespace Aero {
	namespace Renderer {
		struct vkContext {
			// store appinfo in here so we can refernce it later, maybe if we need to pull something like the vulkan api version
			vk::raii::Context context;
			vk::ApplicationInfo appInfo;
			vk::raii::Instance instance = nullptr;
			vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
			vk::raii::PhysicalDevice physicalDevice = nullptr;
			vk::raii::Device device = nullptr;
			vk::raii::Queue graphicsQueue = nullptr;
			vk::raii::SurfaceKHR surface = nullptr;
		};
	}
}