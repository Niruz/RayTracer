#include "Camera.h"
#include <glm/gtc/type_ptr.hpp>
Camera::Camera(const glm::vec3& position, const glm::vec3& up, const GLfloat yaw, const GLfloat pitch) :
mFront(glm::vec3(0.0f, 0.0f, -1.0f)),
mMovementSpeed(SPEED),
mMouseSensitivity(SENSITIVTY),
mZoom(ZOOM)
{
	mPos = position;
	mWorldUp = up;
	mYaw = yaw;
	mPitch = pitch;
	updateCamera();

}
Camera::Camera() :
mFront(glm::vec3(0.0f, 0.0f, -1.0f)),
mMovementSpeed(SPEED),
mMouseSensitivity(SENSITIVTY),
mZoom(ZOOM)
{
	mPos = glm::vec3(0.0f, 0.0f, 0.0f);
	mWorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	mYaw = YAW;
	mPitch = PITCH;
	updateCamera();
}
Camera::~Camera()
{

}
void Camera::processKeyBoard(camMovement direction, GLfloat deltaTime)
{
	GLfloat velocity = mMovementSpeed * deltaTime;

	if (direction == FORWARD)
		mPos += mFront*velocity;
	else if (direction == BACKWARD)
		mPos -= mFront*velocity;
	else if (direction == RIGHT)
		mPos += mRight*velocity;
	else if (direction == LEFT)
		mPos -= mRight*velocity;
	else if (direction == UP)
		mPos += mWorldUp*velocity;
	else if (direction == DOWN)
		mPos -= mWorldUp*velocity;
}
void Camera::processMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch)
{
	xoffset *= mMouseSensitivity;
	yoffset *= mMouseSensitivity;

	mYaw += xoffset;
	mPitch += yoffset;
	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch)
	{
		if (mPitch > 89.0f)
			mPitch = 89.0f;
		if (mPitch < -89.0f)
			mPitch = -89.0f;
	}
	// Update Front, Right and Up Vectors using the updated Eular angles
	updateCamera();
}
void Camera::processMouseWheelMovement(GLfloat zoom)
{

}
void Camera::updateCamera()
{
	glm::vec3 front;
	front.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
	front.y = sin(glm::radians(mPitch));
	front.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));

	mFront = glm::normalize(front);

	mRight = glm::normalize(glm::cross(mFront, mWorldUp));
	mUp = glm::normalize(glm::cross(mRight, mFront));
}

glm::mat4 Camera::getViewMatrix()
{
	return glm::lookAt(mPos, mPos + mFront, mUp);
}
glm::mat4 Camera::getInverseViewMatrix()
{
	return glm::inverse(getViewMatrix());
}