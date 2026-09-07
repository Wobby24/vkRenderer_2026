#pragma once
#include <vector>
#include <Platform/Window/Interface/IRenderSurface.hpp>
#include <Platform/Window/Interface/Types/ExtensionInfo.hpp>
#include <string>

namespace Aero {
	namespace Renderer {
		struct vkInitParams {
			// include basic information
			int VulkanVersionMinor = 4;
			bool enableValidationLayers = true;
			Platform::Window::Interface::ExtensionInfo extensionInfo;
			Platform::Window::Interface::IRenderSurface* renderSurface = nullptr;
			void* windowHandle;
		};
	}
}