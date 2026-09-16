#include "Renderer/Implementation/Graphics_Backend/Vulkan/vkGraphicsBackend.hpp"
#include "Renderer/Implementation/Graphics_Backend/Vulkan/vkInitParams.hpp"
#include <Platform/Window/Interface/Types/ExtensionInfo.hpp>
#include <Utilities/Assert.hpp>
#include <Renderer/Implementation/Graphics_Backend/Vulkan/Types/vk/ValidationLayer.hpp>
#include <Platform/Window/Interface/Types/WindowAPIs.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <limits>
#include <ranges>
#include <map>

namespace Aero {
	namespace Renderer {
		namespace Impl {
			vkGraphicsBackend::vkGraphicsBackend(Renderer::vkInitParams& params) : params_(params) {}

			void vkGraphicsBackend::Initialize() {
				createInstance();
				setupDebugMessenger();
				createSurface();
				pickPhysicalDevice();
				createLogicalDevice();
				createSwapChain();
				createImageView();
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
				// idk why initializer list didnt work for this; I am NOT using modules until compilers can get their shit together with module support
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

				// maybe add a check to ensure that the instance creation was successful? problem is using vk result would lock us into using the c api, and we are avoiding that
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

				for (const auto &physicalDevice: physicalDevices) {
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

			bool vkGraphicsBackend::isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice) const {
				auto queueFamilies = physicalDevice.getQueueFamilyProperties();

				bool isRequestedVulkanVersionSupported = false;
				bool supportsRequiredFeatures = false;

				// features for 1.3+
				auto features = physicalDevice.template getFeatures2<vk::PhysicalDeviceFeatures2,
					vk::PhysicalDeviceVulkan11Features,
					vk::PhysicalDeviceVulkan13Features,
					vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

				uint32_t qfpIndex      = 0;
				bool     supportsGraphicsAndPresent =
					std::ranges::any_of(queueFamilies,
										[&physicalDevice, &surface = this->vkContext_.surface, &qfpIndex](auto const &qfp) {
											bool const suitable = (qfp.queueFlags & vk::QueueFlagBits::eGraphics) && physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface);
											qfpIndex++;
											return suitable;
										});

				// Check if all required physicalDevice extensions are available
				auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
				bool supportsAllRequiredExtensions =
				  std::ranges::all_of( requiredDeviceExtension,
									   [&availableDeviceExtensions]( auto const & requiredDeviceExtension )
									   {
										 return std::ranges::any_of( availableDeviceExtensions,
																	 [requiredDeviceExtension]( auto const & availableDeviceExtension )
																	 { return strcmp( availableDeviceExtension.extensionName, requiredDeviceExtension ) == 0; } );
									   } );

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

				return (supportsGraphicsAndPresent && isRequestedVulkanVersionSupported && supportsRequiredFeatures && supportsAllRequiredExtensions);
			}

			uint64_t vkGraphicsBackend::ratePhysicalDevices(vk::raii::PhysicalDevice const& physicalDevice) {
				// properties, features, and extensions
				auto deviceProperties = physicalDevice.getProperties();
				auto deviceFeatures = physicalDevice.getFeatures();
				auto deviceMemoryProperties = physicalDevice.getMemoryProperties();
				auto deviceExtensionProperties = physicalDevice.enumerateDeviceExtensionProperties();
				
				uint64_t score = 0;

				// device properties

				// discrete gpu check. usually separates the powerful ones with the weak ones. Although there is an edge case where a super weak, older gpu will be selected over the better, integrated one because it's
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
				constexpr vk::Bool32 boolean = true;
				if (deviceFeatures.geometryShader == boolean) {
					score += 2000;
				}

				if (deviceFeatures.tessellationShader == boolean) {
					score += 2000;
				}

				/*	// device memory properties
					uint64_t totalDeviceLocalMemory = 0;
					for (uint32_t i = 0; i < deviceMemoryProperties.memoryHeapCount; i++) {
						if (deviceMemoryProperties.memoryHeaps[i].flags & vk::MemoryHeapFlags::BitsType::eDeviceLocal) {
							totalDeviceLocalMemory += deviceMemoryProperties.memoryHeaps[i].size;
						}
					}

					uint64_t memoryInMB = totalDeviceLocalMemory / (1024 * 1024);

					if (deviceProperties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
						// cap integrated gpu memory contribution so 32GB of slow RAM doesn't break the math
						score += std::min(memoryInMB, static_cast<uint64_t>(2048));
					}
					else {
						score += memoryInMB;
					}*/

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
				std::vector<vk::QueueFamilyProperties> queueFamilyProperties = vkContext_.physicalDevice.getQueueFamilyProperties();

				// get the first index into queueFamilyProperties which supports graphics
				// get the first index into queueFamilyProperties which supports both graphics and present
				uint32_t queueIndex = ~0;
				for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
				{
					if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
						vkContext_.physicalDevice.getSurfaceSupportKHR(qfpIndex, *vkContext_.surface))
					{
						// found a queue family that supports both graphics and present
						queueIndex = qfpIndex;
						break;
					}
				}
				if (queueIndex == ~0)
				{
					throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
				}

