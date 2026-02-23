#include "ViewportGui.h"
#include "Screen.h"

#include <iostream>

using namespace std;

int main()
{
	Screen screen(1270, 720);
	if (!screen.Init()) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	UseImGui ImGui;
	ViewportGui viewportGui;
	ImGui.Init(screen.GetWindow());
	while (!screen.ShouldClose()) {
		screen.NewFrame();
		screen.Update();

		ImGui.PostUpdate();
		viewportGui.PostUpdate();

		ImGui.Update();
		viewportGui.Update();

		ImGui.Render();

	}
	ImGui.Shutdown();
	viewportGui.Shutdown();
	glfwTerminate();
	
	cout << "Hello CMake." << endl;
	return 0;
}
