# Raytracer Viewer
**Tech:** C++17, ImGui, OpenGL, GLFW

## Overview
Multithreaded C++ raytracer with real-time tiling rendering and an ImGui GUI to add objects or change properties

## Demo

## Features
- Blinn-Phong lighting model
- Multiple geometry types (sphere, plane, ...)
- Tiling-based multithreaded rendering via persistent thread pool
- Live progressive viewport with ImGui controls

## Build (Windows x64)
### Prerequisites
- Visual Studio 2022 or 2026 with C++ workload installed
- CMake 3.26+
- Git
  
1. **Clone the repository (with submodules)**  
   ```bash
   git clone --recurse-submodules https://github.com/Darianlime/RaytracerViewer.git
2. **Bootstrap vcpkg:**
   ```bash
   cd RaytracerViewer/ThirdParty/vcpkg
   bootstrap-vcpkg.bat
3. **Open the project in Visual Studio**
4. Build the project
    - Select the x64-Release CMake preset (top toolbar)
    - Press Ctrl + Shift + B
5. Run the application
    - Press Ctrl + F5 or click Run

