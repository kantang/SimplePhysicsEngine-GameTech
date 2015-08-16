#pragma once

#include "Window.h"
#include "Matrix4.h"
#include "Vector3.h"

class Camera
{
public:
	Camera(void)
	{
		yaw = 0.0f;
		pitch = 0.0f;
		roll = 0.0f;
		speed = { 1, 1, 1 };
	};

	Camera(float pitch, float yaw, float roll, Vector3 position,Vector3 speed)
	{
		this->pitch = pitch;
		this->yaw = yaw;
		this->roll = roll;
		this->position = position;
		this->speed = speed;
	};

	~Camera(){};

	void UpdateCamera(float msec = 10.0f);

	Matrix4 BuildViewMatrix();

	Vector3 GetPosition() const{ return position; };
	void SetPostion(Vector3 val){ position = val; };

	Vector3 GetSpeed() const{ return speed; };
	void SetSpeed(Vector3 val){ speed = val; };

	float GetYaw() const { return yaw; };
	void SetYaw(float y){ yaw = y; };

	float GetPitch()const { return pitch; };
	void SetPitch(float p) { pitch = p; };

	float GetRoll()const { return roll; };
	void SetRoll(float r){ roll = r; };


	Vector3 GetViewVector()
	{
		Matrix4 tempMatirx = BuildViewMatrix();
		return Vector3(-tempMatirx(0, 2), -tempMatirx(1, 2), -tempMatirx(2, 2));
	};

protected:
	float yaw;
	float pitch;
	float roll;
	Vector3 speed;
	Vector3 position;
};

