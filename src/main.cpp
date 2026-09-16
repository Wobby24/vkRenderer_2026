#include <iostream>
#include <GLFW/glfw3.h>
#include <cstdint>
#include <Renderer/Implementation/Test_Renderer.hpp>
#include <Platform/Window/Implementation/GLFW/GLFW_Window.hpp>
#include <Renderer/Implementation/Types/RendererInitParams.hpp>

int main() {
    ///
    /// Main entry function contains initialization for the window and renderer. The renderer gathers parameters from the window, which is then passed to the graphics backend.
    ///
    Aero::Platform::Window::Impl::GLFW_Window window("test window");

    Aero::Renderer::Impl::RendererInitParams params;
    params.api = Aero::Renderer::Interface::GraphicsAPIs::Vulkan;
    params.extensionInfo = window.QueryExtensions(params.api);
    params.windowRenderInfo.renderSurface = window.GetSurface();
    params.windowRenderInfo.vsyncState = window.IsVSyncEnabled();
    params.windowRenderInfo.nativeWindowHandle = window.GetNativeHandle();

	Aero::Renderer::Impl::Test_renderer renderer_(params);
    
    try
    {
        renderer_.Initialize();
        
        while (!window.ShouldClose()) {
            window.PollEvents();
            
            renderer_.RenderFrame();
        }

        renderer_.Cleanup();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}