#pragma once
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")

#include <GL\glew.h>
#include <glfw3.h>


#include <stdio.h>  
#include <stdlib.h>  
#include <stdarg.h> 

// glm::vec3, glm::vec4, glm::ivec4, glm::mat4
#include <glm/glm.hpp>
// glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/matrix_transform.hpp>
// glm::value_ptr
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <glm/glm.hpp> // vec3 normalize reflect dot pow
#include <glm\gtx\vector_angle.hpp>

#include "Shader.h"
#include "Texture2D.h"
#include "FullScreenQuad.h"
#include "Lights.h"
#include "Sphere.h"
#include "Triangle.h"
#include "Camera.h"
float mHeight = 720.0f;
float mWidth = 1280.0f;
float aspectRatio = mWidth / mHeight;
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
bool keys[1024] = { false };
Shader mShaders[MAX_SHADERS];
Texture2D* mTexture;
FullScreenQuad* mScreenQuad;
GLuint mLightBuffer;
GLuint mSphereBuffer;
GLuint mTriangleBuffer;
bool firstMouse = true;
Camera mCamera;

GLfloat lastX = 640.0f;
GLfloat lastY = 360.0f;
glm::vec3 cameraPos = glm::vec3(0.0f);

#define MAX_NUM_OF_LIGHTS 1
#define MAX_NUM_OF_SPHERES 4
#define MAX_NUM_OF_TRIANGLES 8

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
	_fgetchar();
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	//cout << key << endl;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}
static void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left

	lastX = xpos;
	lastY = ypos;

	mCamera.processMouseMovement(xoffset, yoffset);
}
void updateInput(GLfloat deltaTime)
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		mCamera.processKeyBoard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		mCamera.processKeyBoard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		mCamera.processKeyBoard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		mCamera.processKeyBoard(RIGHT, deltaTime);
	if (keys[GLFW_KEY_SPACE])
		mCamera.processKeyBoard(UP, deltaTime);
	if (keys[GLFW_KEY_LEFT_CONTROL])
		mCamera.processKeyBoard(DOWN, deltaTime);
}
void renderFullScreenQuad(Shader& shader, bool renderDepth)
{
	glUseProgram(shader.mProgram);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUniform1i(shader.getUniformLocation("diffuseTexture"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mTexture->mFinalTexture);
	mScreenQuad->render();
}

void initializePointLights()
{
	glGetError();
	glm::vec4 currentColor(1.0f, 1.0f, 1.0f, 1.0f);
	glGenBuffers(1, &mLightBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mLightBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_NUM_OF_LIGHTS * sizeof(struct PointLight), NULL, GL_STATIC_DRAW);
	struct PointLight* pointlights = (struct PointLight*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, MAX_NUM_OF_LIGHTS*sizeof(struct PointLight), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (unsigned int i = 0; i < MAX_NUM_OF_LIGHTS /*- 1 <-- wtf?*/; ++i)
	{
		float Max = 50.0f;
		float Min = -50.0f;

		float MaxY = 7.0f;
		float MinY = 3.0f;

		float MaxX = 10.0f;
		float MinX = -10.0f;

		float ranx = ((float(rand()) / float(RAND_MAX)) * (MaxX - MinX)) + MinX;
		float ranz = ((float(rand()) / float(RAND_MAX)) * (Max - Min)) + Min;
		float rany = 5.0f;

		int maxCol = 8;
		int minCol = 1;
		int ranCol = (rand() % (maxCol - minCol)) + minCol;

		if (ranCol == 0)
			printf("error, color 8 doesnt exist");
		if (ranCol == 1)
			currentColor = COLOR_WHITE;
		if (ranCol == 2)
			currentColor = COLOR_RED;
		if (ranCol == 3)
			currentColor = COLOR_GREEN;
		if (ranCol == 4)
			currentColor = COLOR_CYAN;
		if (ranCol == 5)
			currentColor = COLOR_BLUE;
		if (ranCol == 6)
			currentColor = COLOR_PURPLE;
		if (ranCol == 7)
			currentColor = COLOR_ORANGE;
		if (ranCol == 8)
			printf("error, color 8 doesnt exist");

		pointlights[i].setPosition(glm::vec4(ranx, rany, ranz, 1.0f));
		pointlights[i].setRadius(25.0f);
		pointlights[i].setColor(currentColor);
		//pointlights[i].setAttenuationFactors(glm::vec4(1.0f, 0.014f,	0.0007f, 1.0f));
		pointlights[i].setAttenuationFactors(glm::vec4(1.0f, 0.00014f, 0.000007f, 1.0f));
	}
	pointlights[0].setPosition(glm::vec4(0.0f, 20.0f, 0.0f, 1.0f));
	//pointlights[0].setRadius(2500.0f);
	pointlights[0].setColor(COLOR_WHITE);
	pointlights[0].setRadius(25.0f);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	bool test = glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}
void initializeSpheres()
{
	glGetError();
	glGenBuffers(1, &mSphereBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSphereBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_NUM_OF_SPHERES * sizeof(struct Sphere), NULL, GL_STATIC_DRAW);
	struct Sphere* spheres = (struct Sphere*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, MAX_NUM_OF_SPHERES*sizeof(struct Sphere), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (unsigned int i = 0; i < MAX_NUM_OF_SPHERES /*- 1 <-- wtf?*/; ++i)
	{
		float Max = 50.0f;
		float Min = -50.0f;

		float MaxY = 7.0f;
		float MinY = 3.0f;

		float MaxX = 10.0f;
		float MinX = -10.0f;

		float ranx = ((float(rand()) / float(RAND_MAX)) * (MaxX - MinX)) + MinX;
		float ranz = ((float(rand()) / float(RAND_MAX)) * (Max - Min)) + Min;
		float rany = 5.0f;

		int maxCol = 8;
		int minCol = 1;
		int ranCol = (rand() % (maxCol - minCol)) + minCol;

		spheres[i].setPosition(glm::vec4(ranx, rany, ranz, 1.0f));
		spheres[i].setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		spheres[i].setRadius(3.0f);
		spheres[i].setReflective(0);
	}
	spheres[0].setPosition(glm::vec4(-6.0f, 5.0f, -5.0f, 1.0f));
	spheres[1].setPosition(glm::vec4(6.0f, 5.0f, -5.0f, 1.0f));
	spheres[2].setPosition(glm::vec4(-6.0f, 5.0f, 5.0f, 1.0f));
	spheres[3].setPosition(glm::vec4(6.0f, 5.0f, 5.0f, 1.0f));

	spheres[0].setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	spheres[1].setColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	spheres[2].setColor(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
	spheres[3].setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

	spheres[3].setReflective(1);
	spheres[1].setReflective(0);
	spheres[2].setReflective(2);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	bool test = glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}
void initializeTriangles()
{
	glGetError();
	glGenBuffers(1, &mTriangleBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mTriangleBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_NUM_OF_TRIANGLES * sizeof(struct Triangle), NULL, GL_STATIC_DRAW);
	struct Triangle* triangles = (struct Triangle*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, MAX_NUM_OF_TRIANGLES*sizeof(struct Triangle), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (unsigned int i = 0; i < MAX_NUM_OF_TRIANGLES /*- 1 <-- wtf?*/; ++i)
	{
		float Max = 50.0f;
		float Min = -50.0f;

		float MaxY = 7.0f;
		float MinY = 3.0f;

		float MaxX = 10.0f;
		float MinX = -10.0f;

		float ranx = ((float(rand()) / float(RAND_MAX)) * (MaxX - MinX)) + MinX;
		float ranz = ((float(rand()) / float(RAND_MAX)) * (Max - Min)) + Min;
		float rany = 5.0f;

		int maxCol = 8;
		int minCol = 1;
		int ranCol = (rand() % (maxCol - minCol)) + minCol;

		triangles[i].setReflective(0);

	}

	triangles[0].setPositions(glm::vec4(-10.0f, 0.0f, -10.0f, 1.0f), glm::vec4(-10.0f, 20.0f, 10.0f, 1.0f), glm::vec4(-10.0f, 0.0f, 10.0f, 1.0f));
	triangles[1].setPositions(glm::vec4(-10.0f, 20.0f, -10.0f, 1.0f), glm::vec4(-10.0f, 20.0f, 10.0f, 1.0f), glm::vec4(-10.0f, 0.0f, -10.0f, 1.0f));

	triangles[2].setPositions(glm::vec4(10.0f, 0.0f, 10.0f, 1.0f), glm::vec4(10.0f, 20.0f, 10.0f, 1.0f), glm::vec4(10.0f, 0.0f, -10.0f, 1.0f));
	triangles[3].setPositions(glm::vec4(10.0f, 0.0f, -10.0f, 1.0f), glm::vec4(10.0f, 20.0f, 10.0f, 1.0f), glm::vec4(10.0f, 20.0f, -10.0f, 1.0f));

	triangles[4].setPositions(glm::vec4(10.0f, 0.0f, -10.0f, 1.0f), glm::vec4(-10.0f, 20.0f, -10.0f, 1.0f), glm::vec4(-10.0f, 0.0f, -10.0f, 1.0f));
	triangles[5].setPositions(glm::vec4(10.0f, 0.0f, -10.0f, 1.0f), glm::vec4(10.0f, 20.0f, -10.0f, 1.0f), glm::vec4(-10.0f, 20.0f, -10.0f, 1.0f));

	triangles[6].setPositions(glm::vec4(-10.0f, 0.0f, 10.0f, 1.0f), glm::vec4(10.0f, 0.0f, 10.0f, 1.0f), glm::vec4(-10.0f, 0.0f, -10.0f, 1.0f));
	triangles[7].setPositions(glm::vec4(-10.0f, 0.0f, -10.0f, 1.0f), glm::vec4(10.0f, 0.0f, 10.0f, 1.0f), glm::vec4(10.0f, 0.0f, -10.0f, 1.0f));

	triangles[0].setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	triangles[1].setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

	triangles[2].setColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	triangles[3].setColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

	triangles[4].setColor(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
	triangles[5].setColor(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));

	triangles[6].setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	triangles[7].setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	bool test = glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}
void initialize()
{
	mShaders[COMPUTE_SHADER].initComputeShader("Shaders/tiledShading.glsl");
	mShaders[COMPUTE_SHADER].initUniforms();
	mShaders[QUAD_SHADER].initShader("Shaders/quadShader.vs", "Shaders/quadShader.fs");
	mShaders[QUAD_SHADER].initUniforms();

	mScreenQuad = new FullScreenQuad();
	mTexture = new Texture2D(mWidth, mHeight);

	initializePointLights();
	initializeSpheres();
	initializeTriangles();
}
int main(void)
{


	//****************************************************//
	//                                                    //
	//               GLFW INITIALIZATION                  //
	//                                                    //
	//****************************************************//
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	/*	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); //Request a specific OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //Request a specific OpenGL version

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);*/

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);


	int major;
	int minor;
	glfwGetVersion(&major, &minor, NULL);

	GLFWwindow* window;
	window = glfwCreateWindow(1280, 720, "Ray Tracer", NULL, NULL);

	glfwSetErrorCallback(error_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", glfwGetPrimaryMonitor(), NULL);
	if (!window)
	{
		fprintf(stderr, "Failed to open GLFW window.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	//This function makes the context of the specified window current on the calling thread.   
	glfwMakeContextCurrent(window);

	//Sets the relevant callbacks 
	//Initialize GLEW  
	glewExperimental = true;
	GLenum err = glewInit();

	//If GLEW hasn't initialized  
	if (err != GLEW_OK)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return -1;
	}
	//****************************************************//
	//                                                    //
	//             //GLFW INITIALIZATION                  //
	//                                                    //
	//****************************************************//
	initialize();

	glViewport(0, 0, 1280, 720);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);

	do
	{
		// Update the input
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		updateInput(deltaTime);



		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.7f, 0.7f, 0.7f, 1.0f);


		//The geometry buffer is now filled, this is where the compute shader runs
		glUseProgram(mShaders[COMPUTE_SHADER].getProgram());
		mTexture->bind();
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, mLightBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, mSphereBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, mTriangleBuffer);
		glUniform3f(mShaders[COMPUTE_SHADER].getUniformLocation("cameraPos"), 0.0f, 0.0f, 0.0f);
		//	glUniform3f(mShaders[COMPUTE_SHADER].getUniformLocation("cameraPos"), mCamera.mPos.x, mCamera.mPos.y, mCamera.mPos.z);
		//	glUniform3f(mShaders[COMPUTE_SHADER].getUniformLocation("cameraDirection"), mCamera.mFront.x, mCamera.mFront.y, mCamera.mFront.z);
		glUniform3f(mShaders[COMPUTE_SHADER].getUniformLocation("up"), mCamera.mUp.x, mCamera.mUp.y, mCamera.mUp.z);
		glUniformMatrix4fv(mShaders[COMPUTE_SHADER].getUniformLocation("inverseViewMatrix"), 1, GL_FALSE, &mCamera.getInverseViewMatrix()[0][0]);
		glUniformMatrix4fv(mShaders[COMPUTE_SHADER].getUniformLocation("viewMatrix"), 1, GL_FALSE, &mCamera.getViewMatrix()[0][0]);

		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		glDispatchCompute((1280 / 16), (720 / 16), 1);

		glFinish();

		renderFullScreenQuad(mShaders[QUAD_SHADER], false);

		glfwSwapBuffers(window);
		//Get and organize events, like keyboard and mouse input, window resizing, etc...  
		glfwPollEvents();

	} while (!glfwWindowShouldClose(window));

	delete mScreenQuad;
	delete mTexture;

	//Close OpenGL window and terminate GLFW  
	glfwDestroyWindow(window);
	//Finalize and clean up GLFW  
	glfwTerminate();

	exit(EXIT_SUCCESS);
};

