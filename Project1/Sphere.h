#pragma once

#include "glm\glm.hpp"

struct Sphere
{
	Sphere()
	{

	};
	void setPosition(const glm::vec4& position)
	{
		this->position = position;
	}
	void setColor(const glm::vec4& color)
	{
		this->color = color;
	}
	void setRadius(float rad)
	{
		this->radius = rad;
	}
	void setReflective(int r)
	{
		reflective = r;
	}

	glm::vec4 position;
	glm::vec4 color;
	float radius;
	//0 no, 1 reflective, 2 reflective and refractive
	int reflective;
};