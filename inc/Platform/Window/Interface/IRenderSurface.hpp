#pragma once

#include <glm/glm.hpp>

namespace Aero {
    namespace Platform {
        namespace Window {
            namespace Interface {
                class IRenderSurface {
                public:
                    virtual ~IRenderSurface() = default;

                    // expose native handle function?
                    virtual glm::ivec2 GetSize() const = 0;
                    virtual void Bind() = 0;   // make framebuffer current
                    virtual void Present() = 0;   // end frame
                };
            }
        }
    }
}