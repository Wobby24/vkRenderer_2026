#pragma once
#include <string>
#include <vector>

namespace Aero::Platform::Window::Interface {
	struct ExtensionInfo {
		std::vector<const char*> extensions_;
		uint32_t count = 0;
	};
}