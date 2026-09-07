#pragma once
#include <Platform/Window/Interface/IRenderSurface.hpp>
#include <Renderer/Interface/Types/GraphicsAPIs.hpp>
#include <Platform/Window/Interface/Types/ExtensionInfo.hpp>
#include <vector>

namespace Aero::Renderer::Impl {
	struct RendererInitParams {
		// add a constructor because msvc keeps whinning about default init of members
		RendererInitParams() : renderSurface_(nullptr), renderWidth(0), renderHeight(0), vsyncState(false), api(Renderer::Interface::GraphicsAPIs::None) {}

		Aero::Platform::Window::Interface::IRenderSurface* renderSurface_ = nullptr;
		int renderWidth = 0;
		int renderHeight = 0;
		bool vsyncState = false;
		// could use pointer if needed, although usually this will only have 2 or so items so im not worried about performance issues
		Platform::Window::Interface::ExtensionInfo extensionInfo;
		Renderer::Interface::GraphicsAPIs api = Renderer::Interface::GraphicsAPIs::None;
		void* nativeWindowHandle; 
	};
}