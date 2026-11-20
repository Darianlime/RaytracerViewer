# Raytracer Viewer
**Tech:** C++17, ImGui, OpenGL, GLFW

A multithreaded CPU raytracer with a real-time tiled viewport and an ImGui editor for building scenes, animating them on a timeline, and exporting MP4 video.

## Editor Demo
[https://github.com/user-attachments/assets/ac68a6d6-1bcf-4ee2-9978-70c0e0fc3270](https://github.com/user-attachments/assets/ea45aa6f-73f4-4dc6-a1ed-3c7dd3c06c4e)

## Render Demo
https://github.com/user-attachments/assets/642f838e-a7b8-49f8-9415-d003e54f9cba

## Features

**Rendering**
- Multithreaded CPU raytracer with tile-based work distribution on a persistent thread pool
- Blinn-Phong shading with multiple lights and distance attenuation
- Hard shadows with colored, partially transparent occluders (Beer-Lambert absorption)
- Reflection and refraction (Snell's law, Fresnel, total internal reflection)
- Primitives: sphere, ellipsoid, cylinder, cone, and triangle meshes
- OBJ import with a BVH acceleration structure and optional smooth shading
- Fixed render resolution independent of the window size

**Editor**
- ImGui interface: viewport, object list, material and transform properties
- Live viewport that re-renders on camera, material, or object changes
- Free-fly camera
- Create and delete objects, lights, and materials at runtime

**Animation**
- Keyframe timeline for position, rotation, and scale with linear interpolation
- Playback in the viewport
- Export to MP4 through FFmpeg (H.264), with selectable resolution and frame rate

## Controls

**Camera (hover the viewport)**

| Input | Action |
|---|---|
| Hold Right Mouse + move mouse | Look around |
| W / A / S / D | Move forward / left / back / right |
| E / Q | Move up / down |
| Shift | Move faster |

**Scene**

| Input | Action |
|---|---|
| Click an object in the Objects panel | Select it and show its properties |
| Right-click an object | Delete it |
| Objects menu | Add a new object or light |
| File > Import OBJ | Load a mesh |
| File > Export Animation | Render the timeline to an MP4 |

**Timeline**

| Input | Action |
|---|---|
| Select an object, then a channel (Position / Rotation / Scale) | Choose what to key |
| **I** | Insert a keyframe at the current frame (or overwrite the existing one) |
| Drag a keyframe | Move it to another frame |
| **Delete** | Delete the selected keyframes |
| Play / Pause button | Start or stop playback |
| `<` / `>` buttons | Step one frame back or forward |
| Drag the playhead | Scrub to any frame |

Tip: set the object's transform in the Properties panel first, move the playhead to the frame you want, then press **I**.

## Build (Windows x64)

> **Want to skip building from source?** You can download a prebuilt version from the [Releases](https://github.com/Darianlime/RaytracerViewer/releases) page.

### Prerequisites
- Visual Studio 2022 or 2026 with the C++ workload
- CMake 3.26+
- Git
- FFmpeg with libx264 (only needed for MP4 export)

### Steps

1. **Clone the repository (with submodules)**
   ```bash
   git clone --recurse-submodules https://github.com/Darianlime/RaytracerViewer.git
   ```
2. **Bootstrap vcpkg**
   ```bash
   cd RaytracerViewer/ThirdParty/vcpkg
   bootstrap-vcpkg.bat
   ```
3. **Install FFmpeg (for animation export)**
   ```bash
   winget install -e --id Gyan.FFmpeg
   ```
   Restart your terminal and Visual Studio afterward so `ffmpeg` is on your PATH.
4. **Open the project in Visual Studio**
5. **Build**
   - Select the `x64-Release` CMake preset in the top toolbar
   - Press `Ctrl + Shift + B`
6. **Run**
   - Press `Ctrl + F5`

## Credits and Third-Party Libraries

| Library | Use | License |
|---|---|---|
| [Dear ImGui](https://github.com/ocornut/imgui) | Editor UI | MIT |
| [imgui_neo_sequencer](https://github.com/dupasquale/imgui_neo_sequencer) | Timeline and keyframe widget | MIT |
| [GLFW](https://www.glfw.org/) | Window and input | zlib/libpng |
| [glad](https://github.com/Dav1dde/glad) | OpenGL loader | MIT |
| [vcpkg](https://github.com/microsoft/vcpkg) | Dependency management | MIT |
| [FFmpeg](https://ffmpeg.org/) | MP4 encoding, run as an external program (not bundled) | LGPL/GPL, depending on build |
| [x264](https://www.videolan.org/developers/x264.html) | H.264 encoder used by FFmpeg | GPL |

**3D Models**
- Stanford Bunny
- Stanford Dragon
- Stanford Buddha

**Source:** [The Stanford 3D Scanning Repository – Stanford Computer Graphics Laboratory](https://graphics.stanford.edu/data/3Dscanrep/)

This project is licensed under the [MIT License](LICENSE). Third-party libraries keep their own licenses.
