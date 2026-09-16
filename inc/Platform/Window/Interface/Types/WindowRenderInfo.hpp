#pragma once
#include <Platform/Window/Interface/IRenderSurface.hpp>

namespace Aero::Platform::Window::Interface {
    struct WindowRenderInfo {
        Aero::Platform::Window::Interface::IRenderSurface* renderSurface = nullptr;
        bool vsyncState = false;
        void* nativeWindowHandle = nullptr;
    };
}