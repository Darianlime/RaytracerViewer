#include "Texture.h"

int Texture::currentId = 0;

Texture::Texture(bool defaultParams) : id(currentId++) {
	Generate();

	if (defaultParams) {
		SetFilters(GL_NEAREST, GL_NEAREST);
		SetWrap(GL_CLAMP_TO_EDGE);
	}
}

Texture::Texture(const char* path, const char* name, bool defaultParams) : path(path), name(name), id(currentId++) {
	Generate();

	if (defaultParams) {
		SetFilters(GL_NEAREST, GL_NEAREST);
		SetWrap(GL_CLAMP_TO_EDGE);
	}
}

void Texture::Generate() {
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
}

void Texture::Load(bool flip) {
//	stbi_set_flip_vertically_on_load(flip);
//
//	unsigned char* data = stbi_load(path, &width, &height, &nChannels, 0);
//
//	if (!data) {
//		std::cerr << "Failed to load texture: " << path
//			<< "\nReason: " << stbi_failure_reason() << std::endl;
//		return;
//	}
//	else {
//		std::cerr << "loaded texture: " << path << std::endl;
//	}
//	GLenum colorMode = GL_RGB;
//	if (nChannels == 1) {
//		colorMode = GL_RED;
//	}
//	else if (nChannels == 3) {
//		colorMode = GL_RGB;
//	}
//	else if (nChannels == 4) {
//		colorMode = GL_RGBA;
//	}
//	if (data) {
//		glBindTexture(GL_TEXTURE_2D, id);
//		glTexImage2D(GL_TEXTURE_2D, 0, colorMode, width, height, 0, colorMode, GL_UNSIGNED_BYTE, data);
//		glGenerateMipmap(GL_TEXTURE_2D);
//	}
//	else {
//		std::cout << "failed to load texture" << std::endl;
//	}
//	stbi_image_free(data);
}

void Texture::SetTexImage(GLint internalformat, GLsizei width, GLsizei height, GLenum format, const void* data) {
	Bind();
	glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
}

void Texture::SetTexSubImage(GLsizei width, GLsizei height, GLenum format, const void* data) {
	Bind();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
}

void Texture::SetFilters(GLenum all) {
	SetFilters(all, all);
}

void Texture::SetFilters(GLenum mag, GLenum min) {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);
}

void Texture::SetWrap(GLenum all) {
	SetWrap(all, all);
}

void Texture::SetWrap(GLenum s, GLenum t) {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, t);
}

void Texture::Bind() {
	glBindTexture(GL_TEXTURE_2D, tex);
}
