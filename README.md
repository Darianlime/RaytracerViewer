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
- Command-line benchmark mode for reproducible frame-time measurements (see [Benchmarks](#benchmarks))

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

## Benchmarks

A command-line benchmark mode renders fixed scenes without the UI and reports frame times. It measures render time only. It does not include the UI or presenting the frame to the screen.

### Scene

Every run uses the same fixed scene so results can be compared:

- 640x360 resolution, one model, no floor or other objects.
- The camera is placed from the model's bounding sphere, so every mesh is framed the same way regardless of its size in world units.

| Pose | View | Bounding sphere spans | Purpose |
|---|---|---|---|
| 1 | Front, slightly above | about 90% of the frame height | Close view, the largest share of pixels hit the mesh |
| 2 | Three-quarter | about 60% of the frame height | Typical view |
| 3 | Front, far away | about 20% of the frame height | Most rays miss the mesh |

- Lights are point lights placed relative to the model's bounding radius. `--lights 1` uses a key light only, and `--lights 3` adds a fill light and a back light.

### Method

- Warm-up frames are discarded (5, or 1 when fewer than 10 frames are requested), then the timed frames are measured with `std::chrono::steady_clock`.
- The report gives the median and p95 over the timed frames. BVH build time is measured separately.
- The final frame is written to `Benchmarks/Renders/` as a PPM image, which is how correctness is checked across settings.

### Run it

Use a Release build and run benchmarks from a terminal outside the debugger. Benchmark mode renders a fixed scene without opening the editor.

**Single benchmark (Release build)**

If you downloaded the prebuilt Release ZIP, open a terminal in the folder containing `RaytracerViewer.exe` and run:

```powershell
.\RaytracerViewer.exe --bench --mesh buddha.obj --threads 10 --pose 2 --lights 1 --frames 30
```

If you built from source, run the executable from `out/build/x64-Release/Release`, or specify its path directly:

```powershell
$exe = ".\RaytracerViewer\out\build\x64-Release\Release\RaytracerViewer.exe"
& $exe --bench --mesh buddha.obj --threads 10 --pose 2 --lights 1 --frames 30
```

`--mesh` takes the name of an OBJ file in the `assets/` folder.

| Option | Meaning | Default |
|---|---|---|
| `--mesh` | OBJ file in `assets/` | `dragon.obj` |
| `--threads` | Render worker threads | 10 |
| `--bvh` | `1` = BVH, `0` = brute force (root bounding box check, then every triangle) | 1 |
| `--pose` | Camera pose 1 to 3 | 1 |
| `--lights` | Number of lights, 1 or 3 | 1 |
| `--frames` | Timed frames | 30 |
| `--block` | Tile size in pixels | 16 |

It prints one line per run:

```text
mesh=buddha.obj tris=1087474 threads=10 bvh=1 res=640x360 build=530ms median=8.1ms p95=8.7ms (124.1 fps)
```

**Automated benchmarks (PowerShell)**

The included PowerShell script automates the thread-scaling and BVH benchmarks. From the project root, run:

```powershell
.\Benchmarks\benchmark.ps1
```

To save the output to `Benchmarks/benchmark_results.txt` while displaying it in the terminal, append results using:

```powershell
.\Benchmarks\benchmark.ps1 | Tee-Object .\Benchmarks\benchmark_results.txt -Append
```

The script automatically locates the executable and runs the predefined benchmark configurations.

### Results

**Machine:** [CPU model, RAM, OS]. All results use 16 px tiles and a Release build. Thread counts are render worker threads.

**Thread scaling** (Stanford dragon, 202,520 triangles, pose 2, 1 light, median frame time over 30 frames, two separate runs):

| Threads | Run 1 | Run 2 |
|---|---|---|
| 1 | 47.1 ms | 46.5 ms |
| 2 | 24.0 ms | 23.9 ms |
| 4 | 14.3 ms | 14.1 ms |
| 8 | 8.7 ms | 8.9 ms |
| 10 | 8.7 ms | 7.9 ms |

That is about a 5.4x speedup from 1 to 10 threads in run 1 and 5.9x in run 2.

**Mesh size** (10 threads, 1 light):

| Mesh | Triangles | Pose 2 | Pose 1 |
|---|---|---|---|
| Bunny | 144,046 | 5.8 ms | not measured |
| Dragon | 202,520 | 7.9 to 8.7 ms | not measured |
| Buddha | 1,087,474 | 8.0 to 8.1 ms | 12.7 to 13.0 ms |

**BVH versus brute force** (bunny, 144,046 triangles, pose 2, 10 threads):

| | Median frame time |
|---|---|
| BVH | 5.8 ms |
| Brute force (`--bvh 0`) | 9,876.7 ms (3 timed frames) |

The BVH is about 1,700x faster. The two runs wrote byte-identical images (matching file hashes), so the BVH changes speed but not the result.

### Reading the numbers

- **Cost depends on how much of the frame the mesh covers.** Moving the interactive camera very close, so the model fills the whole viewport, reached roughly 40 ms per frame in the viewer. That is why the benchmark fixes the poses.
- **Run-to-run variation is about 5 to 10%.** Close other programs before benchmarking, and compare several runs.
- **Scaling flattens above 8 threads.** The cause has not been investigated. Possible causes include memory latency in BVH traversal and hyperthreading.

### Possible improvements

- Each BVH leaf stores a vector of triangle indices. Storing each leaf's triangles contiguously should reduce cache misses during traversal.
- The BVH uses a midpoint split. A surface area heuristic split would likely give better trees.

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
