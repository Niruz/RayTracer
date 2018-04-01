#ifndef CAMERA_H
#define CAMERA_H
// Std. Includes
#include <vector>
// GL Includes
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

enum camMovement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVTY = 0.25f;
const float ZOOM = 45.0f;

class Camera
{
public:
	Camera();
	Camera(const glm::vec3& position, const glm::vec3& up, const GLfloat yaw, const GLfloat pitch);
	~Camera();

	void processKeyBoard(camMovement direction, GLfloat deltaTime);
	void processMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true);
	void processMouseWheelMovement(GLfloat zoom);

	void updateCamera();
	void updateProjectionMatrix();

	glm::mat4 getViewMatrix();
	glm::mat4 getInverseViewMatrix();
	// Camera Attributes
	glm::vec3 mPos;
	glm::vec3 mFront;
	glm::vec3 mUp;
	glm::vec3 mRight;
	glm::vec3 mWorldUp;
	// Eular Angles
	float mYaw;
	float mPitch;
	// Camera options
	float mMovementSpeed;
	float mMouseSensitivity;
	float mZoom;
	//window values
	float mWindowWidth;
	float mWindowHeight;

private:


};

#endif