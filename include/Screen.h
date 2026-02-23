#ifndef SCREEN_H
#define SCREEN_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Screen {
	private:
		GLFWmonitor* monitor;
		const GLFWvidmode* mode;
		GLFWwindow* window;
	public:
		static unsigned int SCR_WIDTH;
		static unsigned int SCR_HEIGHT;

		GLFWwindow* GetWindow() const { return window; }

		static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

		Screen();
		Screen(unsigned int scr_width, unsigned int scr_height);

		bool Init();

		void GetMonitorSizeGLFW(GLFWmonitor* monitor, const GLFWvidmode* mode, unsigned int& width, unsigned int& height);

		void SetParameters();

		//main loop
		void Update();
		void NewFrame();

		bool ShouldClose();
		void SetShouldClose(bool shouldClose);
};

#endif // !SCREEN_H
