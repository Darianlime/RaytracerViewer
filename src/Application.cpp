#include "Application.h"

Application::Application() {}

int Application::Run() {
	Screen screen(1270, 720);
	if (!screen.Init()) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	ObjectFactory objectFactory;
	objectFactory.CreateFactory<MeshFactory>();
	objectFactory.CreateFactory<LightFactory>();
	objectFactory.AddMaterial(Material(Color(0.0f, 1.0f, 0.0f, false), Color(1.0f, 1.0f, 1.0f, false), Vec3(0.2f, 0.6f, 0.2f), 10));
	std::vector<std::string> sphere = { "0.0f", "0.0f", "-8.0f", "4.0", "1", "-1" };
	objectFactory.GetFactoryMap()[typeid(MeshFactory)].get()->CreateObject(string("sphere"), sphere);

	UseImGui ImGui;
	ViewportGui viewportGui(objectFactory);
	PropertiesGui propertiesGui;
	MenuGui menuGui;
	ImGui.Init(screen.GetWindow());
	while (!screen.ShouldClose()) {
		screen.NewFrame();
		screen.Update();

		ImGui.PostUpdate();
		viewportGui.PostUpdate();

		menuGui.Update(objectFactory);
		propertiesGui.Update(objectFactory);

		viewportGui.Update();

		ImGui.Render();

	}
	ImGui.Shutdown();
	viewportGui.Shutdown();
	glfwTerminate();

	return 0;
}
