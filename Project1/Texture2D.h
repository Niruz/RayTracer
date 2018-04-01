#pragma once
#include <glew.h>
struct Texture2D
{
	Texture2D(int width, int height);
	void bind();
	GLuint mFinalTexture;
};