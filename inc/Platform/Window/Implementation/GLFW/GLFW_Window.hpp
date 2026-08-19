#pragma once

#include <vector>
#include <string>
#include <memory>
#include <GLFW/glfw3.h>
#include <Platform/Window/Interface/IWindow.hpp>
#include <Platform/Window/Implementation/GLFW/GLFWRenderSurface.hpp>
#include <Platform/Window/Interface/Types/WindowAPIs.hpp>
#include <Renderer/Interface/Types/GraphicsAPIs.hpp>
#include <Platform/Window/Interface/Types/ExtensionInfo.hpp>

namespace Aero {
	namespace Platform {
		namespace Window {
			namespace Impl {
					// For full-screen, pointer lock, and Vsync
					struct WindowState {
						bool isFullscreen = false;
						bool isCursorLocked = false;
						bool VSyncEnabled = true;

						int width = 1280;
						int height = 720;
						int windowedPosX = 100;
						int windowedPosY = 100;
						std::string title;
					};

					class GLFW_Window : public Interface::IWindow
					{
					public:
						GLFW_Window(int width, int height, const std::string& title,
							int glMajorVersion = 3, int glMinorVersion = 3);
						GLFW_Window(const std::string& title, int glMajorVersion, int glMinorVersion);
						GLFW_Window(const std::string& title);
						~GLFW_Window() override;

						void PollEvents() override;
						void SwapBuffers() override;
						bool ShouldClose() const override;
						void SetTitle(const std::string& title) override;
						void SetSize(int width, int height) override;
						void GetSize(int& width, int& height) const override;
						glm::ivec2 GetSize() const override;
						Interface::ExtensionInfo QueryExtensions(Renderer::Interface::GraphicsAPIs api) override;
						bool IsFullscreen() const;
						bool IsVSyncEnabled() const;
						bool IsPointerLocked() const;
						void SetPosition(int x, int y);
						void GetPosition(int& x, int& y);
						void ToggleVSync();
						void ToggleFullscreen();
						void TogglePointerLock();
						void SetWindowIcon(const std::string& iconFilePath);
						glm::ivec2 GetPosition() const;
						glm::ivec2 GetFramebufferSize() const override;

						// Return native handle for internal use or advanced scenarios
						void* GetNativeHandle() const override { return static_cast<void*>(window_); }

						Interface::IRenderSurface* GetSurface() { return surface_.get(); }

						const Interface::WindowAPIs GetBackend() override { return Interface::WindowAPIs::GLFW; }

					private:
						GLFWwindow* window_ = nullptr;
						bool shouldClose_ = false;
						bool initialized_ = false;
						bool isCleanedUp_ = false;

						WindowState state_;
						std::unique_ptr<GLFWRenderSurface> surface_;

						static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
						void HandleInternalInput();
					};
				}
		}
	};
};
