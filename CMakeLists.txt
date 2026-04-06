cmake_minimum_required(VERSION 3.26)

# Explicitly set vcpkg toolchain
set(CMAKE_TOOLCHAIN_FILE
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file"
)

# Project setup
project(RaytracerViewer)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Print vcpkg root
message(STATUS "VCPKG_ROOT=$ENV{VCPKG_ROOT}")

# Find packages
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# Confirm targets found
if(TARGET glfw)
    message(STATUS "GLFW target found ✅")
else()
    message(FATAL_ERROR "GLFW NOT found ❌")
endif()

if(TARGET glad::glad)
    message(STATUS "GLAD target found ✅")
else()
    message(FATAL_ERROR "GLAD NOT found ❌")
endif()

# -----------------------
# ImGui library
# -----------------------
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
    glad::glad
    glfw
    OpenGL::GL
)

# -----------------------
# RaytracerCore
# -----------------------
set(RAYTRACER_BUILD_APP OFF CACHE BOOL "" FORCE)
add_subdirectory(ThirdParty/RaytracerCore/RaytracingCore)

# -----------------------
# Main application
# -----------------------
add_executable(RaytracerViewer
    src/main.cpp
    src/Screen.cpp
    src/UseImGui.cpp
    src/ScreenTexture.cpp
    src/ViewportGui.cpp
    src/PropertiesGui.cpp
    src/MenuGui.cpp
    src/Application.cpp
    src/Keyboard.cpp
)

# Explicit include directories for VS
target_include_directories(RaytracerViewer PRIVATE include)

# Link libraries
target_link_libraries(RaytracerViewer PRIVATE
    lib
    imgui
)