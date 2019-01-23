
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
add_subdirectory(bullet3-2.87)

if(WIN32)
    set(VK_LIBS
        vulkan-1.lib)
    set(BULLET_LIBS 
        Bullet3Collision.lib
        Bullet3Common.lib
        Bullet3Dynamics.lib
        Bullet3Geometry.lib
        BulletCollision.lib
        BulletDynamics.lib
        BulletInverseDynamics.lib
        LinearMath.lib)

    set(BULLET_DEBUG_LIBS
        Bullet3Collision_Debug.lib
        Bullet3Common_Debug.lib
        Bullet3Dynamics_Debug.lib
        Bullet3Geometry_Debug.lib
        BulletCollision_Debug.lib
        BulletDynamics_Debug.lib
        BulletInverseDynamics_Debug.lib
        LinearMath_Debug.lib)
        
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /MTd /Zi")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /MT /arch:AVX2 /fp:fast")
    set(CMAKE_CXX_FLAGS "/W3 /MP /GR-")
    include_directories(${CMAKE_SOURCE_DIR}/include "C:/VulkanSDK/Include/")

    set(PROJECT_LINK_LIBS OpenGL32 glfw ${BULLET_LIBS} ${VK_LIBS})
    link_directories(${CMAKE_SOURCE_DIR}/build/lib/Release/ "C:/VulkanSDK/Lib/")

    if(0)
        set(PROJECT_LINK_LIBS OpenGL32 glfw ${BULLET_DEBUG_LIBS} ${VK_LIBS})
        link_directories(${CMAKE_SOURCE_DIR}/build/lib/Debug/ )
    endif()

elseif(UNIX)
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
    set(CMAKE_CXX_FLAGS "-Wall -std=c++11 -Wfatal-errors -pthread -fno-exceptions -fno-rtti")
    include_directories("/usr/include")
    link_directories("/usr/lib")
    set(PROJECT_LINK_LIBS GL glfw3)
endif()

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c" "src/*.cc")
add_executable(rl1 ${SOURCES})
target_link_libraries(rl1 ${PROJECT_LINK_LIBS})
