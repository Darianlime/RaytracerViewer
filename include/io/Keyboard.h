#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Keyboard {
	private:
		static bool keys[];
		static bool keysChanged[];
		static int lastKey;
	public:
		// key state call back
		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

		// accessors
		static bool Key(int key);
		static bool KeyChanged(int key);
		static bool KeyWentUp(int key);
		static bool KeyWentDown(int key);
		static int GetLastPressedKey();
		static void SetLastPressedKey(int key);
};
#endif // !KEYBOARD_H