/******************************************************************************
Class:PhysicsSystem
Implements:
Author:Rich Davison	<richard.davison4@newcastle.ac.uk> and YOU!
Description: A very simple physics engine class, within which to implement the
material introduced in the Game Technologies module. This is just a rough 
skeleton of how the material could be integrated into the existing codebase -
it is still entirely up to you how the specifics should work. Now C++ and
graphics are out of the way, you should be starting to get a feel for class
structures, and how to communicate data between systems.

It is worth poinitng out that the PhysicsSystem is constructed and destructed
manually using static functions. Why? Well, we probably only want a single
physics system to control the entire state of our game world, so why allow 
multiple systems to be made? So instead, the constructor / destructor are 
hidden, and we 'get' a single instance of a physics system with a getter.
This is known as a 'singleton' design pattern, and some developers don't like 
it - but for systems that it doesn't really make sense to have multiples of, 
it is fine!

-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*//////////////////////////////////////////////////////////////////////////////


#pragma once

#include "PhysicsShapes.h"
#include "CollisionPrimitive.h"
//#include "MyGame.h"
#include <vector>

using std::vector;

#define MAXIMUM_UPDATE_RANGE 10000.0f
#define ELASTICITY 0.0f
#define ACCEL_REST_STATE_SQUARE 0.0001f
#define SPEED_REST_STATE_SQUARE 0.01f
#define REST_STATE 0.01f
#define COLLISIONS_BEFORE_EXPLODE 5

class CollisionData {
public:
	Vector3 m_point;
	Vector3 m_normal;
	float m_penetration;
};
//
//class CollisionHelper
//{
//public:
//	static bool SphereSphereCollision(PhysicsNode& p0, PhysicsNode& p1, CollisionData* data = NULL);
//	static bool SpherePlaneCollision(PhysicsNode& p0, PhysicsNode& p1, CollisionData* data = NULL);
//	static bool AddCollisionImpulse(PhysicsNode& p0, PhysicsNode& p1, CollisionData* data = NULL);
//
//};

class PhysicsSystem	{
public:
	friend class GameClass;

	void		Update(float msec);

	void		BroadPhaseCollisions();
	void		NarrowPhaseCollisions();
	void		SortAndSweepCollisions();

	//Statics
	static void Initialise() {
		instance = new PhysicsSystem();
	}

	static void Destroy() {
		delete instance;
	}

	static PhysicsSystem& GetPhysicsSystem() {
		return *instance;
	}

	void	AddNode(PhysicsNode* n);

	void	RemoveNode(PhysicsNode* n);

	void FPSCounterToZero(){ fpsCounter = 0; };
	int  GetFPSCounter() const { return fpsCounter; };

	int GetCollisionCounter() const { return collisionCounter; };
	int GetCollisionDetectionCounter() const{ return collisionDetectionCounter; };

	void ToggleBroadPhaseCulling(){ useBroadPhaseCulling = !useBroadPhaseCulling; };

	bool GetUseBroadPhaseCulling(){ return useBroadPhaseCulling; };

	double GetScore()const{ return score; };

	void SetDodgingEntity(SpherePhysicsNode* dodging){ dodgingEntity = dodging; };

protected:

	SpherePhysicsNode* dodgingEntity;

	bool useBroadPhaseCulling;

	int fpsCounter;
	int collisionCounter;
	int collisionDetectionCounter;

	double score;

	PhysicsSystem(void);
	~PhysicsSystem(void);

	bool SphereSphereCollision(const CollisionSphere &s0, const CollisionSphere &s1, CollisionData *collisionData = NULL) const;
	bool AABBCollision(const CollisionCube &cube0, const CollisionCube &cube1) const;
	bool SpherePlaneCollision(const CollisionSphere &s, const CollisionPlane &p, CollisionData *collisionData = NULL) const;
	bool SphereHeightmapCollision(const CollisionSphere &s, const CollisionHeightmap &p, CollisionData *collisionData = NULL) const;
	bool SphereConeCollision(const CollisionSphere &s, const CollisionCone &c, CollisionData * collisionData = NULL) const;

	void HandleCollision(PhysicsNode* currentNode0, PhysicsNode* currentNode1);

	//bool SphereAABBCollision(const CollisionSphere &sphere, const CollisionAABB &cube, CollisionData *collisionData = NULL) const; //Research!!!! :-)

	bool PointInSquare(const PhysicsNode& square, const Vector3& pointCoordinate);

	//Sphere plane collisions we started in the previous module, and expand upon via the Plane class..

	bool PointInConvexPolygon(const Vector3 testPosition, Vector3 * convexShapePoints, int numPoints, const Vector3& normal) const;

	void AddCollisionImpulse(PhysicsNode& p0, PhysicsNode& p1, CollisionData& data);

	static float LengthSquare(Vector3 vec);

	float currentMsec;

//Statics
	static PhysicsSystem* instance;

	vector<PhysicsNode*> allNodes;
	vector<PhysicsNode*> narrowPhaseNodes;
};