				float queuePriority = 0.5f;

				vk::DeviceQueueCreateInfo deviceQueueCreateInfo;
				deviceQueueCreateInfo.queueFamilyIndex = queueIndex;
				deviceQueueCreateInfo.queueCount = 1;
				deviceQueueCreateInfo.pQueuePriorities = &queuePriority;

				// Create a chain of feature structures
				vk::StructureChain<vk::PhysicalDeviceFeatures2,
					vk::PhysicalDeviceVulkan11Features,
					vk::PhysicalDeviceVulkan13Features,
					vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
					featureChain;

				featureChain.get<vk::PhysicalDeviceVulkan11Features>()
					.setShaderDrawParameters(true);

				featureChain.get<vk::PhysicalDeviceVulkan13Features>()
					.setDynamicRendering(true);

				featureChain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>()
					.setExtendedDynamicState(true);

				vk::DeviceCreateInfo deviceCreateInfo;
				deviceCreateInfo.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>();
				deviceCreateInfo.queueCreateInfoCount = 1;
				deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
				deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtension.size());
				deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtension.data();

				vkContext_.device = vk::raii::Device(vkContext_.physicalDevice, deviceCreateInfo);
				vkContext_.graphicsQueue = vk::raii::Queue(vkContext_.device, queueIndex, 0);
			}

			void vkGraphicsBackend::createSurface() {
				// We use the C API here mainly for compatibility reasons
				VkSurfaceKHR Csurface;
				switch (params_.windowRenderInfo.renderSurface->GetSurfaceAPI()) {
				case Aero::Platform::Window::Interface::WindowAPIs::GLFW: {
					// I DO NOT want to pass the IWindow to the renderer, but honestly we might have to. Or, even better, we could just pass the native window API handle (this case, GLFWwindow*)
					VkResult res = glfwCreateWindowSurface(*vkContext_.instance, static_cast<GLFWwindow*>(params_.windowRenderInfo.nativeWindowHandle), nullptr, &Csurface);
					if (res != VK_SUCCESS) {
						throw std::runtime_error("Failed to create window surface! VkResult: " + std::to_string(static_cast<int>(res)));
					}
					break;
				}
				default:
					throw std::runtime_error("Invalid Window API to operate on for window surface creation!");
				}

				vkContext_.surface = vk::raii::SurfaceKHR(vkContext_.instance, Csurface);
			}

