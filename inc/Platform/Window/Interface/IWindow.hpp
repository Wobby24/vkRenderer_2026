#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <Platform/Window/Interface/Types/WindowAPIs.hpp>
#include <Renderer/Interface/Types/GraphicsAPIs.hpp>
#include <Platform/Window/Interface/Types/ExtensionInfo.hpp>

namespace Aero {
	namespace Platform {
		namespace Window {
			namespace Interface {
				class IRenderSurface; // forward declaration

				class IWindow
				{
				public:
					virtual ~IWindow() = default;

					virtual void PollEvents() = 0;
					virtual void SwapBuffers() = 0;
					virtual bool ShouldClose() const = 0;
					virtual void SetTitle(const std::string& title) = 0;
					virtual void SetSize(int width, int height) = 0;
					virtual void GetSize(int& width, int& height) const = 0;
					virtual glm::ivec2 GetSize() const = 0;
					virtual glm::ivec2 GetFramebufferSize() const = 0;
					virtual void* GetNativeHandle() const = 0;
					// Access the render surface associated with this window
					virtual IRenderSurface* GetSurface() = 0; // non-owning
					virtual const WindowAPIs GetBackend() = 0;
					virtual ExtensionInfo QueryExtensions(Renderer::Interface::GraphicsAPIs api) = 0;
				};
			}
		}
	}
}
