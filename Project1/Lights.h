#pragma once
#include "glm\glm.hpp"
#include <glew.h>

#define COLOR_WHITE glm::vec4(1.0f, 1.0f, 1.0f,1.0f)
#define COLOR_RED glm::vec4(1.0f, 0.0f, 0.0f,1.0f)
#define COLOR_GREEN glm::vec4(0.0f, 1.0f, 0.0f,1.0f)
#define COLOR_CYAN glm::vec4(0.0f, 1.0f, 1.0f,1.0f)
#define COLOR_BLUE glm::vec4(0.0f, 0.0f, 1.0f,1.0f)
#define COLOR_PURPLE glm::vec4(1.0f,0.0f,1.0f,1.0f)
#define COLOR_ORANGE glm::vec4(1.0f,0.5f,0.0f,1.0f)
#define COLOR_YELLOW glm::vec4(1.0f,1.0f,0.0f,1.0f)

class PointLight
{
public:
	PointLight();
	void setPosition(const glm::vec4& position);
	void setColor(const glm::vec4& color);
	void setRadius(float radius);
	void setAttenuationFactors(const glm::vec4& att);

private:
	glm::vec4 position;
	glm::vec4 color;
	//constant,linear,quadatric, w = 1 for symmetry in compute shader
	glm::vec4 attenuationFactors;
	GLfloat radius;
};
