#pragma once

#include <Renderer/Interface/Types/IRenderCommand.hpp>
#include <Renderer/Interface/Types/GraphicsAPIs.hpp>

namespace Aero {
	namespace Renderer {
		namespace Interface {
            class IRenderer {
            public:
                virtual ~IRenderer() = default;

                virtual void Initialize() = 0;
                virtual void Submit(IRenderCommand* command) = 0;
                virtual void RenderFrame() = 0;
                virtual void Cleanup() = 0;
                virtual const GraphicsAPIs GetGraphicsAPI() = 0;
            };
		}
	}
}