#pragma once
#include "Renderer/Interface/Graphics_Backend/IGraphicsBackend.hpp"

namespace Aero {
	namespace Renderer {
		struct RenderContext {
			Interface::IGraphicsBackend* backend;
			// you can add other per frame data here later
		};
	}
}