#pragma once
#include "Renderer/Interface/IRenderer.hpp"
#include "Renderer/Interface/Types/IRenderCommand.hpp"
#include "Renderer/Implementation/Types/RendererInitParams.hpp"
#include "Renderer/Interface/Graphics_Backend/IGraphicsBackend.hpp"
#include "Utilities/ArenaAllocator.hpp"
#include <vector>

namespace Aero {
	namespace Renderer {
		namespace Impl {
			class Test_renderer : public Interface::IRenderer {
			public:
				Test_renderer(Renderer::Impl::RendererInitParams& params);
				~Test_renderer();

				void Initialize() override;
				void Submit(Interface::IRenderCommand* command) override;
				void RenderFrame() override;
				void Cleanup() override;
				// for now just return vulkan
				const Interface::GraphicsAPIs GetGraphicsAPI() override { return Renderer::Interface::GraphicsAPIs::Vulkan; }
			private:
				bool isInit_ = false;
				bool isCleaned_ = false;
				Renderer::Impl::RendererInitParams params_;
				std::vector<Interface::IRenderCommand*> commands_;
				// switch statement based off init param's api memeber to properly create graphics backend
				Renderer::Interface::IGraphicsBackend* graphicsBackend_;
				Aero::Utilities::ArenaAllocator allocator_;
			};
		}
	}
}