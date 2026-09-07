cmake_minimum_required(VERSION 3.28)
cmake_policy(VERSION 3.31)
project(Aero-vkRenderer VERSION 1.0 LANGUAGES CXX C)

# -----------------------------------------------------
# Optional advanced feature(s)
# -----------------------------------------------------
option(ENABLE_SLANG "Enable Slang shader support" ON)

# -----------------------------------------------------
# Build type
# -----------------------------------------------------
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

if(CMAKE_GENERATOR MATCHES "Visual Studio" OR CMAKE_GENERATOR MATCHES "Xcode")
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------------------------------
# Paths
# -----------------------------------------------------
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(INC_DIR "${CMAKE_SOURCE_DIR}/inc")
set(EXT_INC_DIR "${CMAKE_SOURCE_DIR}/ext/inc")
set(TEST_DIR "${CMAKE_SOURCE_DIR}/test")
set(DOCS_DIR "${CMAKE_SOURCE_DIR}/docs")

# -----------------------------------------------------
# Project sources
# -----------------------------------------------------
set(RENDERER_SOURCES
    "src/Renderer/Graphics_Backend/Vulkan/vkGraphicsBackend.cpp"
    "src/Renderer/Test_Renderer.cpp"
)

set(PLATFORM_SOURCES
    "src/Platform/Window/GLFW_Window.cpp"
    "src/Platform/Window/GLFWRenderSurface.cpp"
)

set(EXTERNAL_SOURCES
    "src/ext/stb_image.cpp"
)

set(MAIN_SOURCES "src/main.cpp")
#set(TEST_SOURCES "test/test_main.cpp")

# -----------------------------------------------------
# Dependencies (FetchContent)
# -----------------------------------------------------
include(FetchContent)

# GoogleTest
#FetchContent_Declare(
#  googletest
#  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
#)
#FetchContent_MakeAvailable(googletest)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.2
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
        slang
        GIT_REPOSITORY https://github.com/shader-slang/slang.git
        GIT_TAG v2026.17
)
set(SLANG_ENABLE_OPTIX OFF CACHE INTERNAL "")
set(SLANG_ENABLE_CUDA OFF CACHE INTERNAL "")
set(SLANG_ENABLE_NVAPI OFF CACHE INTERNAL "")
set(SLANG_ENABLE_SLANG_RHI OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_REPLAYER OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(slang)

# Vulkan
find_package(Vulkan REQUIRED COMPONENTS glslc)
message(STATUS "Vulkan libraries: ${Vulkan_LIBRARIES}")

# -----------------------------------------------------
# Project Library
# -----------------------------------------------------
add_library(Renderer_lib STATIC ${RENDERER_SOURCES})
add_library(Platform_lib STATIC ${PLATFORM_SOURCES})

add_compile_definitions(NDEBUG)
#add_compile_definitions(VULKAN_HPP_NO_STRUCT_CONSTRUCTORS)

target_include_directories(Renderer_lib
    PUBLIC
        ${INC_DIR}
        ${EXT_INC_DIR}
        ${glm_SOURCE_DIR}
)

target_include_directories(Platform_lib
    PUBLIC
        ${INC_DIR}
        ${EXT_INC_DIR}
        ${glm_SOURCE_DIR}
)

message(STATUS "Vulkan include dirs: ${Vulkan_INCLUDE_DIRS}")

target_link_libraries(Renderer_lib
    PUBLIC
        glfw
        Vulkan::Vulkan
)

target_link_libraries(Platform_lib
    PUBLIC
        glfw
)

# -----------------------------------------------------
# Main executable
# -----------------------------------------------------
add_executable(aero-vkrenderer ${MAIN_SOURCES} ${EXTERNAL_SOURCES})
target_link_libraries(aero-vkrenderer
    PRIVATE
        Renderer_lib
        Platform_lib
)

# -----------------------------------------------------
# Tests
# -----------------------------------------------------
#add_executable(aero-vkrenderer-tests ${TEST_SOURCES})
#target_link_libraries(aero-vkrenderer-tests PRIVATE vkRenderer_lib GTest::gtest_main)

#include(GoogleTest)
#gtest_discover_tests(aero-vkrenderer-tests)
#enable_testing()

# -----------------------------------------------------
# Shader compilation paths
# -----------------------------------------------------
set(SHADER_DIR "${CMAKE_SOURCE_DIR}/res/shaders")
set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/res/shaders")
file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

# GLSL shader compilation (glslc)
set(GLSLC_EXECUTABLE Vulkan::glslc)

file(GLOB_RECURSE GLSL_SOURCES
    "${SHADER_DIR}/*.vert"
    "${SHADER_DIR}/*.frag"
)

set(SPIRV_OUTPUTS "")
foreach(SHADER ${GLSL_SOURCES})
    get_filename_component(STEM ${SHADER} NAME_WE)
    set(SPIRV_FILE "${SHADER_OUTPUT_DIR}/${STEM}.spv")
    add_custom_command(
        OUTPUT ${SPIRV_FILE}
        COMMAND ${GLSLC_EXECUTABLE} ${SHADER} -o ${SPIRV_FILE}
        DEPENDS ${SHADER}
        COMMENT "Compiling GLSL shader: ${STEM}"
    )
    list(APPEND SPIRV_OUTPUTS ${SPIRV_FILE})
endforeach()
add_custom_target(compile_glsl_shaders ALL DEPENDS ${SPIRV_OUTPUTS})

set(SLANGC_EXECUTABLE $<TARGET_FILE:slangc>)

# Slang shader compilation, this is inefficient because we can handle the compliation directly in our code via the Slang C++ API, allowing for better control over the shader comp process and potentially better build times
if(ENABLE_SLANG)

    file(GLOB_RECURSE SLANG_SOURCES "${SHADER_DIR}/*.slang")
    set(SLANG_OUTPUTS "")
    foreach(SLANG_FILE ${SLANG_SOURCES})
        get_filename_component(STEM ${SLANG_FILE} NAME_WE)
        set(SLANG_SPV "${SHADER_OUTPUT_DIR}/${STEM}.spv")
        add_custom_command(
            OUTPUT ${SLANG_SPV}
            COMMAND ${SLANGC_EXECUTABLE}
                    ${SLANG_FILE}
                    -target spirv
                    -profile spirv_1_4+spvRayQueryKHR
                    -fvk-use-entrypoint-name
                    -entry vertMain
                    -entry fragMain
                    -o ${SLANG_SPV}
            DEPENDS ${SLANG_FILE}
            COMMENT "Compiling Slang shader: ${STEM}"
            VERBATIM
        )
        list(APPEND SLANG_OUTPUTS ${SLANG_SPV})
    endforeach()
    add_custom_target(compile_slang_shaders ALL DEPENDS ${SLANG_OUTPUTS})
endif()

# Doxygen
#find_package(Doxygen REQUIRED)
#if(DOXYGEN_FOUND)
 #   set(DOXYFILE_IN ${CMAKE_SOURCE_DIR}/Doxyfile)
  #  set(DOXYFILE_OUT ${DOCS_DIR}/Doxyfile)
   # configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)
    #add_custom_target(doc ALL
     #   COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
      #  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
       # COMMENT "Generating documentation with Doxygen"
        #VERBATIM
    #)
#endif()
