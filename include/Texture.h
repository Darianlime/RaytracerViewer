#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Texture {
	private:
		static int currentId;

		const char* path;
		int width;
		int height;
		int nChannels;
	public:
		Texture(bool defaultParams = true);
		Texture(const char* path, const char* name, bool defaultParams = true);

		void Generate();
		void Load(bool flip = true);

		void SetTexImage(GLint internalformat, GLsizei width, GLsizei height, GLenum format, const void* data);
		void SetTexSubImage(GLsizei width, GLsizei height, GLenum format, const void* data);

		void SetFilters(GLenum all);
		void SetFilters(GLenum mag, GLenum min);

		void SetWrap(GLenum all);
		void SetWrap(GLenum s, GLenum t);

		void Bind();

		int GetWidth() const { return width; }
		int GetHeight() const { return height; }

		int id;
		unsigned int tex;
		const char* name;
};

#endif // !TEXTURE_H