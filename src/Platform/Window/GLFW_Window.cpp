#include <Platform/Window/Implementation/GLFW/GLFW_Window.hpp>
#include <string>
#include <stdexcept>
#include <GLFW/glfw3.h> // Needed for GLFW calls
#include <iostream>
#include <Utilities/Assert.hpp>
#include <Platform/Window/Interface/Types/ExtensionInfo.hpp>

// suppress warnings for stb_image
#ifdef _MSC_VER
#pragma warning(push, 0)
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#endif

#include <stb_image/stb_image.h>

#ifdef _MSC_VER
#pragma warning(pop)
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif


namespace Aero {
	namespace Platform {
		namespace Window {
			namespace Impl {
				GLFW_Window::GLFW_Window(int width, int height, const std::string& title, int glMajorVersion, int glMinorVersion) 
					: window_(nullptr), shouldClose_(false), initialized_(false), isCleanedUp_(false)
				{
					if (!glfwInit())
						throw std::runtime_error("Failed to initialize GLFW");

					glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glMajorVersion);
					glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glMinorVersion);
					glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
					glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

					// set stuff for window state 
					state_.width = width;
					state_.height = height;
					state_.title = title;

					window_ = glfwCreateWindow(state_.width, state_.height, state_.title.c_str(), nullptr, nullptr);
					if (!window_) {
						glfwTerminate();
						throw std::runtime_error("Failed to create GLFW window");
					}
					glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);

					glfwMakeContextCurrent(window_);
					glfwSwapInterval(1);  // 0 = VSync off, 1 = VSync on
					glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

					// Create the render surface
					surface_ = std::make_unique<GLFWRenderSurface>(window_);
				}

