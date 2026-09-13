#include "Renderer/Implementation/Test_Renderer.hpp"
#include "Renderer/Implementation/Graphics_Backend/Vulkan/vkGraphicsBackend.hpp"
#include "Renderer/Implementation/Graphics_Backend/Vulkan/vkInitParams.hpp"
#include <vector>

namespace Aero::Renderer::Impl {
	Test_renderer::Test_renderer(Renderer::Impl::RendererInitParams& params) : isInit_(false), isCleaned_(false), params_(params), graphicsBackend_(nullptr), allocator_(2048) {

	}

	Aero::Renderer::Impl::Test_renderer::~Test_renderer()
	{
		if (!isCleaned_ && isInit_)
			Cleanup();
	}

	void Aero::Renderer::Impl::Test_renderer::Initialize()
	{
		switch (params_.api) {
		case Renderer::Interface::GraphicsAPIs::Vulkan:
		{
			// forge params for graphics backend. basic values for now, will eventually allow for more customization
			Renderer::vkInitParams params;
			// for testing, integrate with ndebug macro eventually
			params.enableValidationLayers = true; 
			params.extensionInfo = params_.extensionInfo;
			params.renderSurface = params_.renderSurface_;
			params.VulkanVersionMinor = 3;
			params.windowHandle = params_.nativeWindowHandle;

			graphicsBackend_ = allocator_.allocate<Renderer::Impl::vkGraphicsBackend>(params);
			graphicsBackend_->Initialize();
			
			break;
		}
		}
	}

	void Aero::Renderer::Impl::Test_renderer::Submit(Interface::IRenderCommand* command)
	{
		commands_.push_back(command);
	}

	void Aero::Renderer::Impl::Test_renderer::RenderFrame()
	{
		for (auto* command : commands_)
		{
			command->Execute();
		}

		commands_.clear();
	}

	void Aero::Renderer::Impl::Test_renderer::Cleanup()
	{
		allocator_.reset();
	}
}