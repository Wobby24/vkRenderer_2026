#pragma once

#include <stdexcept>
#include <glm/glm.hpp>
#include <Platform/Window/Interface/IRenderSurface.hpp>

// Forward declare GLFW types
struct GLFWwindow;

namespace Aero {
    namespace Platform {
        namespace Window {
            namespace Impl {
                class GLFWRenderSurface final : public Interface::IRenderSurface
                {
                public:
                    explicit GLFWRenderSurface(GLFWwindow* window);

                    glm::ivec2 GetSize() const override;
                    void Bind() override;
                    void Present() override;

                private:
                    GLFWwindow* window_ = nullptr; // non-owning
                };
            }
        }
    }
}
