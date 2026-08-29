#include "Renderer/Implementation/Graphics_Backend/Vulkan/vkGraphicsBackend.hpp"
#include "Renderer/Implementation/Graphics_Backend/Vulkan/vkInitParams.hpp"
#include <Platform/Window/Interface/Types/ExtensionInfo.hpp>
#include <Utilities/Assert.hpp>
#include <Renderer/Implementation/Graphics_Backend/Vulkan/Types/vk/ValidationLayer.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <algorithm>
#include <map>

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

				std::multimap<uint64_t, vk::raii::PhysicalDevice> candidates;

				for (auto physicalDevice: physicalDevices) {
					if (isDeviceSuitable(physicalDevice)) {
						uint64_t score = ratePhysicalDevices(physicalDevice);
						candidates.insert(std::make_pair(score, physicalDevice));
					}
				}

				if (candidates.empty()) {
					throw std::runtime_error("No GPU with requested Vulkan support found!");
				}

				// multi map is auto ordered, so because the highest score will be at the top, we can just select the top one's second param (which is the physical device)
				vkContext_.physicalDevice = candidates.rbegin()->second;
			}

			bool vkGraphicsBackend::isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice) {
				auto queueFamilies = physicalDevice.getQueueFamilyProperties();

				bool isGrahpicsQueueSupported = false;
				bool isRequestedVulkanVersionSupported = false;
				bool supportsRequiredFeatures = false;

				// features for 1.3+
				auto features = physicalDevice.template getFeatures2<vk::PhysicalDeviceFeatures2,
					vk::PhysicalDeviceVulkan11Features,
					vk::PhysicalDeviceVulkan13Features,
					vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

				// queuflags uses a bitmask, so we cant just do ==, instead we use the and op to compare the 2 and see if it produces a non zero value, or zero
				for (const auto& queueFamily : queueFamilies) {
					if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
						isGrahpicsQueueSupported = true;
						break;
					}
				}

				// implement api checks. i dont really like this, however its good enough for now
				switch (params_.VulkanVersionMinor) {
				case 0:
					isRequestedVulkanVersionSupported = physicalDevice.getProperties().apiVersion >= vk::ApiVersion10;
					supportsRequiredFeatures = true;
					break;
				case 1:
					isRequestedVulkanVersionSupported = physicalDevice.getProperties().apiVersion >= vk::ApiVersion11;
					supportsRequiredFeatures = true;
					break;
				case 2:
					isRequestedVulkanVersionSupported = physicalDevice.getProperties().apiVersion >= vk::ApiVersion12;
					supportsRequiredFeatures = true;
					break;
				case 3:
					isRequestedVulkanVersionSupported = physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;
					break;
				case 4:
					isRequestedVulkanVersionSupported = physicalDevice.getProperties().apiVersion >= vk::ApiVersion14;

					break;
				default:
					return false;
				}

				if (params_.VulkanVersionMinor >= 3) {
					supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
						features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
						features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;
				}

				return (isGrahpicsQueueSupported && isRequestedVulkanVersionSupported && supportsRequiredFeatures);
			}

			uint64_t vkGraphicsBackend::ratePhysicalDevices(vk::raii::PhysicalDevice const& physicalDevice) {
				// properties, features, and extensions
				auto deviceProperties = physicalDevice.getProperties();
				auto deviceFeatures = physicalDevice.getFeatures();
				auto deviceMemoryProperties = physicalDevice.getMemoryProperties();
				auto deviceExtensionProperties = physicalDevice.enumerateDeviceExtensionProperties();
				
				uint64_t score = 0;

				// device properties

				// discrete gpu check. usually seperates the powerful ones with the weak ones. Although there is an edge case where a super weak, older gpu will be selected over the better, integrated one because it's
				// discrete, but we will fix that
				if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
					score += 10000;
				}
				else if (deviceProperties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
					score += 1000;  // iGPUs get a lower base baseline
				}

				// maximum possible size of textures 
				score += deviceProperties.limits.maxImageDimension2D;

				// device features
				vk::Bool32 boolean = true;
				if (deviceFeatures.geometryShader == boolean) {
					score += 2000;
				}

				if (deviceFeatures.tessellationShader == boolean) {
					score += 2000;
				}

				// device memory properties

				uint64_t totalDeviceLocalMemory = 0;
				for (uint32_t i = 0; i < deviceMemoryProperties.memoryHeapCount; i++) {
					if (deviceMemoryProperties.memoryHeaps[i].flags & vk::MemoryHeapFlags::BitsType::eDeviceLocal) {
						totalDeviceLocalMemory += deviceMemoryProperties.memoryHeaps[i].size;
					}
				}

				uint64_t memoryInMB = totalDeviceLocalMemory / (1024 * 1024);

				if (deviceProperties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
					// cap integrated gpu memory contribution so 32GB of slow RAM doesn't break the math
					score += std::min(memoryInMB, uint64_t(2048));
				}
				else {
					score += memoryInMB;
				}
				
				// extensions feature check
				const std::string extensionsForPoints[4] = {
					"VK_KHR_ray_tracing_pipeline",
					"VK_EXT_descriptor_indexing",
					"VK_KHR_dynamic_rendering",
					"VK_KHR_synchronization2",
				};

				for (int i = 0; i < 4; i++) {
					const std::string& targetExt = extensionsForPoints[i];

					auto it = std::find_if(deviceExtensionProperties.begin(),
						deviceExtensionProperties.end(),
						[&targetExt](const VkExtensionProperties& prop) {
							return targetExt == prop.extensionName; 
						});

					if (it != deviceExtensionProperties.end()) {
						score += 2500;
					}
				}

				return score;
			}

			void vkGraphicsBackend::createLogicalDevice() {
			


			}
		}
	}
}