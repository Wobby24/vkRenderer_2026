#include <PLatform/Window/Implementation/GLFW/GLFWRenderSurface.hpp>
#include <Platform/Window/Interface/Types/WindowAPIs.hpp>
#include <GLFW/glfw3.h>

namespace Aero {
	namespace Platform {
		namespace Window {
			namespace Impl {

                GLFWRenderSurface::GLFWRenderSurface(GLFWwindow* window)
                    : window_(window)
                {
                    if (!window_) {
                        throw std::runtime_error("GLFWRenderSurface: GLFWwindow is null");
                    }
                }

                glm::ivec2 GLFWRenderSurface::GetSize() const
                {
                    int w = 0, h = 0;
                    glfwGetFramebufferSize(window_, &w, &h);
                    return { w, h };
                }

                void GLFWRenderSurface::Bind()
                {
                    glfwMakeContextCurrent(window_);
                  //  glBindFramebuffer(GL_FRAMEBUFFER, 0);
                }

                void GLFWRenderSurface::Present()
                {
                    glfwSwapBuffers(window_);
                }

                Aero::Platform::Window::Interface::WindowAPIs GLFWRenderSurface::GetSurfaceAPI() {
                    return Aero::Platform::Window::Interface::WindowAPIs::GLFW;
                }
			}
		}
	}
}

