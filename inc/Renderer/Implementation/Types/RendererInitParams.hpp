#pragma once
#include <Renderer/Interface/Types/GraphicsAPIs.hpp>
#include <Platform/Window/Interface/Types/ExtensionInfo.hpp>
#include <Platform/Window/Interface/Types/WindowRenderInfo.hpp>


namespace Aero::Renderer::Impl {
	struct RendererInitParams {
		// add a constructor because msvc keeps whinning about default init of members
		RendererInitParams() :  api(Renderer::Interface::GraphicsAPIs::None) {}

		Platform::Window::Interface::WindowRenderInfo windowRenderInfo;
		Platform::Window::Interface::ExtensionInfo extensionInfo;
		Renderer::Interface::GraphicsAPIs api = Renderer::Interface::GraphicsAPIs::None;
	};
}