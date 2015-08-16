#include "stdafx.h"
#include "MyGame.h"

/*
Creates a really simple scene for our game - A cube robot standing on
a floor. As the module progresses you'll see how to get the robot moving
around in a physically accurate manner, and how to stop it falling
through the floor as gravity is added to the scene. 

You can completely change all of this if you want, it's your game!

*/
MyGame::MyGame()	{

	destroyedEntities.clear();

	gameCamera = new Camera(-25, 180, 0.0f, Vector3(2000, 1350, -1100), Vector3(2, 2, 2));

	Renderer::GetRenderer().SetCamera(gameCamera);

	CubeRobot::CreateCube();

	/*
	We're going to manage the meshes we need in our game in the game class!

	You can do this with textures, too if you want - but you might want to make
	some sort of 'get / load texture' part of the Renderer or OGLRenderer, so as
	to encapsulate the API-specific parts of texture loading behind a class so
	we don't care whether the renderer is OpenGL / Direct3D / using SOIL or 
	something else...
	*/

	

	cube	= new OBJMesh(MESHDIR"cube.obj");
	quad	= Mesh::GenerateQuad();
	sphere	= new OBJMesh(MESHDIR"sphere.obj");
	quad->SetTexture(SOIL_load_OGL_texture(TEXTUREDIR"brick.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0));
	quad->SetBumpMap(SOIL_load_OGL_texture(TEXTUREDIR"brickDOT3.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0));
	heightmap = new HeightMap(TEXTUREDIR"terrain.raw");
	heightmap->SetTexture(SOIL_load_OGL_texture(TEXTUREDIR"GRASS.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));
	heightmap->SetBumpMap(SOIL_load_OGL_texture(TEXTUREDIR"GRASS_NORMAL.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));
	

	sphereTexture = (SOIL_load_OGL_texture(TEXTUREDIR"earth.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0));

	

	if (!quad->GetTexture() || !heightmap->GetTexture() || !heightmap->GetBumpMap())
	{
		return;
	}
	
	auto SetTextureRepeating = [](GLuint target, bool repeating)	{
		glBindTexture(GL_TEXTURE_2D, target);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeating ? GL_REPEAT : GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeating ? GL_REPEAT : GL_CLAMP);
		glBindTexture(GL_TEXTURE_2D, 0);
	};


	SetTextureRepeating(heightmap->GetTexture(), true);
	SetTextureRepeating(heightmap->GetBumpMap(), true);
 	SetTextureRepeating(quad->GetTexture(), true);
	SetTextureRepeating(quad->GetBumpMap(), true);
	SetTextureRepeating(sphereTexture, true);

	/*
	A more 'robust' system would check the entities vector for duplicates so as
	to not cause problems...why not give it a go?
	*/
//	allEntities.push_back(BuildRobotEntity());
	Quaternion newQuaternion = Quaternion::AxisAngleToQuaterion(Vector3(1.0f, 0, 0), 90);
	Vector3 position = Vector3(5000,-5000.0f,5000);
	allEntities.push_back(BuildQuadEntity(50000.0f,newQuaternion,position));
	//allEntities.push_back(BuildSphereEntity(200.0f));

	allEntities.push_back(BuildHeightmapEntity(HEIGHTMAP_SIZE, Vector3(0, 0, 0)));

	speedFactor = 10;
	sizeFactor = 5;

	CreateRestSpheres(Vector3(0, 2000.0f, 0), 250.0f);

	InitalizeTreesEntities();

	BuildDodgingEntities(250.0f, Vector3(0, 5000.0f, 0));
}

MyGame::~MyGame(void)	{
	/*
	We're done with our assets now, so we can delete them
	*/
	delete cube;
	delete quad;
	delete sphere;

	CubeRobot::DeleteCube();

	//GameClass destructor will destroy your entities for you...
}

void MyGame::InitalizeTreesEntities()
{
	//get the trees as scene node from the renderer
	Tree& tree0 = Renderer::GetRenderer().GetTrees()[0];
	Tree& tree1 = Renderer::GetRenderer().GetTrees()[1];
	////set each of the branches as a new cone physics node and add to game entities.
	//auto createConePhyscsNode = [&](Tree& tree)
	//{
	//	vector<SceneNode*>& treeBranches = tree.getBranches();
	//	for (unsigned int i = 0; i < treeBranches.size(); i++)
	//	{
	//		SceneNode* currentBranch = treeBranches[i];
	//		currentBranch->Update(0.0f);
	//		Branch* branch = (Branch*)(currentBranch->GetMesh());
	//		ConePhysicsNode* currentBranchNode = new ConePhysicsNode(currentBranch->GetWorldTransform().GetPositionVector(),
	//			currentBranch->getRotation() * Vector3(0, 1.0f, 0), branch->getMaxHeight(), branch->getRootDiameter());
	//	}
	//};

	auto createTreePhysics = [&](Tree& tree){
		tree.Update(0.0f);
		ConePhysicsNode* treeNode = new ConePhysicsNode(tree.GetWorldTransform().GetPositionVector(),
			Vector3(0, 1.0f, 0), tree.GetTreeAttribute().length,
			tree.GetTreeAttribute().diameter);
		treeNode->SetInvMass(0);
		treeNode->SetGravityFactor(NON_GRAVITY);
		treeNode->SetTarget(&tree);
		treeNode->ActivateNode();
		PhysicsSystem::GetPhysicsSystem().AddNode(treeNode);

	};

	createTreePhysics(tree0);
	createTreePhysics(tree1);
}

/*
Here's the base 'skeleton' of your game update loop! You will presumably
want your games to have some sort of internal logic to them, and this
logic will be added to this function.
*/
void MyGame::UpdateGame(float msec) {
	if(gameCamera) {
		gameCamera->UpdateCamera(msec);
	}

	if (Window::GetWindow().GetKeyboard()->KeyTriggered(KEYBOARD_1))
	{
		CreateSphereToCameraPosition();
	}
	if (Window::GetWindow().GetKeyboard()->KeyTriggered(KEYBOARD_UP))
	{
		if (speedFactor < 25)
			speedFactor += 5;
	}
	if (Window::GetWindow().GetKeyboard()->KeyTriggered(KEYBOARD_DOWN))
	{
		if (speedFactor > 0)
			speedFactor -= 5;
	}
	if (Window::GetWindow().GetKeyboard()->KeyTriggered(KEYBOARD_LEFT))
	{
		if (sizeFactor > 5)
			--sizeFactor;
	}
	if (Window::GetWindow().GetKeyboard()->KeyTriggered(KEYBOARD_RIGHT))
	{
		if (sizeFactor < 10)
			++sizeFactor;
	}
	if (Window::GetWindow().GetKeyboard()->KeyTriggered(KEYBOARD_B))
	{
		PhysicsSystem::GetPhysicsSystem().ToggleBroadPhaseCulling();
	}
	if (Window::GetWindow().GetKeyboard()->KeyTriggered(KEYBOARD_T))
	{
		if (!sphere->GetTexture())
			sphere->SetTexture(sphereTexture);
		else
			sphere->SetTexture(0);
	}
	/*for(vector<GameEntity*>::iterator i = allEntities.begin(); i != allEntities.end(); ++i) {
		(*i)->Update(msec);
	}*/

	destroyedEntitiesMutex.lock();
	vector<GameEntity*> copy = destroyedEntities;
	for (int i = ((int)copy.size()) - 1; i >= 0; --i)
	{
		copy[i]->Update(msec);
		if (copy[i]->destroyedTimePassed > RESPAWN_TIMER)
		{
			copy[i]->destroyedTimePassed = 0;
			Renderer::GetRenderer().rootMutex.lock();
			copy[i]->ConnectToSystems();
			Renderer::GetRenderer().rootMutex.unlock();
			allEntities.push_back(copy[i]);
			destroyedEntities.erase(destroyedEntities.begin() + i);

			cout << "a sphere is back to life" << endl;
			continue;
		}
	}
	destroyedEntitiesMutex.unlock();
	/*
	Here's how we can use OGLRenderer's inbuilt debug-drawing functions! 
	I meant to talk about these in the graphics module - Oops!

	We can draw squares, lines, crosses and circles, of varying size and
	colour - in either perspective or orthographic mode.

	Orthographic debug drawing uses a 'virtual canvas' of 720 * 480 - 
	that is 0,0 is the top left, and 720,480 is the bottom right. A function
	inside OGLRenderer is provided to convert clip space coordinates into
	this canvas space coordinates. How you determine clip space is up to you -
	maybe your renderer has getters for the view and projection matrix?

	Or maybe your Camera class could be extended to contain a projection matrix?
	Then your game would be able to determine clip space coordinates for its
	active Camera without having to involve the Renderer at all?

	Perspective debug drawing relies on the view and projection matrices inside
	the renderer being correct at the point where 'SwapBuffers' is called. As
	long as these are valid, your perspective drawing will appear in the world.

	This gets a bit more tricky with advanced rendering techniques like deferred
	rendering, as there's no guarantee of the state of the depth buffer, or that
	the perspective matrix isn't orthographic. Therefore, you might want to draw
	your debug lines before the inbuilt position before SwapBuffers - there are
	two OGLRenderer functions DrawDebugPerspective and DrawDebugOrtho that can
	be called at the appropriate place in the pipeline. Both take in a viewProj
	matrix as an optional parameter.

	Debug rendering uses its own debug shader, and so should be unaffected by
	and shader changes made 'outside' of debug drawing

	*/
	//Lets draw a box around the cube robot!
//	Renderer::GetRenderer().DrawDebugBox(DEBUGDRAW_PERSPECTIVE, Vector3(0,51,0), Vector3(100,100,100), Vector3(1,0,0));

	////We'll assume he's aiming at something...so let's draw a line from the cube robot to the target
	////The 1 on the y axis is simply to prevent z-fighting!
//	Renderer::GetRenderer().DrawDebugLine(DEBUGDRAW_PERSPECTIVE, Vector3(0,1,0),Vector3(200,1,200), Vector3(0,0,1), Vector3(1,0,0));

	////Maybe he's looking for treasure? X marks the spot!
//	Renderer::GetRenderer().DrawDebugCross(DEBUGDRAW_PERSPECTIVE, Vector3(200,1,200),Vector3(50,50,50), Vector3(0,0,0));

	////CubeRobot is looking at his treasure map upside down!, the treasure's really here...
//	Renderer::GetRenderer().DrawDebugCircle(DEBUGDRAW_PERSPECTIVE, Vector3(-200,1,-200),50.0f, Vector3(0,1,0));
}

void MyGame::CreateSphereToCameraPosition()
{
	SceneNode* s = new SceneNode(sphere);
	s->SetRotation(Matrix4::Translation(gameCamera->GetPosition()));
	//s->SetTranslation(Vector3());

	s->SetModelScale(Vector3(SPHERE_INITIAL_SIZE * sizeFactor, SPHERE_INITIAL_SIZE * sizeFactor, SPHERE_INITIAL_SIZE * sizeFactor));
	//s->SetModelScale(Vector3(1000.0f, 1000.0f , 1000.0f));

	s->SetBoundingRadius(SPHERE_INITIAL_SIZE * sizeFactor);
	s->SetColour(Vector4((((int)rand()) % 100) / 100.0f, (((int)rand()) % 100) / 100.0f, (((int)rand()) % 100) / 100.0f, 1.0f));

	GameEntity*g = new GameEntity(s, new SpherePhysicsNode());
	((SpherePhysicsNode&)(g->GetPhysicsNode())).SetRadius(SPHERE_INITIAL_SIZE*sizeFactor);

	g->GetPhysicsNode().SetPosition(gameCamera->GetPosition());

	cout << "sphere created on position " << gameCamera->GetPosition() << endl;

	g->GetPhysicsNode().SetInvMass(SPHERE_INITIAL_INVMASS / sizeFactor);
	Vector3 speedVec = gameCamera->GetViewVector();
	speedVec = speedVec * SPHERE_INITIAL_SPEED * (const float)speedFactor;
	g->GetPhysicsNode().SetLinearVelocity(speedVec);
	g->GetPhysicsNode().SetGravityFactor(EARTH_GRAVITY_FACTOR);
	g->GetPhysicsNode().ActivateNode();

	g->GetPhysicsNode().BuildInertiaMatrix();
	g->ConnectToSystems();
	allEntities.push_back(g);
}

/*
Makes an entity that looks like a CubeRobot! You'll probably want to modify
this so that you can have different sized robots, with different masses and
so on!
*/
GameEntity* MyGame::BuildRobotEntity() {
	GameEntity*g = new GameEntity(new CubeRobot(), new PhysicsNode());

	g->GetPhysicsNode().SetInvMass(0.2f);
	g->GetPhysicsNode().SetGravityFactor(NON_GRAVITY);
	g->GetPhysicsNode().SetForce(Vector3(0.1f, 0, 0));
	g->GetPhysicsNode().SetForcePosition(Vector3(0, 30, 0));
	g->GetPhysicsNode().ActivateNode();
	g->ConnectToSystems();
	return g;
}

GameEntity* MyGame::BuildCubeEntity(float size, const Vector3& position) {
	GameEntity*g = new GameEntity(new SceneNode(cube), new CuboidPhysicsNode());
	((CuboidPhysicsNode&)(g->GetPhysicsNode())).SetSize(Vector3(size,size,size));
	g->GetPhysicsNode().ActivateNode();
	g->GetPhysicsNode().BuildInertiaMatrix();
	g->ConnectToSystems();

	SceneNode &test = g->GetRenderNode();

	test.SetModelScale(Vector3(size,size,size));
	test.SetBoundingRadius(size);

	return g;
}
/*
Makes a sphere.
*/
GameEntity* MyGame::BuildSphereEntity(float radius, const Vector3& position) {
	SceneNode* s = new SceneNode(sphere);

	s->SetRotation(Matrix4::Translation(position));

	s->SetModelScale(Vector3(radius,radius,radius));
	s->SetBoundingRadius(radius);

	GameEntity*g = new GameEntity(s, new SpherePhysicsNode());
	((SpherePhysicsNode&)(g->GetPhysicsNode())).SetRadius(radius);
	g->GetPhysicsNode().SetPosition(position);
	g->GetPhysicsNode().SetGravityFactor(EARTH_GRAVITY_FACTOR);
	float massFactor = radius / SPHERE_INITIAL_SIZE;
	g->GetPhysicsNode().SetInvMass(SPHERE_INITIAL_INVMASS / massFactor);
	g->GetPhysicsNode().ActivateNode();
	g->GetPhysicsNode().BuildInertiaMatrix();
	g->ConnectToSystems();
	return g;
}

/*
Makes a flat quad, initially oriented such that we can use it as a simple
floor. 
*/
GameEntity* MyGame::BuildQuadEntity(float size, const Quaternion& direction, const Vector3& position) {
	SceneNode* s = new SceneNode(quad);

	s->SetModelScale(Vector3(size,size,size));
	//Oh if only we had a set texture function...we could make our brick floor again WINK WINK
	s->SetBoundingRadius(size * 2.0f);

	SquarePhysicsNode*p = new SquarePhysicsNode(Quaternion::AxisAngleToQuaterion(Vector3(1, 0, 0), 90.0f), position);
	
	p->ActivateNode();
	GameEntity*g = new GameEntity(s, p);
	p->BuildInertiaMatrix();
	g->ConnectToSystems();
	return g;
}

void MyGame::CreateRestSpheres(Vector3 position, float size)
{
	for (int counter = 0; counter < SPHEPE_PER_AXIS * SPHEPE_PER_AXIS * SPHEPE_PER_AXIS; counter++)
	{
		int x = counter % SPHEPE_PER_AXIS;
		int z = (counter / SPHEPE_PER_AXIS) % SPHEPE_PER_AXIS;
		int y = (counter / (SPHEPE_PER_AXIS * SPHEPE_PER_AXIS)) % SPHEPE_PER_AXIS;

		Vector3 currentPosition = Vector3(position.x + x * 3 * size, position.y + y * 3 * size, position.z + z * 3 * size);

		GameEntity *currentEntity = BuildSphereEntity(size, currentPosition);
		currentEntity->GetPhysicsNode().GetTarget()->SetColour(Vector4((((int)rand()) % 100) / 100.0f, (((int)rand()) % 100) / 100.0f, (((int)rand()) % 100) / 100.0f, 1.0f));
		currentEntity->GetPhysicsNode().Update(0.0f);
		currentEntity->GetPhysicsNode().DeactivateNode();

		allEntities.push_back(currentEntity);
	}
}

GameEntity* MyGame::BuildHeightmapEntity(float size, Vector3& position)
{
	SceneNode* s = new SceneNode(heightmap);

	s->SetModelScale(Vector3(size, size / 4, size));
	s->SetBoundingRadius(size * 5000.0f);

	HeightmapPhysicsNode *p = new HeightmapPhysicsNode(position);
	p->SetTarget(s);
	p->ActivateNode();
	GameEntity*g = new GameEntity(s, p);
	p->BuildInertiaMatrix();
	g->ConnectToSystems();
	return g;
}

void MyGame::DestroySphereNode(SpherePhysicsNode* node)
{
	node->CollisionCounterToZero();
	for (vector<GameEntity*>::iterator i = allEntities.begin(); i != allEntities.end(); ++i) 
	{
		if (&((*i)->GetPhysicsNode()) == node)
		{
			Renderer::GetRenderer().rootMutex.lock();
			(*i)->DisconnectFromSystems();
			Renderer::GetRenderer().rootMutex.unlock();
			destroyedEntitiesMutex.lock();
			destroyedEntities.push_back(*i);
			destroyedEntitiesMutex.unlock();
			allEntities.erase(i);
			for (int i = 0; i < 27; i++)
			{
				int x = i % 3;
				--x;
				int z = (i / 3) % 3;
				--z;
				int y = (i / 9) % 3;
				--y;
				float size = node->GetRadius() / 3;
				GameEntity* newEntity = BuildSphereEntity(size, node->GetPosition() + (Vector3((float)x, (float)y, (float)z) * size));
				newEntity->GetPhysicsNode().SetLinearVelocity(node->GetLinearVelocity() + (Vector3((float)x, (float)y, (float)z) * REMAINANT_SPEED));
				allEntities.push_back(newEntity);
			}
			break;
		}
	}
	node->SetAngularVelocity(Vector3(0, 0, 0));
	node->SetLinearVelocity(Vector3(0, 0, 0));
}

void MyGame::BuildDodgingEntities(float size, Vector3& initialPosition)
{
	SceneNode* s = new SceneNode(sphere);

	s->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));

	s->SetRotation(Matrix4::Translation(initialPosition));

	s->SetModelScale(Vector3(size, size, size));
	s->SetBoundingRadius(size);
	
	SpherePhysicsNode* g = new SpherePhysicsNode();
	g->SetRadius(size);
	g->SetPosition(initialPosition);
	g->SetGravityFactor(NON_GRAVITY);
	float massFactor = size / SPHERE_INITIAL_SIZE;
	g->SetInvMass(SPHERE_INITIAL_INVMASS / massFactor);
	g->ActivateNode();
	g->BuildInertiaMatrix();
	g->SetTarget(s);
	g->dumpingFactor = 0.9f;
	Renderer::GetRenderer().AddNode(s);
	PhysicsSystem::GetPhysicsSystem().SetDodgingEntity(g);
}
