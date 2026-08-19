#pragma once

#include <vector>
#include <string>
#include <vulkan/vulkan_raii.hpp>
#include <Utilities/Logger.hpp>

namespace Aero::Renderer::Impl {
	const std::vector<char const*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

#ifdef NDEBUG
	constexpr bool enableValidationLayers = false;
#else
	constexpr bool enableValidationLayers = true;
#endif

	static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
		vk::DebugUtilsMessageTypeFlagsEXT              type,
		const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
	void setupDebugMessenger(vk::raii::DebugUtilsMessengerEXT& debugMessenger, vk::raii::Instance& instance);
}