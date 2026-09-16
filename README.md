# Vulkan Renderer 2026

## Goals:
- Create a modern, modular, and robust Vulkan renderer. The renderer is using modern C++ 20 and Vulkan features to ensure 
high performance and maintainability

- Target modern hardware and using it to the fullest to achieve high-fidelity graphics
- Allow for different use cases, making the renderer versatile and powerful for different workloads.
- Create a toolchain for developers to have an easier time using Vulkan to its fullest
- Ensure the renderer is ready for integration with my game engine and can seamlessly operate inside of it

## Roadmap

End of 2026:

- Have basic rendering technology implemented with stable architecture
- Test renderer on different hardware, such as different operating systems, GPUs, CPUs, etc.

## Build Instructions:

This project uses CMake as its build system generator. All code should be tested to run on Windows and Linux (MacOS support is not a major priority right now, but will be in the future).
To build the project, run the following commands (work on both Linux and Windows):

    mkdir build
    cd build
    cmake ..
    cmake --build .

To generate documentation, run the following:
    
    cmake --build . --target docs

## Prerequisites:

 - GPU with Vulkan 1.3 support or higher, dynamic rendering, and extended dynamic state via PhysicalDeviceExtendedDynamicStateFeaturesEXT 

 - Proper environment to display window and results from rendering
 - Compiler with C++ 20 support 

## Acknowledgements:

* [GLFW](https://www.glfw.org/)
* [Vulkan](https://www.vulkan.org/)
* [glm](https://glm.g-truc.net/0.9.9/index.html)
* [doxygen](https://www.doxygen.nl/)
* [stb_image](https://nothings.org/)
* [Slang Shading Language](https://shader-slang.org/)