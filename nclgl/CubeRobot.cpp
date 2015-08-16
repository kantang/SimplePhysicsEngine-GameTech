#include "CubeRobot.h"

//cube is a static class variable, and so must be initialised outside of the 
//class...here's as good a place as any!
Mesh* CubeRobot::cube = NULL;

CubeRobot::CubeRobot(void)	{
	if (!cube) {
		CreateCube();
	}
	SetMesh(cube);

	//Make the body
	SceneNode*body = new SceneNode(cube, Vector4(1, 0, 0, 1));
	body->SetModelScale(Vector3(10, 15, 5));
	body->SetTranslation(Vector3(0, 35, 0));
	AddChild(body);

	//Add the head
	head = new SceneNode(cube, Vector4(0, 1, 0, 1));
	head->SetModelScale(Vector3(5, 5, 5));
	head->SetTranslation(Vector3(0, 30, 0));
	body->AddChild(head);

	//Add the left arm
	leftArm = new SceneNode(cube, Vector4(0, 0, 1, 1));
	leftArm->SetModelScale(Vector3(3, -18, 3));
	leftArm->SetTranslation(Vector3(-12, 30, -1));
	body->AddChild(leftArm);

	//Add the right arm
	rightArm = new SceneNode(cube, Vector4(0, 0, 1, 1));
	rightArm->SetModelScale(Vector3(3, -18, 3));
	rightArm->SetTranslation(Vector3(12, 30, -1));
	body->AddChild(rightArm);

	//Add the left leg
	leftLeg = new SceneNode(cube, Vector4(0, 0, 1, 1));
	leftLeg->SetModelScale(Vector3(3, -17.5, 3));
	leftLeg->SetTranslation(Vector3(-8, 0, 0));
	body->AddChild(leftLeg);

	//Finally the right leg!
	rightLeg = new SceneNode(cube, Vector4(0, 0, 1, 1));
	rightLeg->SetModelScale(Vector3(3, -17.5, 3));
	rightLeg->SetTranslation(Vector3(8, 0, 0));
	body->AddChild(rightLeg);

	//Giant CubeRobot!
	//transform = Matrix4::Scale(Vector3(10,10,10));

	//The Scene Management Tutorial introduces these, as cheap culling tests
	body->SetBoundingRadius(15.0f);
	head->SetBoundingRadius(5.0f);

	leftArm->SetBoundingRadius(18.0f);
	rightArm->SetBoundingRadius(18.0f);

	leftLeg->SetBoundingRadius(18.0f);
	rightLeg->SetBoundingRadius(18.0f);
}

void CubeRobot::Update(float msec) {
	rotation = Matrix4::Rotation(msec / 10.0f, Vector3(0, 1, 0));

	head->SetRotation(Matrix4::Rotation(-msec / 10.0f, Vector3(0, 1, 0))* head->GetTransform());
	leftArm->SetRotation(leftArm->GetTransform() * Matrix4::Rotation(-msec / 10.0f, Vector3(1, 0, 0)));
	rightArm->SetRotation(rightArm->GetTransform() * Matrix4::Rotation(msec / 10.0f, Vector3(1, 0, 0)));

	SceneNode::Update(msec);
}