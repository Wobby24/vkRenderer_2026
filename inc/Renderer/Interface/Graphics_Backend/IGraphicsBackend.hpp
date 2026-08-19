#pragma once
#include "Renderer/Interface/Types/GraphicsAPIs.hpp"

namespace Aero {
	namespace Renderer {
		namespace Interface {
			class IGraphicsBackend {
			public:
				virtual ~IGraphicsBackend() = default;

				virtual void Initialize() = 0;
				virtual void Shutdown() = 0;
				virtual const Interface::GraphicsAPIs GetGraphicsAPI() = 0;
			};
		}
	}
}