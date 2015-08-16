#include "SceneNode.h"


SceneNode::SceneNode(Mesh* mesh, Vector4 colour)
{
	this->mesh = mesh;
	this->colour = colour;
	parent = NULL;
	modelScale = Vector3(1, 1, 1);
	children = vector<SceneNode*>(0);
	boundingRadius = 1.0f;
	distanceFromCamera = 0.0f;
	rotation = Matrix4::Rotation(0, Vector3(0, 0, 0));
	translation = Vector3(0, 0, 0);
}


SceneNode::~SceneNode(void)
{
	for (unsigned int i = 0; i < children.size(); ++i)
		delete children[i];
}

void SceneNode::AddChild(SceneNode* s)
{
	children.push_back(s);
	s->parent = this;
}

void SceneNode::Draw()
{
	if (mesh)
		mesh->Draw();
}

void SceneNode::Update(float msec)
{
	if (parent)
	{
		Matrix4 transform = Matrix4::Translation(Vector3(modelScale.x * translation.x, modelScale.y * translation.y, modelScale.z * translation.z)) * rotation;
		worldTransform = parent->worldTransform * transform;
	}
	else
	{
		Matrix4 transform = Matrix4::Translation(Vector3(modelScale.x * translation.x, modelScale.y * translation.y, modelScale.z * translation.z)) * rotation;
		worldTransform = transform;

	}

	for (vector<SceneNode*>::iterator i = children.begin(); i != children.end(); i++)
		(*i)->Update(msec);
}

bool SceneNode::RemoveChild(SceneNode* s, bool recursive) {

	for (auto i = children.begin(); i != children.end(); ++i) {
		if ((*i) == s) {
			i = children.erase(i);
			return true;
		}
	}

	if (recursive) {
		for (auto i = children.begin(); i != children.end(); ++i) {
			if ((*i)->RemoveChild(s, recursive)) {
				return true;
			}
		}
	}
	return false;
}
