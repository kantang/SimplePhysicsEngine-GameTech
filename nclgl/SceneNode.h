#pragma once
#include "../nclgl/Matrix4.h"
#include "../nclgl/Vector3.h"
#include "../nclgl/Vector4.h"
#include "../nclgl/Mesh.h"

using namespace std;

class SceneNode
{
public:
	SceneNode(Mesh* m = NULL, Vector4 colour = Vector4(1, 1, 1, 1));
	~SceneNode(void);

	void SetTranslation(Vector3 translation){ this->translation = translation; };
	Vector3 getTranslation(){ return translation; };

	Matrix4 GetTransform(){ return Matrix4::Translation(Vector3(modelScale.x * translation.x, modelScale.y * translation.y, modelScale.z * translation.z)) * rotation; };

	void SetRotation(Matrix4 rotation){ this->rotation = rotation; };
	Matrix4 getRotation(){ return rotation; };

	Matrix4 GetWorldTransform() const{ return worldTransform; };

	Vector4 GetColour() const{ return colour; };
	void SetColour(Vector4 c) { colour = c; };

	Vector3 GetModelScale() const { return modelScale; };
	void SetModelScale(Vector3 s) { modelScale = s; };

	Mesh* GetMesh() const{ return mesh; }
	void SetMesh(Mesh* m){ mesh = m; };

	void AddChild(SceneNode* s);
	bool RemoveChild(SceneNode* s, bool recursive = true);

	virtual void Update(float msec);
	virtual void Draw();

	float GetBoundingRadius() const{ return boundingRadius; };
	void SetBoundingRadius(float f){ boundingRadius = f; };

	float GetCameraDistance()const { return distanceFromCamera; };
	void SetCameraDistance(float f){ distanceFromCamera = f; };

	static bool CompareByCameraDistance(SceneNode *a, SceneNode *b){ return (a->distanceFromCamera < b->distanceFromCamera) ? true : false; };

	vector<SceneNode*>::const_iterator GetChildIteratorStart() { return children.begin(); };
	vector<SceneNode*>::const_iterator GetChildIteratorEnd(){ return children.end(); };

	SceneNode* getParent(){ return parent; };
	vector<SceneNode*> getChildren()const{ return children; };

protected:
	SceneNode* parent;
	Mesh* mesh;
	Matrix4 worldTransform;
	Vector3 translation;
	Matrix4 rotation;
	Vector3 modelScale;
	Vector4 colour;
	vector<SceneNode*> children;

	float distanceFromCamera;
	float boundingRadius;
};

