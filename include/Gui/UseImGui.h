#ifndef USING_IMGUI_H
#define USING_IMGUI_H

#include <glad/glad.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

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
