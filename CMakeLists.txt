
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

include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/build/lib/ )
set(PROJECT_LINK_LIBS OpenGL32 glfw)

if(0)
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /MTd /Zi")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /MT /arch:AVX2 /fp:fast")
    set(CMAKE_CXX_FLAGS "/W3 /MP /GR-")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
    set(CMAKE_CXX_FLAGS "-march=native -ffast-math -std=c++11 -Wfatal-errors -pthread -fno-exceptions -fno-rtti -fopenmp")
endif()

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c" "src/*.cc")
add_executable(rl1 ${SOURCES})
target_link_libraries(rl1 ${PROJECT_LINK_LIBS})
