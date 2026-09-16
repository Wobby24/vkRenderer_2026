#include "Renderer/Interface/Graphics_Backend/IGraphicsBackend.hpp"
#include <Renderer/Interface/IRenderer.hpp>
#include <Renderer/Interface/Types/IRenderCommand.hpp>
#include <Renderer/Interface/Types/GraphicsAPIs.hpp>
#include <Renderer/Implementation/Graphics_Backend/Vulkan/vkInitParams.hpp>
#include <Renderer/Implementation/Graphics_Backend/Vulkan/Types/vk/vkContext.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <memory>
#include <cstdint>
#include <vector>

namespace Aero {
    namespace Renderer {
        namespace Impl {
            class vkGraphicsBackend : public Interface::IGraphicsBackend {
            public:
                vkGraphicsBackend(Renderer::vkInitParams& params);
                ~vkGraphicsBackend() = default;

                void Initialize() override;
                void Shutdown() override;

                const Interface::GraphicsAPIs GetGraphicsAPI() override
                {
                    return Interface::GraphicsAPIs::Vulkan;
                }

            private:
                Renderer::vkInitParams params_;
                Renderer::vkContext vkContext_;

                // INITIAL SETUP FUNCTIONS: //
                void createInstance();
                void setupDebugMessenger();
                void pickPhysicalDevice();
                uint64_t ratePhysicalDevices(vk::raii::PhysicalDevice const& physicalDevice);
                bool isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice) const;
                void createLogicalDevice();

                // PRESENTATION SETUP FUNCTIONS: //

                void createSurface();
                vk::SurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const &availableFormats);
                vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentModes);
                vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities);
                void createSwapChain();
                uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities);
                void createImageView();

                // GRAPHICS PIPELINE SETUP FUNCTIONS: //

                void createGraphicsPipeline();

                // DATA TYPES FOR INITIALIZATION: //

                // extensions feature check
                const std::string extensionsForPoints[4] = {
                    "VK_KHR_ray_tracing_pipeline",
                    "VK_EXT_descriptor_indexing",
                    "VK_KHR_dynamic_rendering",
                    "VK_KHR_synchronization2",
                };

                std::vector<const char*> requiredDeviceExtension = {
                    vk::KHRSwapchainExtensionName};
            };
        }
    }
}