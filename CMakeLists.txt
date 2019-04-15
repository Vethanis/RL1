
cmake_minimum_required(VERSION 3.1)
project(rl1)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw-master)

set(VK_LIBS vulkan-1.lib)

set(CMAKE_CXX_FLAGS_DEBUG "/Od /MTd /Zi")
set(CMAKE_CXX_FLAGS_RELEASE "/O2 /MT /arch:AVX2 /fp:fast")
set(CMAKE_CXX_FLAGS "/W3 /MP /GR-")
include_directories(${CMAKE_SOURCE_DIR}/include "C:/VulkanSDK/Include/")

set(PROJECT_LINK_LIBS OpenGL32 glfw ${BULLET_LIBS} ${VK_LIBS})
link_directories(${CMAKE_SOURCE_DIR}/build/lib/Release/ "C:/VulkanSDK/Lib/")

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c" "src/*.cc")
add_executable(rl1 ${SOURCES})
target_link_libraries(rl1 ${PROJECT_LINK_LIBS})