			vk::SurfaceFormatKHR vkGraphicsBackend::chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const &availableFormats)
			{
				AERO_ASSERT(!availableFormats.empty(), "Swapchain has no available formats!");
				return availableFormats[0];

				const auto formatIt = std::ranges::find_if(
					availableFormats,
					[](const auto &format) { return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
				// if formatIt is pointing to a valid element (not availableFormats.end(), that means no element was found that matches the lambda logic), then de-ref formatIt, then return it, otherwise just pick
				// the first element in availableFormats. ? : is just an if else statement
				return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
			}

			vk::PresentModeKHR vkGraphicsBackend::chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentModes) {\
				// eFIFO is the only guaranteed mode to be available. eMailbox is probably the better one, being equivalent to triple buffer vsync. If we can't get eMailBox, we just use FIFO
				// Also, the tutorial has an assert statement to check if fifo was available. That is redundant because FIFO is always available in Vulkan, so we don't need to check
				// This function checks in the entire vector if mailbox shows up at all, and if it does (via the lambda, which has an input parameter of value (which the std ranges any of repeatedly runs, changing the
				// value from the vector that we input) of a present mode enum, and we check if it is equal to mailbox. if it is (the question mark, the main if statement part), then we return that.
				// otherwise, we just return FIFO because it is guaranteed
				return std::ranges::any_of(availablePresentModes, [](const vk::PresentModeKHR value) { return vk::PresentModeKHR::eMailbox == value;}) ? vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;
			}

			vk::Extent2D vkGraphicsBackend::chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities) {
				// currentExtent is only set to the special "undefined" value described above
				// when the window manager lets us choose the extent ourselves; any other value
				// means the surface already dictates a fixed event that we must use as-is.
				if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
					return capabilities.currentExtent;
				}

				return {
					std::clamp<uint32_t>(static_cast<uint32_t>(params_.windowRenderInfo.renderSurface->GetSize().x), capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
					std::clamp<uint32_t>(static_cast<uint32_t>(params_.windowRenderInfo.renderSurface->GetSize().y), capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
				};
			}


			uint32_t vkGraphicsBackend::chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities) {
				auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
				if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount)) {
					minImageCount = surfaceCapabilities.maxImageCount;
				}
				return minImageCount;
			}

			void vkGraphicsBackend::createSwapChain() {
				vk::SurfaceCapabilitiesKHR surfaceCapabilities = vkContext_.physicalDevice.getSurfaceCapabilitiesKHR( *vkContext_.surface );
				vkContext_.swapChainExtent = chooseSwapExtent(surfaceCapabilities);
				uint32_t minImageCount = chooseSwapMinImageCount(surfaceCapabilities);

				std::vector<vk::SurfaceFormatKHR> availableFormats = vkContext_.physicalDevice.getSurfaceFormatsKHR(*vkContext_.surface);
				vkContext_.swapChainSurfaceFormat = chooseSwapSurfaceFormat(availableFormats);

				std::vector<vk::PresentModeKHR> availablePresentModes = vkContext_.physicalDevice.getSurfacePresentModesKHR(*vkContext_.surface);
				vk::PresentModeKHR presentMode = chooseSwapPresentMode(availablePresentModes);

				uint32_t imageCount = surfaceCapabilities.minImageCount + 1;

				vk::SwapchainCreateInfoKHR swapChainCreateInfo;
				swapChainCreateInfo.surface = *vkContext_.surface;
				swapChainCreateInfo.minImageCount = minImageCount;
				swapChainCreateInfo.imageFormat = vkContext_.swapChainSurfaceFormat.format;
				swapChainCreateInfo.imageColorSpace = vkContext_.swapChainSurfaceFormat.colorSpace;
				swapChainCreateInfo.imageExtent = vkContext_.swapChainExtent;
				// specifies the number of layers each image consists of. Always 1 unless doing stereoscopic stuff
				swapChainCreateInfo.imageArrayLayers = 1;
				// specifies what kind of operations we will be doing on the images in the swapchain. Right now we only have color, but we will add depth eventually,
				// but other operations can be applied, like memory transfer commands (assemble images elsewhere then transfer them to here) or depth, stencil, etc.
				swapChainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
				// specifies how to handle images that belong to multiple queue families. We can either use exclusive, which makes images owned by one queue family at a time,
				// and ownership must be explicitly transferred before using it in other queue family (best perf), or concurrent, which allows for it to be used across multiple
				// families without direct ownership transfers. On most hardware, the graphics and present queues are in the same family, so no need for transfer of ownership.
				swapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
				// specifies transformation operations on the images in the swapchain, like 90 degree rotations, flipping images horizontally, etc.
				swapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
				// specifies blending mode to be used with other windows in the window system. We want a fully opaque presentation.
				swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
				// Our present mode we choose. In our function, we either get FIFO or mailbox.
				swapChainCreateInfo.presentMode = chooseSwapPresentMode(availablePresentModes);
				// clipped specifies whether or not we care about the color of pixels that are obscured (e.g. a window in front of them)
				swapChainCreateInfo.clipped = true;
				// specifies old swapchain, as we might need to ref it once it becomes unoptimized  or we resize the window
				swapChainCreateInfo.oldSwapchain = nullptr;

				// finally, create the swapchain
				vkContext_.swapChain = vk::raii::SwapchainKHR(vkContext_.device, swapChainCreateInfo);
				vkContext_.swapChainImages = vkContext_.swapChain.getImages();
			}

			void vkGraphicsBackend::createImageView() {
				AERO_ASSERT(vkContext_.swapChainImageViews.empty(), "Swapchain image views were not empty during creation!");

				vk::ImageViewCreateInfo imageViewCreateInfo;
				imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
				imageViewCreateInfo.format = vkContext_.swapChainSurfaceFormat.format;
				imageViewCreateInfo.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0 ,1};
				imageViewCreateInfo.components = {
					vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity};

				for (auto &image : vkContext_.swapChainImages) {
					imageViewCreateInfo.image = image;
					vkContext_.swapChainImageViews.emplace_back(vkContext_.device, imageViewCreateInfo);
				}
			}
		}
	}
}