#pragma once

namespace Aero::Renderer::Interface {
    class IShader {
        virtual ~IShader() = default;

        virtual const std::vector<char>& getCode();
    };
}