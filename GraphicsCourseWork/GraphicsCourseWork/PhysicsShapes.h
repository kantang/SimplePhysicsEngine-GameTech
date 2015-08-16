#pragma once
#include "PhysicsNode.h"

#define ONE_TWELVE 0.08333f
#define TWO_FIFTHS 0.4f

class PhysicsShapes :
	public PhysicsNode
{
};

class CuboidPhysicsNode : public PhysicsNode
{
public:
	CuboidPhysicsNode(void) : PhysicsNode()
	{
		BuildCubeCollisionPrimitive();
	};
	CuboidPhysicsNode(Quaternion orientation, Vector3 position) : PhysicsNode(orientation, position)
	{
		BuildCubeCollisionPrimitive();
	};

	virtual void BuildInertiaMatrix();

	Vector3 GetSize() const { return size; };
	void SetSize(const Vector3& s){ size = s; ((CollisionCube*)collisionPrimitive)->halfSize = s * 0.5f; };

protected:

	void BuildCubeCollisionPrimitive(){ collisionPrimitive = new CollisionCube(&m_position, Vector3(0.0f, 0.0f, 0.0f)); };

	Vector3 size;

	void InverseInertia();
};

class SpherePhysicsNode : public PhysicsNode
{
public:
	SpherePhysicsNode(void) : PhysicsNode()
	{
		BuildSphereCollisionPrimitive();
	};
	SpherePhysicsNode(Quaternion orientation, Vector3 position) : PhysicsNode(orientation, position)
	{
		BuildSphereCollisionPrimitive();
	};

	virtual void BuildInertiaMatrix();

	float GetRadius() const { return radius; };
	void SetRadius(float rad){ radius = rad; ((CollisionSphere*)collisionPrimitive)->m_radius = rad; };

protected:

	void BuildSphereCollisionPrimitive(){ collisionPrimitive = new CollisionSphere(&m_position, 0.0f); };

	float radius;

	void InverseInertia();
};

class SquarePhysicsNode : public PhysicsNode
{
public:
	SquarePhysicsNode(void) : PhysicsNode()
	{
		BuildSquareCollisionPrimitive();
	};
	SquarePhysicsNode(Quaternion orientation, Vector3 position) : PhysicsNode(orientation, position)
	{
		normal = m_orientation.GetNormal();
		distance = Vector3::Dot(m_position, normal);
		BuildSquareCollisionPrimitive();
	};

	virtual void BuildInertiaMatrix();

	Vector3 GetNormal() const { return normal; };
	void SetNormal(const Vector3& n){ normal = n; };

	float GetDistance() const { return distance; };
	void SetDistance(float dis){ distance = dis; };

protected:

	void BuildSquareCollisionPrimitive()
	{ 
		m_invMass = 0.0f;
		m_gravity_factor = NON_GRAVITY;
		collisionPrimitive = new CollisionPlane(&m_position, normal , distance);
	};

	Vector3 normal;
	float distance;
};

class HeightmapPhysicsNode : public PhysicsNode
{
public:
	HeightmapPhysicsNode(void) : PhysicsNode()
	{
		BuildHeightmapCollisionPrimitive();
		InitializeHeightmapCoord();
	};
	HeightmapPhysicsNode(Vector3 position) : PhysicsNode(Quaternion(), position)
	{
		normal = Vector3(0,1.0f,0);
		distance = m_position.y;
		BuildHeightmapCollisionPrimitive();
		InitializeHeightmapCoord();
	};

	~HeightmapPhysicsNode(void){};

	virtual void BuildInertiaMatrix();

	Vector3 GetNormal() const { return normal; };
	void SetNormal(const Vector3& n){ normal = n; };

	float GetDistance() const { return distance; };
	void SetDistance(float dis){ distance = dis; };

	float GetMaxHeight() const { return maxHeight; };
	float GetMinHeight() const { return minHeight; }; 

	void InitializeHeightmapCoord();

	virtual void SetTarget(SceneNode* t){
		target = t;
		InitializeHeightmapCoord();
	};

	//use map here to store the height maps coordinate to reduce the
	//complexcity when searching the height for given x and y

protected:

	void BuildHeightmapCollisionPrimitive()
	{
		m_invMass = 0.0f;
		m_gravity_factor = NON_GRAVITY;
		collisionPrimitive = new CollisionHeightmap(&m_position, normal, distance, &heightMapCoordinatesMap);
	};

	map<float, map<float, float>> heightMapCoordinatesMap;
	Vector3 normal;
	float distance;
	float maxHeight;
	float minHeight;
};

class ConePhysicsNode : public PhysicsNode
{
public:
	ConePhysicsNode(void) : PhysicsNode()
	{
		BuildConeCollisionPrimitive();
	};
	ConePhysicsNode(Vector3 position, Vector3 nm, float len, float rad) : PhysicsNode(Quaternion(), position)
	{
		normal = nm;
		length = len;
		radius = rad;
		BuildConeCollisionPrimitive();
	};

	virtual void BuildInertiaMatrix();

	float   GetLength() const { return length; };
	Vector3 GetNormal() const { return normal; };
	float   GetRadius() const { return radius; };

	virtual void Update(float msec){
		m_torque.ToZero();
		if (target) {
			target->SetRotation(BuildTransform());
		}

		xBegin = m_position.x - target->GetBoundingRadius();
		xEnd = m_position.x + target->GetBoundingRadius();

		Tree* tree = (Tree*)target;
		((CollisionCone*)collisionPrimitive)->length = length * tree->GetMainTrunkScale().y;
		((CollisionCone*)collisionPrimitive)->radius = radius * tree->GetMainTrunkScale().x;

	};
	

protected:

	void BuildConeCollisionPrimitive()
	{
		m_invMass = 0.0f;
		m_gravity_factor = NON_GRAVITY;
		collisionPrimitive = new CollisionCone(&m_position, normal, length, radius);
	};

	Vector3 normal;
	float length;
	float radius;

};