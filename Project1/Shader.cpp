#include "Shader.h"

Shader::Shader() :
mVertexShader(-1),
mFragmentShader(-1),
mComputeShader(-1),
mProgram(-1)
{

}
Shader::~Shader()
{
	if (mVertexShader != -1)
	{
		glDetachShader(mProgram, mVertexShader);
		glDeleteShader(mVertexShader);
	}
	if (mFragmentShader != -1)
	{
		glDetachShader(mProgram, mFragmentShader);
		glDeleteShader(mFragmentShader);
	}
	if (mComputeShader != -1)
		glDeleteShader(mComputeShader);
	glDeleteProgram(mProgram);
}
void Shader::setUniformLocation(const char* inName, const int inLocation)
{
	mUniforms[inName] = inLocation;
}

int Shader::getUniformLocation(const char* inName)
{
	if (mUniforms.find(std::string(inName)) != mUniforms.end())
		return mUniforms[inName];
	else
		return -1;
}
void Shader::initShader(const char* vname, const char* fname)
{
	std::string source;
	loadFile(vname, source);
	mVertexShader = loadShader(source, GL_VERTEX_SHADER);
	source = "";
	loadFile(fname, source);
	mFragmentShader = loadShader(source, GL_FRAGMENT_SHADER);

	mProgram = glCreateProgram();
	glAttachShader(mProgram, mVertexShader);
	glAttachShader(mProgram, mFragmentShader);

	glLinkProgram(mProgram);
	glUseProgram(mProgram);

	///Check if the compilation went without any problems
	GLint compiled;
	glGetProgramiv(mProgram, GL_COMPILE_STATUS, &compiled);
	if (compiled)
		std::cout << "Shader compiled" << std::endl;

	GLint linked;
	glGetProgramiv(mProgram, GL_LINK_STATUS, &linked);
	if (linked)
		std::cout << "Shader linked" << std::endl;
}
void Shader::initComputeShader(const char* cname)
{
	std::string source;
	loadFile(cname, source);
	mComputeShader = loadShader(source, GL_COMPUTE_SHADER);

	mProgram = glCreateProgram();
	glAttachShader(mProgram, mComputeShader);
	glLinkProgram(mProgram);

	///Check if the compilation went without any problems
	GLint compiled;
	glGetProgramiv(mProgram, GL_COMPILE_STATUS, &compiled);
	if (compiled)
		std::cout << "Compiled compute shader\n" << std::endl;
	else
	{
		std::cout << "ERROR COMPILING THE COMPUTE SHADER\n" << std::endl;
		GLchar log[10240];
		GLsizei length;
		glGetProgramInfoLog(mProgram, 10239, &length, log);
		fprintf(stderr, "Linker log:\n%s\n", log);
	}

	GLint linked;
	glGetProgramiv(mProgram, GL_LINK_STATUS, &linked);
	if (linked)
		std::cout << "Linked compute shader\n" << std::endl;
	else
	{
		std::cout << "ERROR LINKING THE COMPUTE SHADER\n" << std::endl;
		GLchar log[10240];
		GLsizei length;
		glGetProgramInfoLog(mProgram, 10239, &length, log);
		fprintf(stderr, "Linker log:\n%s\n", log);
	}
}
void Shader::loadFile(const char* fn, std::string& str)
{
	///open up an in stream
	std::ifstream in(fn);
	///If it fails to open, return
	if (!in.is_open())
	{
		return;
	}
	///Temporary char to hold a line
	char tmp[300];
	while (!in.eof())
	{
		///Get the line
		in.getline(tmp, 300);
		///Read in the the line to the string
		str += tmp;
		str += '\n';
	}
}
unsigned int Shader::loadShader(std::string& source, unsigned int mode)
{
	///id used by opengl to reference the shader object
	unsigned int id;
	///Gl create shader creates a shader object with a certain specified shadertype
	id = glCreateShader(mode);
	const char* csource = source.c_str();
	///glShaderSource sets the source code in shader to the source code in the array of strings specified by string, number of strings is the second parameter (count)
	glShaderSource(id, 1, &csource, NULL);
	///Compiles the shader and used the reference to find it
	glCompileShader(id);

	///Check if there were any errors
	char error[1000];
	glGetShaderInfoLog(id, 1000, NULL, error);
	std::cout << "Compile Status: \n" << error << std::endl;

	return id;
}
void Shader::initUniforms()
{
	int numberOfUniforms = -1;
	int nameLength = -1, uniformSize = -1;
	GLenum type = 0;
	char name[32];
	int location = -1;

	glGetProgramiv(mProgram, GL_ACTIVE_UNIFORMS, &numberOfUniforms);
	for (int uniformIt = 0; uniformIt < numberOfUniforms; uniformIt++)
	{
		glGetActiveUniform(mProgram, uniformIt, sizeof(name) - 1, &nameLength, &uniformSize, &type, name);
		location = -1;
		location = glGetUniformLocation(mProgram, name);
		setUniformLocation(name, location);
	}
	std::cout << std::endl;
}
void Shader::bindShader()
{
	glUseProgram(mProgram);
}
void Shader::unbindShader()
{
	glUseProgram(0);
}
int Shader::getProgram()
{
	return mProgram;
}