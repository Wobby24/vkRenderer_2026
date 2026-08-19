#include <iostream>
#include <GLFW/glfw3.h>
#include <cstdint>
#include <Renderer/Implementation/Test_Renderer.hpp>
#include <Platform/Window/Implementation/GLFW/GLFW_Window.hpp>
#include <Renderer/Implementation/Types/RendererInitParams.hpp>

int main() {
    Aero::Platform::Window::Impl::GLFW_Window window("test window");

    Aero::Renderer::Impl::RendererInitParams params;
    params.api = Aero::Renderer::Interface::GraphicsAPIs::Vulkan;
    params.extensionInfo = window.QueryExtensions(params.api);
    params.renderWidth = window.GetFramebufferSize().x;
    params.renderHeight = window.GetFramebufferSize().y;
    params.renderSurface_ = window.GetSurface();
    params.vsyncState = window.IsVSyncEnabled();

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