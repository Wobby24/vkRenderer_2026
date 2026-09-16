#pragma once
#include <Platform/Window/Interface/Types/ExtensionInfo.hpp>
#include <Platform/Window/Interface/Types/WindowRenderInfo.hpp>

namespace Aero {
	namespace Renderer {
		struct vkInitParams {
			// include basic information
			int VulkanVersionMinor = 4;
			bool enableValidationLayers = true;
			Platform::Window::Interface::ExtensionInfo extensionInfo;
			Platform::Window::Interface::WindowRenderInfo windowRenderInfo;
		};
	}
}