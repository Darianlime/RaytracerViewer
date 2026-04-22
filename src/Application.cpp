#include "Application.h"
#include "io/Keyboard.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

Application::Application() {}

int Application::Run() {
	Screen screen(1270, 720);
	if (!screen.Init()) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	ObjectFactory objectFactory;
	objectFactory.AddCamera(Raytracer::Camera(Vec3(0.0f, 0.0f, -10.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, 1.0f, 0.0f), 45.0f));
	objectFactory.AddMaterial(Material(Color(0.2f, 1.0f, 0.2f, false), Color(1.0f, 1.0f, 1.0f, false), Vec3(0.2f, 0.6f, 0.0f), 100, Color(1.0f, 1.0f, 1.0f, false), 1.0f));
	objectFactory.AddMaterial(Material(Color(0.2f, 0.2f, 1.0f, false), Color(1.0f, 1.0f, 1.0f, false), Vec3(0.2f, 0.6f, 0.5f), 100, Color(0.4f, 0.4f, 0.4f, false), 1.0f));
	objectFactory.AddMaterial(Material(Color(0.0f, 1.0f, 0.0f, false), Color(1.0f, 1.0f, 1.0f, false), Vec3(0.02f, 0.1f, 0.2f), 100, Color(0.9f, 0.2f, 0.9f, false), 1.6f));

	std::vector<std::string> sphere = { "0.0f", "0.0f", "0.0f", "1", "2", "-1" };
	objectFactory.GetFactoryMap()[typeid(ModelFactory)].get()->CreateObject(string("sphere"), sphere);
	//std::vector<std::string> sphere1 = { "-2.0f", "0.0f", "3.0f", "1.0f", "0", "-1" };
	//objectFactory.GetFactoryMap()[typeid(ModelFactory)].get()->CreateObject(string("sphere"), sphere1);
	//std::vector<std::string> sphere2 = { "2.0f", "0.0f", "3.0f", "1.0f", "1", "-1" };
	//objectFactory.GetFactoryMap()[typeid(ModelFactory)].get()->CreateObject(string("sphere"), sphere2);
	std::vector<std::string> light = { "-3.0f", "15.0f", "-10.0f", "1", "1", "1", "1"};
	objectFactory.GetFactoryMap()[typeid(LightFactory)].get()->CreateObject(string("light"), light);

	UseImGui ImGui;
	ViewportGui viewportGui(objectFactory);
	PropertiesGui propertiesGui;
	HWND hwnd = glfwGetWin32Window(screen.GetWindow());
	MenuGui menuGui(hwnd);
	ImGui.Init(screen.GetWindow());
	while (!screen.ShouldClose()) {
		if (Keyboard::Key(GLFW_KEY_ESCAPE)) {
			printf("in escape\n");
			screen.SetShouldClose(true);
		}

		screen.NewFrame();
		screen.Update();

		ImGui.PostUpdate();
		viewportGui.PostUpdate();

		menuGui.Update(objectFactory);
		propertiesGui.Update(objectFactory);
		viewportGui.PropertiesUpdate(propertiesGui.IsUpdating() | menuGui.IsUpdating(), propertiesGui.GetUpdatingIndex());

		viewportGui.Update();


		ImGui.Render();

	}
	ImGui.Shutdown();
	glfwTerminate();

	return 0;
}
