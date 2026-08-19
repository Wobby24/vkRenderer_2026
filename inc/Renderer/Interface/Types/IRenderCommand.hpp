#pragma once

namespace Aero {
	namespace Renderer {
		namespace Interface {
			class IRenderCommand {
			public:
				virtual ~IRenderCommand() = default;

				virtual void Execute() = 0;
			};
		}
	}
}
