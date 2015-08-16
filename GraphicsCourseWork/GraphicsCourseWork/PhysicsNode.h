/******************************************************************************
Class:PhysicsNode
Implements:
Author:Rich Davison	<richard.davison4@newcastle.ac.uk> and YOU!
Description: This class represents the physical properties of your game's
entities - their position, orientation, mass, collision volume, and so on.
Most of the first few tutorials will be based around adding code to this class
in order to add correct physical integration of velocity / acceleration etc to
your game objects. 


In addition to the tutorial code, this class contains a pointer to a SceneNode.
This pointer is to the 'graphical' representation of your game object, just 
like the SceneNode's used in the graphics module. However, instead of 
calculating positions etc as part of the SceneNode, it should instead be up
to your 'physics' representation to determine - so we calculate a transform
matrix for your SceneNode here, and apply it to the pointer. 

Your SceneNode should still have an Update function, though, in order to
update any graphical effects applied to your object - anything that will end
up modifying a uniform in a shader should still be the job of the SceneNode. 

Note that since the SceneNode can still have children, we can represent our
entire CubeRobot with a single PhysicsNode, and a single SceneNode 'root'.

-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*//////////////////////////////////////////////////////////////////////////////


#pragma once

#include "Quaternion.h"
#include "Vector3.h"
#include "Matrix4.h"
#include "SceneNode.h"
#include "Matrix3.h"
#include "CollisionPrimitive.h"
#include <algorithm>
#include <map>


#define DUMPING_FACTOR 0.995f
#define MINIMUM_VELOCITY_SQUARE 0.0001f
#define EARTH_GRAVITY_FACTOR 9.8f
#define NON_GRAVITY 0.0f

class PhysicsNode	{
public:
	PhysicsNode(void);
	PhysicsNode(Quaternion orientation, Vector3 position);
	~PhysicsNode(void);

	Vector3		GetPosition()	   const{ return m_position;}
	Vector3		GetLinearVelocity()const{ return m_linearVelocity;}
	
	float		GetInverseMass()   const{ return m_invMass; }
	float		GetGravityFactor() const{ return m_gravity_factor; };

	Quaternion	GetOrientation()	const{ return m_orientation;}
	Vector3		GetAngularVelocity()const{ return m_angularVelocity;}

	Matrix3		GetInvInertia()		const{ return m_invInertia; };

	Matrix4		BuildTransform();

	virtual void		Update(float msec);

	void	SetLinearVelocity(Vector3 linearVelocity){ m_linearVelocity = linearVelocity; };
	void	SetAngularVelocity(Vector3 angularV){ m_angularVelocity = angularV; };
	void	SetPosition(Vector3 position){ m_position = position; };
	virtual void SetTarget(SceneNode *s) { 
		target = s;
		if (target->GetColour() == Vector4())
		{
			target->SetColour(Vector4((((int)rand()) % 100) / 100.0f, (((int)rand()) % 100) / 100.0f, (((int)rand()) % 100) / 100.0f, 1.0f));
		}
		initialColour = target->GetColour();
	}
	void	SetInvMass(float invMass){ m_invMass = invMass; m_gravity = m_gravity_factor / invMass; };
	void	SetForce(Vector3 force){ m_force = force; };
	void	SetForcePosition(Vector3 forcePos){ m_force_position = forcePos; };
	void	SetCentroidPosition(Vector3 centroidPos){ m_centroid_position = centroidPos; };
	void	SetGravityFactor(float gravityFac){ 
		m_gravity_factor = gravityFac; 
		if (m_invMass != 0.0f)
			m_gravity = m_gravity_factor / m_invMass;
		else
			m_gravity = FLT_MAX;
	};
	void	SetOrientation(const Matrix4& orientation){ m_orientation = Quaternion::FromMatrix(orientation); };

	void	SetCollisionPrimitive(CollisionPrimitive *cP){ collisionPrimitive = cP; };
	CollisionPrimitive* GetCollisionPrimitive(){ return collisionPrimitive; };

	Vector3	GetForce()	{ return m_force;}
	Vector3	GetTorque() { return m_torque;}

	void CollisionCounterIncrement(){ ++collisionCounter; };
	void CollisionCounterToZero(){ collisionCounter = 0; };

	int GetCollisionCounter() const { return collisionCounter; };

	bool isNodeActive() const{ return isActive; };
	void ActivateNode();
	void DeactivateNode();

	//method to update the inertia matrix for the object
	//must be implemented for each of the subclass.
	virtual void BuildInertiaMatrix(){};

	//build the torque vector after the inverse inertia actually has it's valid value
	void BuildTorque();

	SceneNode* GetTarget() const{ return target; };
	float xBegin;
	float xEnd;

	float dumpingFactor;

protected:
	//<---------LINEAR-------------->
	//position of the physics node.
	Vector3		m_position;
	//the linear velocity of the node.
	Vector3		m_linearVelocity;
	//the force on the object. Local axis
	Vector3		m_force;
	//the position of the force on the object.
	//the coordinate is in the dimention with the object
	//hence no matter how much the object spins, the vector
	//will remain the same
	Vector3		m_force_position;
	//the position of the centroid of the object.
	//the coordinate is in the dimention with the object
	//hence no matter how much the object spins, the vector
	//will remain the same
	Vector3		m_centroid_position;
	//the inverse mass of the object.
	float		m_invMass;

	//<----------ANGULAR--------------->
	//the orientation of the object.
	Quaternion  m_orientation;
	//the angular velocity of the object.
	Vector3		m_angularVelocity;
	//the torque of the object, use build torque to update.
	Vector3		m_torque;
	//the inverse inertia matrix of the object, use build inertia
	//to update or initialise.
	Matrix3     m_invInertia;

	//<----------GAVITY--------------->
	float		m_gravity_factor;
	float		m_gravity;

	bool		isActive;
	int			collisionCounter;

	virtual void InverseInertia(){};

	Vector3 ForceOnCentroid();

	CollisionPrimitive* collisionPrimitive;

	Vector4 initialColour;
	
	//The sceneNode that holds the object
	SceneNode*	target;  

};

