#include "Renderer/Implementation/Graphics_Backend/Vulkan/vkGraphicsBackend.hpp"
#include "Renderer/Implementation/Graphics_Backend/Vulkan/vkInitParams.hpp"
#include <Platform/Window/Interface/Types/ExtensionInfo.hpp>
#include <Utilities/Assert.hpp>
#include <Renderer/Implementation/Graphics_Backend/Vulkan/Types/vk/ValidationLayer.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace Aero {
	namespace Renderer {
		namespace Impl {
			vkGraphicsBackend::vkGraphicsBackend(Renderer::vkInitParams& params) : params_(params) {}

			void vkGraphicsBackend::Initialize() {
				createInstance();
				setupDebugMessenger();
			}

			void vkGraphicsBackend::Shutdown() {
				// idk what to do here yet
			}

			void vkGraphicsBackend::createInstance() {
				vkContext_.appInfo = {
					"VulkanRenderer",
					VK_MAKE_VERSION(1, 0, 0),
					"Aero Game Development Engine (GDE)",
					VK_MAKE_VERSION(1, 0, 0),
					vk::ApiVersion14
			};

				std::vector<char const*> requiredLayers;
				if (enableValidationLayers) {
					requiredLayers.assign(validationLayers.begin(), validationLayers.end());
				}

				// REQUIRED VALIDATION LAYERS
				// 
				// check if the required layers are supported by the Vulkan implementation
				auto layerProperties = vkContext_.context.enumerateInstanceLayerProperties();
				// why tf do we have to do this, this syntax is so freaking ugly.
				auto unsupportedLayerIt = std::ranges::find_if(requiredLayers,
					[&layerProperties](auto const& requiredLayer) {
						return std::ranges::none_of(layerProperties,
							[requiredLayer](auto const& layerProperty) { return strcmp(layerProperty.layerName, requiredLayer) == 0; });
					});

				if (unsupportedLayerIt != requiredLayers.end()) {
					throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
				}

				// REQUIRED EXTENSIONS

				// Check if the required extensions are supported by the Vulkan implementation.
				auto extensionProperties = vkContext_.context.enumerateInstanceExtensionProperties();
				auto unsupportedPropertyIt =
					std::ranges::find_if(params_.extensionInfo.extensions_,
						[&extensionProperties](auto const& requiredExtension) {
							return std::ranges::none_of(extensionProperties,
								[requiredExtension](auto const& extensionProperty) { return strcmp(extensionProperty.extensionName, requiredExtension) == 0; });
						});
				if (unsupportedPropertyIt != params_.extensionInfo.extensions_.end())
				{
					throw std::runtime_error("Required extension not supported: " + std::string(*unsupportedPropertyIt));
				}

				// we need to grab extensions for vulkan; make a function in the iwindow class that enables us to grab extensions, however though not all window libs support grabbing that,
				// so if they dont, just throw an exception when those functions are used in ones that dont support it
				// idk why intializer list didnt work for this; I am NOT using modules until compilers can get their shit together with module support
				vk::InstanceCreateInfo createInfo;
				createInfo.pApplicationInfo = &vkContext_.appInfo;

				// Validation layers
				AERO_ASSERT(!requiredLayers.empty(), "Validation Layer info passed to graphics backend was empty!");
				createInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
				createInfo.ppEnabledLayerNames = requiredLayers.data();

				//extensions
				AERO_ASSERT(!params_.extensionInfo.extensions_.empty(), "Extension info passed to graphics backend was empty!");
				createInfo.enabledExtensionCount = params_.extensionInfo.count;
				createInfo.ppEnabledExtensionNames = params_.extensionInfo.extensions_.data();

				// maybe add a check to ensure that the instance creation was succesful? problem is using vk result would lock us into using the c api, and we are avoiding that
				vkContext_.instance = vk::raii::Instance(vkContext_.context, createInfo);

			}

			void vkGraphicsBackend::setupDebugMessenger() {
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

				vkContext_.debugMessenger = vkContext_.instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
			}

			void vkGraphicsBackend::pickPhysicalDevice() {
				auto physicalDevices = vkContext_.instance.enumeratePhysicalDevices();
				if (physicalDevices.empty()) {
					throw std::runtime_error("Failed to find GPUs with Vulkan support!");
				}

				for (auto physicalDevice: physicalDevices) {
	
				}
			}

			uint32_t vkGraphicsBackend::ratePhysicalDevices(vk::raii::PhysicalDevice const& physicalDevice) {
				auto deviceProperties = physicalDevice.getProperties();
				auto deviceFeatures = physicalDevice.getFeatures();
				
				uint32_t score = 0;

				// discrete gpu check. usually seperates the powerful ones with the weak ones. Although there is an edge case where a super weak, older gpu will be selected over the better, integrated one because it's
				// discrete, but we will fix that
				if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
					score += 10000;
				}

				if (deviceFeatures.geometryShader == true) {
					score += 2000;
				}

				if (deviceFeatures.tessellationShader == true) {
					score += 2000;
				}

				// idk what else to put here besides memory checks
			}
		}
	}
}