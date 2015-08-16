#include "Camera.h"

void Camera::UpdateCamera(float msec)
{
	msec = msec / 2;
	pitch -= (Window::GetMouse()->GetRelativePosition().y);
	yaw -= (Window::GetMouse()->GetRelativePosition().x);

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_Z))
		roll += msec;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_X))
		roll -= msec;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_C))
		roll = 0;

	pitch = min(pitch, 90.0f);
	pitch = max(pitch, -90.0f);

	if (yaw < 0)
		yaw += 360.0f;

	if (yaw > 360.0f)
		yaw -= 360.0f;

	if (roll < 0)
		roll += 360.0f;

	if (roll > 360.0f)
		roll -= 360.0f;
	
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_W))
		position += Matrix4::Rotation(yaw, Vector3(0.0f, 1.0f, 0.0f)) * Vector3(0.0f, 0.0f, -1.0f) *msec * speed.y;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_S))
		position -= Matrix4::Rotation(yaw, Vector3(0.0f, 1.0f, 0.0f)) * Vector3(0.0f, 0.0f, -1.0f) *msec * speed.y;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_A))
		position += Matrix4::Rotation(yaw, Vector3(0.0f, 1.0f, 0.0f)) * Vector3(-1.0f, 0.0f, 0.0f) *msec * speed.x;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_D))
		position -= Matrix4::Rotation(yaw, Vector3(0.0f, 1.0f, 0.0f)) * Vector3(-1.0f, 0.0f, 0.0f) *msec * speed.x;
/*
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_Q))
		position += Matrix4::Rotation(roll, Vector3(0.0f, 1.0f, 0.0f)) * Vector3(0.0f, -1.0f, 0.0f) *msec * speed.z;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_E))
		position -= Matrix4::Rotation(roll, Vector3(0.0f, 1.0f, 0.0f)) * Vector3(0.0f, -1.0f, 0.0f) *msec * speed.z;*/

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SHIFT))
		position.y -= msec * speed.z;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SPACE))
		position.y += msec * speed.z;

	/*cout << pitch << endl;
	cout << yaw << endl;
	cout << position << endl;*/
}

Matrix4 Camera::BuildViewMatrix()
{
	return Matrix4::Rotation(-pitch, Vector3(1, 0, 0)) * Matrix4::Rotation(-yaw, Vector3(0, 1, 0)) * Matrix4::Rotation(-roll, Vector3(0, 0, 1)) * Matrix4::Translation(-position);
}
