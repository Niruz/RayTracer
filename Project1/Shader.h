#ifndef SHADER_H
#define SHADER_H
#include <map>
#include <string>
#include <glm/glm.hpp>
#include <fstream>
#include <iostream>
#include <glew.h>
enum SHADERS
{
	COMPUTE_SHADER,
	QUAD_SHADER,
	MAX_SHADERS
};

class Shader
{
public:
	Shader();
	~Shader();

	//Map the name of an uniform to its location.
	void setUniformLocation(const char* inName, const int inLocation);
	int  getUniformLocation(const char* inName);

	void initShader(const char* vname, const char* fname);
	void initComputeShader(const char* cname);
	void loadFile(const char* fn, std::string& str);
	unsigned int loadShader(std::string& source, unsigned int mode);
	void initUniforms();

	void bindShader();
	void unbindShader();

	int getProgram();

	int mVertexShader;
	int mFragmentShader;
	int mComputeShader;
	int mProgram;

private:
	std::map<std::string, int> mUniforms;
};

#endif