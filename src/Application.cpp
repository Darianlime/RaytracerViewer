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
	objectFactory.AddCamera(Raytracer::Camera(Vec3(0.0f, 4.4f, 9.4f), Vec3(0.0f, -0.342f, -0.940f), Vec3(0.0f, 1.0f, 0.0f), 45.0f));
	objectFactory.AddMaterial(Material(Color(0.8f, 0.1f, 0.1f, false), Color(0.0f, 0.0f, 0.0f, false), Vec3(0.15f, 0.8f, 0.0f), 10, Color(1.0f, 1.0f, 1.0f, false), 1.0f));
	objectFactory.AddMaterial(Material(Color(0.85f, 0.40f, 0.15f, false), Color(1.0f, 1.0f, 1.0f, false), Vec3(0.15f, 0.7f, 0.4f), 60, Color(1.0f, 1.0f, 1.0f, false), 1.5f));
	objectFactory.AddMaterial(Material(Color(0.9f, 0.9f, 0.94f, false), Color(1.0f, 1.0f, 1.0f, false), Vec3(0.02f, 0.05f, 0.3f), 200, Color(1.0f, 1.0f, 1.0f, false), 6.0f));
	objectFactory.AddMaterial(Material(Color(0.9f, 0.94f, 1.0f, false), Color(1.0f, 1.0f, 1.0f, false), Vec3(0.02f, 0.1f, 0.6f), 100, Color(0.05f, 0.05f, 0.05f, false), 1.5f));

	std::vector<std::string> sphere = { "0.0f", "0.7f", "-3.5f", "1", "0", "-1" };
	objectFactory.GetFactoryMap()[typeid(ModelFactory)].get()->CreateObject(string("sphere"), sphere);
	//std::vector<std::string> sphere1 = { "-2.0f", "0.0f", "3.0f", "1.0f", "0", "-1" };
	//objectFactory.GetFactoryMap()[typeid(ModelFactory)].get()->CreateObject(string("sphere"), sphere1);
	//std::vector<std::string> sphere2 = { "2.0f", "0.0f", "3.0f", "1.0f", "1", "-1" };
	//objectFactory.GetFactoryMap()[typeid(ModelFactory)].get()->CreateObject(string("sphere"), sphere2);
	std::vector<std::string> light = { "-6.0f", "10.0f", "6.0f", "1", "1", "1", "1"};
	objectFactory.GetFactoryMap()[typeid(LightFactory)].get()->CreateObject(string("light"), light);

	UpdateGUIState updateGUIState{};
	UseImGui ImGui;
	ViewportGui viewportGui(objectFactory, updateGUIState);
	PropertiesGui propertiesGui(updateGUIState);
	TimelineGui timelineGui(objectFactory, updateGUIState);
	HWND hwnd = glfwGetWin32Window(screen.GetWindow());
	MenuGui menuGui(hwnd, objectFactory, viewportGui,timelineGui, updateGUIState);

	menuGui.CreatePlane();

	timelineGui.isRenderIdle = [&] { return viewportGui.IsRenderingDone(); };
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

		updateGUIState.updateType = UpdateType::NONE;

		menuGui.Update();
		propertiesGui.Update(objectFactory);
		timelineGui.Update();
		viewportGui.Update();

		ImGui.Render();

	}
	ImGui.Shutdown();
	glfwTerminate();

	return 0;
}
