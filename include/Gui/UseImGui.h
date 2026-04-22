#ifndef USING_IMGUI_H
#define USING_IMGUI_H

#include <glad/glad.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include "Factory/ObjectFactory.h"

enum UpdateType {
	NONE = 0,
	PROPERTIES_OBJECTS = 1 << 0,
	PROPERTIES_CAMERA = 1 << 1,
	DELETING_MODEL = 1 << 2,
	DELETING_LIGHT = 1 << 3,
	DELETING_MATERIAL = 1 << 4
};

class UseImGui {
	private:
		ImVec4 color;
	public:
		void Init(GLFWwindow* window);
		virtual void PostUpdate();
		virtual void Update();
		void Render();
		void Shutdown();
};

#endif // USING_IMGUI_H
