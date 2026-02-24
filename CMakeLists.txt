set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_minimum_required(VERSION 3.11)

set(CMAKE_TOOLCHAIN_FILE
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file"
)

project(RaytracerViewer)

message(STATUS "VCPKG_ROOT=${VCPKG_ROOT}")

find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

add_library(imgui
    ThirdParty/imgui/imgui.cpp
    ThirdParty/imgui/imgui_draw.cpp
    ThirdParty/imgui/imgui_tables.cpp
    ThirdParty/imgui/imgui_widgets.cpp
    ThirdParty/imgui/backends/imgui_impl_glfw.cpp
    ThirdParty/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    ThirdParty/imgui
    ThirdParty/imgui/backends
)

target_compile_definitions(imgui PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)

target_link_libraries(imgui PUBLIC
    glfw
    OpenGL::GL
)

add_executable(RaytracerViewer
    src/RaytracerViewer.cpp
    src/Screen.cpp
    src/UseImGui.cpp
    src/Texture.cpp
    src/ViewportGui.cpp
    src/PropertiesGui.cpp
)

target_include_directories(RaytracerViewer PRIVATE include)

target_link_libraries(RaytracerViewer PRIVATE
    glad::glad
    glfw
    imgui
)

add_subdirectory(ThirdParty/RaytracerCore/RaytracingCore)

