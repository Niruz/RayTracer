#include "Texture2D.h"
#include <string>
Texture2D::Texture2D(int width, int height)
{
	glGenTextures(1, &mFinalTexture);
	glBindTexture(GL_TEXTURE_2D, mFinalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}
void Texture2D::bind()
{
	glBindImageTexture(0, mFinalTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
}