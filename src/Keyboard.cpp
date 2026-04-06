#include "io/Keyboard.h"

bool Keyboard::keys[GLFW_KEY_LAST] = { 0 };
bool Keyboard::keysChanged[GLFW_KEY_LAST] = { 0 };
int Keyboard::lastKey = GLFW_KEY_LAST;

// key state call back
void Keyboard::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action != GLFW_RELEASE) {
		if (!keys[key]) {
			keys[key] = true;
		}
	}
	else {
		keys[key] = false;
	}

	keysChanged[key] = action != GLFW_REPEAT;
}

// accessors
bool Keyboard::Key(int key) {
	return keys[key];
}

bool Keyboard::KeyChanged(int key) {
	bool ret = keysChanged[key];
	keysChanged[key] = false;
	return ret;
}

bool Keyboard::KeyWentUp(int key) {
	return !keys[key] && KeyChanged(key);
}

bool Keyboard::KeyWentDown(int key) {
	return keys[key] && KeyChanged(key);
}

int Keyboard::GetLastPressedKey() {
	return lastKey;
}

void Keyboard::SetLastPressedKey(int key) {
	lastKey = key;
}