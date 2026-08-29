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
                void createInstance();
                void setupDebugMessenger();
                void pickPhysicalDevice();
                uint64_t ratePhysicalDevices(vk::raii::PhysicalDevice const& physicalDevice);
                bool isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice);
                void createLogicalDevice();
            };
        }
    }
}