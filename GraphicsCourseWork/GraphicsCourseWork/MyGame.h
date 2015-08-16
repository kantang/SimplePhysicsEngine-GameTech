/******************************************************************************
Class:MyGame
Implements:GameEntity
Author:Rich Davison	<richard.davison4@newcastle.ac.uk> and YOU!
Description: This is an example of a derived game class - all of your game 
specific things will go here. Such things would include setting up the game 
environment (level, enemies, pickups, whatever), and keeping track of what the
gameplay state is (Is the player dead? Has he/she won the game? And so on).

To get you started, there's a few static functions included that will create 
create some simple game objects - ideally your game object loading would be
handled by config files or somesuch, but for a simple demo game, these 
functions are a good start. Don't be afraid to modify/delete/add more of these!

Finally, there's some calls to the OGLRenderer debug draw functions that have
been added to the course this year. These are really useful for debugging the
state of your game in a graphical way - i.e drawing a line from a character
to the item it is aiming at, or pointing in the direction it is moving, or
for drawing debug bounding volumes to see when objects collide. Feel free
to add more of these debug functions - how they are implemented is covered
in the code definitions for them, and also in the extra introduction 
document on the research website.

Have fun!


-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*//////////////////////////////////////////////////////////////////////////////


#pragma once
#include "GameEntity.h"
#include "GameClass.h"
#include "Camera.h"
#include "CubeRobot.h"
#include "PhysicsShapes.h"
#include <mutex>

#define SPHERE_INITIAL_SIZE 50.0f
#define SPHERE_INITIAL_SPEED 2.0f
#define SPHERE_INITIAL_INVMASS 3.0f
#define SPHEPE_PER_AXIS 3
#define HEIGHTMAP_SIZE 4.0f
#define RESPAWN_TIMER 5000.0f
#define REMAINANT_SPEED 5.0f

class MyGame : public GameClass	{
public:
	MyGame();
	~MyGame(void);

	virtual void UpdateGame(float msec);

	int GetSpeedFactor() const { return speedFactor; };
	int GetSizeFactor() const { return sizeFactor; };

	void DestroySphereNode(SpherePhysicsNode* node);

protected:

	GameEntity* BuildRobotEntity();

	GameEntity* BuildCubeEntity(float size, const Vector3& position);

	GameEntity* BuildSphereEntity(float radius, const Vector3& position);

	GameEntity* BuildQuadEntity(float size, const Quaternion& direction, const Vector3& position);

	GameEntity* BuildHeightmapEntity(float size, Vector3& position);

	void BuildDodgingEntities(float size, Vector3& initialPosition);

	void CreateSphereToCameraPosition();

	void InitalizeTreesEntities();

	void CreateRestSpheres(Vector3 position, float size);

	int speedFactor;
	int sizeFactor;

	vector<GameEntity*> destroyedEntities;
	mutex destroyedEntitiesMutex;

	GLuint sphereTexture;

	Mesh* cube;
	Mesh* quad;
	Mesh* sphere;
	Mesh* heightmap;
};