				GLFW_Window::GLFW_Window(const std::string& title, int glMajorVersion, int glMinorVersion)
					: window_(nullptr), shouldClose_(false), initialized_(false), isCleanedUp_(false)
				{
					if (!glfwInit())
						throw std::runtime_error("Failed to initialize GLFW");

					glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glMajorVersion);
					glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glMinorVersion);
					glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
					glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
					state_.title = title;

					window_ = glfwCreateWindow(state_.width, state_.height, state_.title.c_str(), nullptr, nullptr);
					if (!window_) {
						glfwTerminate();
						throw std::runtime_error("Failed to create GLFW window");
					}
					glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);

					glfwMakeContextCurrent(window_);
					glfwSwapInterval(1);  // 0 = VSync off, 1 = VSync on
					glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

					// Create the render surface
					surface_ = std::make_unique<GLFWRenderSurface>(window_);
				}

				GLFW_Window::GLFW_Window(const std::string& title)
					: window_(nullptr), shouldClose_(false), initialized_(false), isCleanedUp_(false)
				{
					if (!glfwInit())
						throw std::runtime_error("Failed to initialize GLFW");

					glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
					glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
					glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
					state_.title = title;

					window_ = glfwCreateWindow(state_.width, state_.height, state_.title.c_str(), nullptr, nullptr);
					if (!window_) {
						glfwTerminate();
						throw std::runtime_error("Failed to create GLFW window");
					}
					glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);

					glfwMakeContextCurrent(window_);
					glfwSwapInterval(1);  // 0 = VSync off, 1 = VSync on
					glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

					// Create the render surface
					surface_ = std::make_unique<GLFWRenderSurface>(window_);
				}

				GLFW_Window::~GLFW_Window()
				{
					if (isCleanedUp_) return;

					glfwDestroyWindow(window_);
					glfwTerminate();
					isCleanedUp_ = true;
				}

				void GLFW_Window::SetWindowIcon(const std::string& iconFilePath) {
					stbi_set_flip_vertically_on_load(0);
					int width, height, channels;
					unsigned char* data = stbi_load(iconFilePath.c_str(), &width, &height, &channels, 4);

					if (!data) {
						std::cerr << "Failed to load icon from: " << iconFilePath << std::endl;
						return;
					}

					GLFWimage icon = { width, height, data };
					glfwSetWindowIcon(window_, 1, &icon);
					stbi_image_free(data);
				}

				void GLFW_Window::HandleInternalInput()
				{
					if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS)
						glfwSetWindowShouldClose(window_, true);

					static bool f11Pressed = false;
					if (glfwGetKey(window_, GLFW_KEY_F11) == GLFW_PRESS) {
						if (!f11Pressed) {
							ToggleFullscreen();
							f11Pressed = true;
						}
					}
					else {
						f11Pressed = false;
					}

					static bool tabPressed = false;
					if (glfwGetKey(window_, GLFW_KEY_TAB) == GLFW_PRESS) {
						if (!tabPressed) {
							TogglePointerLock();
							tabPressed = true;
						}
					}
					else {
						tabPressed = false;
					}
				}

				void GLFW_Window::framebuffer_size_callback(GLFWwindow*, int width, int height)
				{
					//glViewport(0, 0, width, height);
					(void)width;
					(void)height;
				}

				void GLFW_Window::SetTitle(const std::string& title)
				{
					state_.title = title;
					if (window_)
						glfwSetWindowTitle(window_, title.c_str());
				}

				void GLFW_Window::SetSize(int width, int height)
				{
					state_.width = width;
					state_.height = height;
					if (window_)
						glfwSetWindowSize(window_, state_.width, state_.height);
				}

				void GLFW_Window::GetSize(int& width, int& height) const
				{
					if (window_)
						glfwGetWindowSize(window_, &width, &height);
					else
						width = height = 0;
				}

				glm::ivec2 GLFW_Window::GetSize() const {
					if (window_) {
						int width = 0;
						int height = 0;
						glfwGetWindowSize(window_, &width, &height);
						return glm::ivec2(width, height);
					}
					return glm::ivec2(0, 0); // or throw, or handle error as needed
				}

				Interface::ExtensionInfo GLFW_Window::QueryExtensions(Renderer::Interface::GraphicsAPIs api)
				{
					// Switch statement based off api enum then we use glfw functions to query. This function is primarily meant for Vulkan but could work for OpenGL too
					Interface::ExtensionInfo extensionInfo;

					switch (api) {
					case Renderer::Interface::GraphicsAPIs::Vulkan:
					{
						uint32_t count;
						const char** extensions_char = glfwGetRequiredInstanceExtensions(&count);
						AERO_ASSERT(extensions_char != nullptr, "Extensions return nullptr! No extensions supported for Vulkan!");

						extensionInfo.extensions_.assign(
							extensions_char,
							extensions_char + count
						);

						// including ValidationLayers.hpp will give errors because the platform library which is this compiled into doesn't include vulkan, creating preprocessor errors. 
						// to resolve this, we simply just check if NDEBUG is defined, as enableValidationLayers is based off the state of the NDEBUG
						if (!NDEBUG) {
							extensionInfo.extensions_.push_back("vk::EXTDebugUtilsExtensionName");
						}

						extensionInfo.count = count;

						return extensionInfo;
					}
					default:
						Utilities::Logger::instance().log(Utilities::Logger::Severity::INFO, "Graphics API provided to the window does not have extension support.");
						return extensionInfo;
					}
				}

				bool GLFW_Window::ShouldClose() const
				{
					return window_ ? glfwWindowShouldClose(window_) : true;
				}

				void GLFW_Window::PollEvents()
				{
					if (window_)
						glfwPollEvents();
					//handle internal input like fullscreen or closing the window
					HandleInternalInput();
				}

				void GLFW_Window::SwapBuffers()
				{
					surface_->Present();
				}

				void GLFW_Window::ToggleFullscreen() {
					state_.isFullscreen = !state_.isFullscreen;

					if (state_.isFullscreen) {
						//save current windowed size and position for when we toggle again
						GetSize(state_.width, state_.width);
						GetPosition(state_.windowedPosX, state_.windowedPosY);
						//get monitor and apply settings
						GLFWmonitor* monitor = glfwGetPrimaryMonitor();
						const GLFWvidmode* mode = glfwGetVideoMode(monitor);
						glfwSetWindowMonitor(window_, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
					}
					else {
						glfwSetWindowMonitor(window_, nullptr, state_.windowedPosX, state_.windowedPosY,
							state_.width, state_.width, 0);
					}
				}

				void GLFW_Window::TogglePointerLock() {
					state_.isCursorLocked = !state_.isCursorLocked;
					glfwSetInputMode(window_, GLFW_CURSOR,
						state_.isCursorLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
				}

				void GLFW_Window::ToggleVSync() {
					state_.VSyncEnabled = !state_.VSyncEnabled;
					glfwSwapInterval(state_.VSyncEnabled ? 1 : 0);
				}

				void GLFW_Window::SetPosition(int x, int y) {
					glfwSetWindowPos(window_, x, y);
				}

				void GLFW_Window::GetPosition(int& x, int& y) {
					glfwGetWindowPos(window_, &x, &y);
				}

				glm::ivec2 GLFW_Window::GetPosition() const {
					int x, y;
					glfwGetWindowPos(window_, &x, &y);
					return { x, y };
				}

				glm::ivec2 GLFW_Window::GetFramebufferSize() const {
					if (window_) {
						int width = 0;
						int height = 0;
						glfwGetFramebufferSize(window_, &width, &height);
						return glm::ivec2(width, height);
					}
					return glm::ivec2(0, 0); // or throw, or handle error as needed
				}

				//basic get or is methods
				bool GLFW_Window::IsFullscreen() const { return state_.isFullscreen; }
				bool GLFW_Window::IsPointerLocked() const { return state_.isCursorLocked; }
				bool GLFW_Window::IsVSyncEnabled() const { return state_.VSyncEnabled; }

			}
			}
		}
	}