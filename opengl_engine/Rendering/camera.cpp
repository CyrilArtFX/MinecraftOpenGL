#include "camera.h"

Camera::Camera(Vector3 position_, Vector3 front_, float yaw_, float pitch_, float fov_) :
	position(position_), front(front_), yaw(yaw_), pitch(pitch_), fov(fov_),
	camSpeed(SPEED), sensitivity(SENSITIVITY),
	pitchMin(PITCH_MIN), pitchMax(PITCH_MAX), fovMin(FOV_MIN), fovMax(FOV_MAX)
{
	updateCameraVectors();
}


Matrix4 Camera::getViewMatrix()
{
	return Matrix4::createLookAt(position, position + front, up);
}


void Camera::processKeyboard(Camera_Movement direction, float deltaTime)
{
	float velocity = camSpeed * deltaTime;

	if (direction == Forward)
		position += front * velocity;

	if (direction == Backward)
		position -= front * velocity;

	if (direction == Left)
		position += right * velocity;

	if (direction == Right)
		position -= right * velocity;

	if (direction == Up)
		position += Vector3::unitY * velocity;

	if (direction == Down)
		position -= Vector3::unitY * velocity;
}

void Camera::processMouseMovement(float xoffset, float yoffset)
{
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch = Maths::clamp(pitch + yoffset, pitchMin, pitchMax);

	updateCameraVectors();
}

void Camera::processMouseScroll(float yoffset)
{
	fov = Maths::clamp(fov - yoffset, fovMin, fovMax);
}


void Camera::updateCameraVectors()
{
	Vector3 n_front;
	n_front.x = Maths::cos(Maths::toRadians(yaw)) * Maths::cos(Maths::toRadians(pitch));
	n_front.y = Maths::sin(Maths::toRadians(pitch));
	n_front.z = Maths::sin(Maths::toRadians(yaw)) * Maths::cos(Maths::toRadians(pitch));
	front = Vector3::normalize(n_front);

	right = Vector3::normalize(Vector3::cross(front, Vector3::unitY));
	up = Vector3::normalize(Vector3::cross(right, front));
}


void Camera::setPosition(Vector3 newPos)
{
	position = newPos;
}

Vector3 Camera::getFlatFront()
{
	Vector3 flat_front{ front.x, 0.0f, front.z };
	flat_front.normalize();
	return flat_front;
}

void Camera::setSpeed(float newSpeed)
{
	camSpeed = newSpeed;
}

void Camera::setSensitivity(float newSensitivity)
{
	sensitivity = newSensitivity;
}

void Camera::setPitchClamp(float min, float max)
{
	pitchMin = min;
	pitchMax = max;
}

void Camera::setFovClamp(float min, float max)
{
	fovMin = min;
	fovMax = max;
}
