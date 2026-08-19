#include <Renderer/Implementation/Graphics_Backend/Vulkan/Types/vk/ValidationLayer.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <string>


namespace Aero::Renderer::Impl {
	VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
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

	void setupDebugMessenger(vk::raii::DebugUtilsMessengerEXT& debugMessenger, vk::raii::Instance& instance) {
		if (!enableValidationLayers) return;

		vk::DebugUtilsMessageSeverityFlagsEXT severityFlags =
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;


		vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags =
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
			vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
			vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;

		vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT;
		debugUtilsMessengerCreateInfoEXT.messageSeverity = severityFlags;
		debugUtilsMessengerCreateInfoEXT.messageType = messageTypeFlags;
		debugUtilsMessengerCreateInfoEXT.pfnUserCallback = &debugCallback;

		debugMessenger = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
	}
}

