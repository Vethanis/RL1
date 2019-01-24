# RL1

A toy engine for testing and implementing engine and rendering techniques.

## **Video**
[![](https://i.imgur.com/d03TeoN.png)](https://vimeo.com/313102703)

## **Details**

Currently implemented:

* Image based lighting
* Entity Component System
* Generational indices / handles
* Frame lifetime linear memory allocator
* Constructive Solid Geometry via signed distance functions and marching cubes
* Constructive Solid Geometry via Binary Space Partitioning trees
* Control abstraction via action / axis binding
* Multithreaded task system

Third party libraries / technologies in use:

* GLFW
* Glad
* GLM
* PCG32
* FNV1a
* Dear ImGui
* Bullet physics
* STB libraries
* sokol_time

Planned features or techniques:

* Switch to vulkan rendering backend
* Integrate use of AMD's vulkan memory allocator
* Experiment further with parser combinators, bytecode VMs, and scripting systems
* Audio system


Currently only supports builds using MSVC and CMake
