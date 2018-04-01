#pragma once

#include "glm\glm.hpp"

struct Triangle
{
	Triangle(const glm::vec4& p0, const glm::vec4& p1, const glm::vec4& p2, const glm::vec4& tuv)
		: p0(p0), p1(p1), p2(p2), tuv(tuv)
	{

	};

	Triangle()
	{
		tuv = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	void setPositions(const glm::vec4& p0, const glm::vec4& p1, const glm::vec4& p2)
	{
		this->p0 = p0;
		this->p1 = p1;
		this->p2 = p2;
	}
	void setColor(const glm::vec4& col)
	{
		this->color = col;
	}

	void setReflective(int r)
	{
		reflective = r;
	}

	glm::vec4 p0;
	glm::vec4 p1;
	glm::vec4 p2;
	glm::vec4 tuv;
	glm::vec4 color;
	int reflective;
};