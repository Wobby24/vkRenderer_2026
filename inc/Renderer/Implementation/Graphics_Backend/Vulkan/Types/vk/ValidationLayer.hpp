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
		void* pUserData)

	{
		(void)pUserData;

		// Fixed: Explicitly checking if the bit is set
		if ((severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError) == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError) {
			// Double-check: Did you mean to match ERROR severity to your logger's ERROR severity? (Swapped in your original snippet)
			Aero::Utilities::Logger::instance().log(Utilities::Logger::Severity::ERROR, "validation layer: type " + std::to_string(static_cast<uint32_t>(type)) + " msg: " + pCallbackData->pMessage);
		}
		else if ((severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
			Aero::Utilities::Logger::instance().log(Utilities::Logger::Severity::WARNING, "validation layer: type " + std::to_string(static_cast<uint32_t>(type)) + " msg: " + pCallbackData->pMessage);
		}
		else {
			Aero::Utilities::Logger::instance().log(Utilities::Logger::Severity::INFO, "validation layer: type " + std::to_string(static_cast<uint32_t>(type)) + " msg: " + pCallbackData->pMessage);
		}

		return vk::False;
	}
}