#include "Lights.h"

PointLight::PointLight()
	:position(0.0f, 0.0f, 0.0f, 1.0f), color(COLOR_WHITE), radius(10.0f)
{

}
void PointLight::setPosition(const glm::vec4& position)
{
	this->position = position;
}
void PointLight::setColor(const glm::vec4& color)
{
	this->color = color;
}
void PointLight::setRadius(float radius)
{
	this->radius = radius;
}
void PointLight::setAttenuationFactors(const glm::vec4& att)
{
	this->attenuationFactors = att;
}