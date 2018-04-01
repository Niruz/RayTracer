#version 430 

in vec2 v2fTexcoords;

layout(binding = 0) uniform sampler2D diffuseTexture;

uniform bool renderDepth;

layout(location = 0) out vec4 color;

void main()
{
	if(renderDepth)
		color = vec4(vec3(texture2D(diffuseTexture,v2fTexcoords).r),1.0f);
	else
		color = vec4(texture2D(diffuseTexture,v2fTexcoords).xyz,1.0f);
}