#pragma once
#include <glew.h>
class FullScreenQuad
{
public:
	FullScreenQuad();
	~FullScreenQuad();

	void render();

	GLuint mVAO;
	GLuint mVBO;

private:
};
