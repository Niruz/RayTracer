#pragma once
#include "glm\glm.hpp"

struct Plane
{
	Plane();
	void setPosition(const glm::vec4& position)
	{
		this->position = position;
		this->normal = glm::normalize(position - glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	}
	void setColor(const glm::vec4& color)
	{
		this->color = color;
	}

	glm::vec4 position;
	glm::vec4 color;
	glm::vec4 normal;
};