#include "stdafx.h"
#include "Renderer.h"
#include "MyGame.h"

Renderer* Renderer::instance = NULL;

Renderer::Renderer(Window &parent) : OGLRenderer(parent),
showFPS(true)
{
	//set the initial value of all the boolean values
	//those values control what to put onto the screen
	togglePostProcessOn = false;
	snow = true;
	rain = false;
	particleInitialized = false;
	showHelp = false;
	toggleDeferedRenderingOn = false;

	currentPhysicsFPS = 120;
	//initialize the sphere
	sphere = new OBJMesh();
	if (!sphere->LoadOBJMesh(MESHDIR"ico.obj"))
		return;

	//initialize the camera using for the whole scene
	//camera = new Camera(-25, 180, 0.0f, Vector3(2000, 1350, -1100), Vector3(2, 2, 2));
	angle = 200;

	//initialize the light using for the whole scene
	light = new Light(Vector3(0, 0, 0), Vector4(1, 1, 1, 1), 55000.0f);
	
	//Generate all the shaders needed for the Renderer class
	if (!GenerateAllShaders())
		return;
	//initialize the point lights
	if (!GeneratePointLights())
		return;

	SetCurrentShader(normalShader);

	if (!currentShader->LinkProgram())
		return;
	//Generate another buffer object for shadow
	if (!GenerateShadowBuffer())
		return;

	//Generate other buffer objects for blurring post processing
	if (!GenerateBlurBuffer())
		return;

	//Generate other buffer objects for defered rendering
	if (!GenerateDeferedFBO())
		return;

	//initialize the trees and the root node
	root = new SceneNode(NULL, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	
	treeRoot = new SceneNode(NULL, Vector4(1.0f, 1.0f, 1.0f, 1.0f));

	root->AddChild(treeRoot);

	particleRoot = new SceneNode(NULL, Vector4(0, 0, 0, 1));
	
	if (!GenerateTreesAndParticles())
		return;

	//create a snow
	CreateSnow();
	//create a rain
	CreateRain();
	
	
	//generate the quad for water
	quad = Mesh::GenerateQuad();
	quad->SetTexture(SOIL_load_OGL_texture(TEXTUREDIR"water.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));
	quad->SetBumpMap(SOIL_load_OGL_texture(TEXTUREDIR"water_bump.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	//generate the quad for blur
	blurQuad = Mesh::GenerateQuad();
	//generate the quad for the defered rendering
	deferedQuad = Mesh::GenerateQuad();

	SetTextureRepeating(quad->GetTexture(), true);
	SetTextureRepeating(quad->GetBumpMap(), true);
	
	//generate the cube map for skybox.
	cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"rusted_west.jpg", TEXTUREDIR"rusted_east.jpg", TEXTUREDIR"rusted_up.jpg", TEXTUREDIR"rusted_down.jpg", TEXTUREDIR"rusted_south.jpg", TEXTUREDIR"rusted_north.jpg", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
	

	if (!cubeMap || !quad->GetTexture())
		return;
	//finish create an environment

	projMatrix = Matrix4::Perspective(ZNEAR, ZFAR, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);

	//set the basicFont.
	/*
	Just makes a new 'font', a struct containing a texture (of the tahoma font)
	and how many characters across each axis the font contains. (look at the
	font texture in paint.net if you don't quite 'get' this)
	*/
	basicFont = new Font(SOIL_load_OGL_texture(TEXTUREDIR"tahoma.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_COMPRESS_TO_DXT), 16, 16);
	
	waterRotate = 0.0f;
	rotation = 0.0f;

	glEnable(GL_BLEND);
	/*glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);*/
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	init = true;
}


Renderer::~Renderer()
{
	delete basicFont;
//	delete weatherParticle;
//	delete root;
//	delete[] flowerParticles;
	delete shadowShader;
	delete blurShader;
	delete reflectShader;
	delete skyboxShader;
	delete normalShader;
	delete particleShader;
	delete snowShader;
	delete textureShader;
	currentShader = NULL;

	glDeleteTextures(1, &shadowTex);
	glDeleteTextures(1, &bufferNormalTex);
	glDeleteTextures(1, &bufferDepthTex);
	glDeleteTextures(1, &lightEmissiveTex);
	glDeleteTextures(2, &bufferColourTex[2]);
	glDeleteTextures(1, &lightSpecularTex);
	glDeleteTextures(1, &deferedBufferColourTex);

	glDeleteFramebuffers(1, &bufferFBO);
	glDeleteFramebuffers(1, &processFBO);
	glDeleteFramebuffers(1, &deferedBufferFBO);
	glDeleteFramebuffers(1, &pointLightFBO);
	glDeleteFramebuffers(1, &deferedBufferFBO);
	glDeleteFramebuffers(1, &pointLightFBO);

}

bool Renderer::GenerateShadowBuffer()
{
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &shadowFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);

	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

bool Renderer::GenerateAllShaders()
{
	//shader for normal objects, like trees and flowers
	normalShader = new Shader(SHADERDIR"shadowSceneVert.glsl", SHADERDIR"shadowSceneFrag.glsl");
	//shader for 1st pass of shadow rendering
	shadowShader = new Shader(SHADERDIR"shadowVert.glsl", SHADERDIR"shadowFrag.glsl");
	//shader for particles
	particleShader = new Shader(SHADERDIR"vertex.glsl", SHADERDIR"fragment.glsl", SHADERDIR"geometry.glsl");
	snowShader = new Shader(SHADERDIR"vertex.glsl", SHADERDIR"snowFragment.glsl", SHADERDIR"geometry.glsl");
	//shader for blurring post processing
	blurShader = new Shader(SHADERDIR"TexturedVertex.glsl", SHADERDIR"processfrag.glsl");
	//shader for the post processing quad drawing, also basic shader for texturing
	textureShader = new Shader(SHADERDIR"TexturedVertex.glsl", SHADERDIR"TexturedFragment.glsl");
	//shader for the water
	reflectShader = new Shader(SHADERDIR"PerPixelVertex.glsl", SHADERDIR"reflectFragment.glsl");
	//shader for the sky box
	skyboxShader = new Shader(SHADERDIR"skyboxVertex.glsl", SHADERDIR"skyboxFragment.glsl");
	//shader for drawing the initial scene for defered rendering
	sceneShader = new Shader(SHADERDIR"BumpVertex.glsl", SHADERDIR"bufferFragment.glsl");
	//shader for combining the scene into a defered rendered scene
	combineShader = new Shader(SHADERDIR"combineVert.glsl", SHADERDIR"combineFrag.glsl");
	//shader for the point light
	pointlightShader = new Shader(SHADERDIR"pointlightVert.glsl", SHADERDIR"pointlightFrag.glsl");

	if (!sceneShader->LinkProgram() || !combineShader->LinkProgram() || !pointlightShader->LinkProgram())
		return false;

	if (!reflectShader->LinkProgram() || !skyboxShader->LinkProgram() || !textureShader->LinkProgram() || !normalShader->LinkProgram() || !shadowShader->LinkProgram() || !particleShader->LinkProgram() || !snowShader->LinkProgram() || !blurShader->LinkProgram())
		return false;
	else
		return true;

}

bool Renderer::GenerateBlurBuffer()
{
	//generate another frame buffer object for blurring post processing
	glGenTextures(1, &bufferDepthTex);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	for (int i = 0; i < 2; ++i)
	{
		glGenTextures(1, &bufferColourTex[i]);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}

	glGenFramebuffers(1, &bufferFBO);
	glGenFramebuffers(1, &processFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !bufferDepthTex || !bufferColourTex[0])
		return false;
	else
		return true;
}

bool Renderer::GenerateTreesAndParticles()
{
	trees = new Tree[NUMBER_OF_TREES];
	attr[0].numberOfBranches = 4;
	attr[0].length = 200.0f;
	attr[0].diameter = 20.0f;
	attr[0].diversity = 0;
	attr[0].bendRate = 0.4f;
	attr[0].bendCount = 0;
	attr[0].branchTextureInt = SOIL_load_OGL_texture(TEXTUREDIR"Tree Bark02.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	attr[0].branchBumpMap = SOIL_load_OGL_texture(TEXTUREDIR"tree bark_normal.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	particleTexture = SOIL_load_OGL_texture(TEXTUREDIR"particle.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_COMPRESS_TO_DXT);
	SetTextureRepeating(attr[0].branchTextureInt, true);
	SetTextureRepeating(attr[0].branchBumpMap, true);

	attr[0].leavesTextureInt = SOIL_load_OGL_texture(TEXTUREDIR"Leaf_1_Color.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	attr[0].flowerTextureInt = SOIL_load_OGL_texture(TEXTUREDIR"flowersTexture.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	if (!attr[0].branchTextureInt && !attr[0].leavesTextureInt && !attr[0].flowerTextureInt)
		return false;

	attr[1] = attr[0];
	attr[1].numberOfBranches = 5;
	attr[1].bendCount = 0;
	/*attr[2] = attr[0];
	attr[2].numberOfBranches = 9;
	attr[2].bendCount = 1;
	attr[3] = attr[0];
	attr[3].numberOfBranches = 9;
	attr[3].bendCount = 2;
	attr[4] = attr[0];
	attr[4].numberOfBranches = 9;
	attr[4].bendCount = 3;
	attr[5] = attr[0];
	attr[5].numberOfBranches = 4;
	attr[5].bendCount = 5;*/

	int x = 1000;
	int z = 1000;
	numberOfFlowerPartricles = 0;
	for (int i = 0; i < NUMBER_OF_TREES; ++i)
	{
		trees[i].setTreeAttribute(attr[i]);
		trees[i].SetRotation(Matrix4::Translation(Vector3((float)x, 50, (float)z)));
		treeRoot->AddChild(&trees[i]);
		numberOfFlowerPartricles += trees[i].getFlowers().size();
		x += 2000;
		if (x > 3000)
		{
			z += 1000;
			x -= 4000;
		}
	}

	return true;
}

bool Renderer::GeneratePointLights()
{
	pointLights = new Light[LIGHTNUM * LIGHTNUM];
	for (int x = 0; x < LIGHTNUM; ++x)
	{
		for (int z = 0; z < LIGHTNUM; ++z)
		{
			Light &l = pointLights[(x * LIGHTNUM) + z];

			float xPos = (RAW_WIDTH * HEIGHTMAP_X / (LIGHTNUM - 1)) * x;
			float zPos = (RAW_HEIGHT*HEIGHTMAP_Z / (LIGHTNUM - 1)) * z;
			l.SetPosition(Vector3(xPos, 100.0f, zPos));

			float r = 0.1f + (float)(rand() % 129) / 128.0f;
			float g = 0.1f + (float)(rand() % 129) / 128.0f;
			float b = 0.1f + (float)(rand() % 129) / 128.0f;
			l.SetColour(Vector4(r, g, b, 1.0f));

			float radius = (RAW_WIDTH*HEIGHTMAP_X / LIGHTNUM);
			l.SetRadius(radius);
		}
	}
	return true;
}

bool Renderer::GenerateDeferedFBO()
{
	glGenFramebuffers(1, &deferedBufferFBO);
	glGenFramebuffers(1, &pointLightFBO);

	GLenum buffers[2];
	buffers[0] = GL_COLOR_ATTACHMENT0;
	buffers[1] = GL_COLOR_ATTACHMENT1;

	GenerateScreenTexture(bufferDepthTex, true);
	GenerateScreenTexture(deferedBufferColourTex);
	GenerateScreenTexture(bufferNormalTex);
	GenerateScreenTexture(lightEmissiveTex);
	GenerateScreenTexture(lightSpecularTex);

	glBindFramebuffer(GL_FRAMEBUFFER, deferedBufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, deferedBufferColourTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, bufferNormalTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);

	glDrawBuffers(2, buffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;

	glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightEmissiveTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, lightSpecularTex, 0);
	glDrawBuffers(2, buffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;

	return true;
}

void Renderer::UpdateScene(float msec)	{

	//downwards is the speed control method for the scene
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_NUMPAD8))
		msec *= 4;
	else if (Window::GetKeyboard()->KeyDown(KEYBOARD_NUMPAD6))
		msec *= 8;
	else if (Window::GetKeyboard()->KeyDown(KEYBOARD_NUMPAD2))
		msec *= 0.5f;
	else if (Window::GetKeyboard()->KeyDown(KEYBOARD_NUMPAD4))
		msec *= 0.2f;
	else if (Window::GetKeyboard()->KeyDown(KEYBOARD_NUMPAD5))
		msec = 0;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_P))
	{
		togglePostProcessOn = !togglePostProcessOn;
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_O))
	{
		toggleDeferedRenderingOn = !toggleDeferedRenderingOn;
	}
	{
		vector<SceneNode*> rootChildren = root->getChildren();
		for (vector<SceneNode*>::iterator i = rootChildren.begin(); i != rootChildren.end(); ++i)
		{
			(*i)->Update(msec);
		}
	}
	angle += msec / 100;
	if (angle > 360)
		angle -= 360;
	Vector3 centerCoord = Vector3(RAW_WIDTH * HEIGHTMAP_X / 2, 0, RAW_HEIGHT * HEIGHTMAP_Z / 2);
	light->SetPosition(centerCoord + Vector3(sin(float(angle) / 360.0f * 6.28f) * RAW_WIDTH * HEIGHTMAP_X , 1200, cos(float(angle) / 360.0f * 6.28f) * RAW_WIDTH * HEIGHTMAP_X ));
	auto thread_generation = [&](int thread_id, int tree_ID)
	{
		trees[tree_ID].grow(msec);
		trees[tree_ID].Update(msec);

	};
	thread thread[NUMBER_OF_TREES];
	for (int i = 0; i < NUMBER_OF_TREES; i++)
	{
		thread[i] = std::thread(thread_generation, i, i);
	}
	for (int i = 0; i < NUMBER_OF_TREES; i++)
	{
		thread[i].join();
	}
	if (!particleInitialized && trees[0].getFlowers()[0]->GetModelScale().x > MAXIMUM_TREE_SCALE * 2)
	{
		flowerParticles = new SceneNode[numberOfFlowerPartricles]();
		/*initialize all the particles and add to particles node.
		the transformation of the particles are world tranformation
		of all the flowers on the tree*/
		int number = 0;
		for (int i = 0; i < NUMBER_OF_TREES; ++i)
		{
			for (unsigned int k = 0; k < trees[i].getFlowers().size(); k++)
			{
				ParticleEmitter* particleEmitter = new ParticleEmitter(particleTexture);
				particleEmitter->SetParticleSpeed(0.001f);
				particleEmitter->SetParticleSize(8.0f);
				particleEmitter->SetParticleVariance(1.0f);
				particleEmitter->SetLaunchParticles(10);
				flowerParticles[number].SetMesh(particleEmitter);
				flowerParticles[number].SetRotation(trees[i].getFlowers()[k]->GetWorldTransform());
				flowerParticles[number].SetModelScale(Vector3(100, 100, 100));
				flowerParticles[number].Update(0);
				particleRoot->AddChild(&(flowerParticles[number]));
				++number;
			}
		}
		particleInitialized = true;
	}
	if (particleInitialized && trees[0].getFlowers()[0]->GetModelScale().x > MAXIMUM_TREE_SCALE * 2)
	{
		for (int i = 0; i < numberOfFlowerPartricles; i++)
		{
			ParticleEmitter* currentEmitter = (ParticleEmitter*)flowerParticles[i].GetMesh();
			currentEmitter->Update(msec);
		}
	}
	if (snow)
	{
		for (int i = 1; i < NUMBER_OF_WEATHER_NODES; i++)
		{
			ParticleEmitter* currentEmitter = (ParticleEmitter*)snowParticleNode[i].GetMesh();
			currentEmitter->Update(msec);
		}
	}
	if (rain)
	{
		for (int i = 1; i < NUMBER_OF_WEATHER_NODES; i++)
		{
			ParticleEmitter* currentEmitter = (ParticleEmitter*)rainParticleNode[i].GetMesh();
			currentEmitter->Update(msec);
		}
	}
	viewMatrix = camera->BuildViewMatrix();
	waterRotate += msec / 1000.0f;
	if (toggleDeferedRenderingOn)
		rotation = msec * 0.01f;
}

void Renderer::RenderScene()	{

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// Clear Screen And Depth Buffer
	if (togglePostProcessOn)
	{
			
		glUseProgram(currentShader->GetProgram());	//Enable the shader...

		projMatrix = Matrix4::Perspective(ZNEAR, ZFAR, (float)width / (float)height, 45.0f);
		glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		DrawSkybox();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		DrawShadowScene();
		glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);


		DrawCombinedScene();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		DrawPostProcess();
		PresentScene();
	}
	if (toggleDeferedRenderingOn)
	{
		glUseProgram(currentShader->GetProgram());

		projMatrix = Matrix4::Perspective(ZNEAR, ZFAR, (float)width / (float)height, 45.0f);
		glBindFramebuffer(GL_FRAMEBUFFER, deferedBufferFBO);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		DrawSkybox();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		DrawShadowScene();
		glBindFramebuffer(GL_FRAMEBUFFER, deferedBufferFBO);


		DrawCombinedScene();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glUseProgram(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		DrawPointLights();
		CombineBuffers();
	}
	if (!toggleDeferedRenderingOn && !togglePostProcessOn)
	{
		glUseProgram(currentShader->GetProgram());	//Enable the shader...

		projMatrix = Matrix4::Perspective(ZNEAR, ZFAR, (float)width / (float)height, 45.0f);

		DrawSkybox();

		DrawShadowScene();

		DrawCombinedScene();
	}


	if (showFPS)
	{
		glDisable(GL_DEPTH_TEST);
		DrawText("FPS:" + std::to_string(currentFPS), Vector3(0, 0, 0), 16.0f);
		DrawText("Physics FPS:" + std::to_string(currentPhysicsFPS), Vector3(0, 16, 0), 16.0f);
		DrawText("Collision Made:" + std::to_string(PhysicsSystem::GetPhysicsSystem().GetCollisionCounter()), Vector3(0, 32, 0), 16.0f);
		string isOn = (PhysicsSystem::GetPhysicsSystem().GetUseBroadPhaseCulling()) ? "ON" : "OFF";
		DrawText("Sort And Sweep:" + isOn, Vector3(0, 48, 0), 16.0f);
		DrawText("Score:" + std::to_string(PhysicsSystem::GetPhysicsSystem().GetScore()), Vector3(0, 64, 0), 16.0f);
		DrawText("1: Shoot Projectile", Vector3(0, 80, 0), 16.0f);
		DrawText("Projectile Speed:" + std::to_string(((MyGame&)(MyGame::GetGameClass())).GetSpeedFactor()), Vector3(0, 96, 0), 16.0f);
		DrawText("Projectile Size:" + std::to_string(((MyGame&)(MyGame::GetGameClass())).GetSizeFactor()), Vector3(0, 112, 0), 16.0f);

		//draw all the things that outside needs to be drawn
		for (unsigned int i = 0; i < text_attributes.size(); i++)
		{
			DrawText(*(text_attributes[i]->textContent), text_attributes[i]->positionOnScreen, text_attributes[i]->fontSize);
		}

		DrawText("H: More Instruction", Vector3(0, 416, 0), 16.0f);

		glEnable(GL_DEPTH_TEST);
	}
	if (showHelp)
	{
		glDisable(GL_DEPTH_TEST);
		DrawText("L: Light View", Vector3(0, 432, 0), 16.0f);
		DrawText("M: Polygon Mode", Vector3(0, 448, 0), 16.0f);
		DrawText("R: Reset Trees and Camera", Vector3(0, 464, 0), 16.0f);
		DrawText("Q: Toggle Rain / Snow", Vector3(0, 480, 0), 16.0f);
		DrawText("B: Toggle Sort & Sweep", Vector3(0, 496, 0), 16.0f);
		DrawText("T: Toggle Sphere Texture", Vector3(0, 512, 0), 16.0f);
		DrawText("Numpad8: 4 X Speed", Vector3(0, 528, 0), 16.0f);
		DrawText("Numpad6: 8 X Speed", Vector3(0, 544, 0), 16.0f);
		DrawText("Numpad2: 0.2 X Speed", Vector3(0, 560, 0), 16.0f);
		DrawText("Numpad4: 0.5 X Speed", Vector3(0, 576, 0), 16.0f);
		DrawText("Numpad5: Pause", Vector3(0, 592, 0), 16.0f);
		DrawText("Up: Projectile Speed Up", Vector3(0, 608, 0), 16.0f);
		DrawText("Down: Projectile Speed Down", Vector3(0, 624, 0), 16.0f);
		DrawText("Left: Projectile Size Increase", Vector3(0, 640, 0), 16.0f);
		DrawText("Right: Projectile Size Decrease", Vector3(0, 656, 0), 16.0f);

		
		glEnable(GL_DEPTH_TEST);
	}
	showHelp = false;
	glUseProgram(0);	//That's everything!

	SwapBuffers();
}

/*
Draw a line of text on screen. If we were to have a 'static' line of text, we'd
probably want to keep the TextMesh around to save processing it every frame,
but for a simple demonstration, this is fine...
*/
void Renderer::DrawText(const std::string &text, const Vector3 &position, const float size, const bool perspective)	{
	//Create a new temporary TextMesh, using our line of text and our font
	TextMesh* mesh = new TextMesh(text, *basicFont);
	Matrix4 mdMTX = modelMatrix;
	Matrix4 projMTX = projMatrix;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	SetCurrentShader(textureShader);

	//This just does simple matrix setup to render in either perspective or
	//orthographic mode, there's nothing here that's particularly tricky.
	if (perspective) {
		modelMatrix = Matrix4::Translation(position) * Matrix4::Scale(Vector3(size, size, 1));
		viewMatrix = camera->BuildViewMatrix();
		projMatrix = Matrix4::Perspective(ZNEAR, ZFAR, (float)width / (float)height, 45.0f);
	}
	else{
		//In ortho mode, we subtract the y from the height, so that a height of 0
		//is at the top left of the screen, which is more intuitive
		//(for me anyway...)
		modelMatrix = Matrix4::Translation(Vector3(position.x, height - position.y, position.z)) * Matrix4::Scale(Vector3(size, size, 1));
		viewMatrix.ToIdentity();
		projMatrix = Matrix4::Orthographic(-ZNEAR, ZNEAR, (float)width, 0.0f, (float)height, 0.0f);
	}
	//Either way, we update the matrices, and draw the mesh
	UpdateShaderMatrices();
	mesh->Draw();
	modelMatrix = mdMTX;
	projMatrix = projMTX;
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);
	delete mesh; //Once it's drawn, we don't need it anymore!
}

void Renderer::DrawNode(SceneNode* n)
{

	if (n->GetMesh())
	{
		
		Matrix4 transform = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
		glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "modelMatrix"), 1, false, (float*)&transform);

		Matrix4 tempMatrix = textureMatrix * transform;
		glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "textureMatrix"), 1, false, *&tempMatrix.values);

		glUniform4fv(glGetUniformLocation(currentShader->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());
		if (n->GetMesh()->GetTexture())
		{
			glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "useTexture"), 1);
		}
		else
		{
			glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "useTexture"), 0);
		}

		n->Draw();
	}
	for (vector<SceneNode*>::const_iterator i = n->GetChildIteratorStart(); i != n->GetChildIteratorEnd(); ++i)
		DrawNode(*i);

	

}

void Renderer::DrawSkybox()
{
	glDepthMask(GL_FALSE);
	SetCurrentShader(skyboxShader);

	UpdateShaderMatrices();
	glDisable(GL_CULL_FACE);
	quad->Draw();

	glEnable(GL_CULL_FACE);
	glUseProgram(0);
	glDepthMask(GL_TRUE);
}

void Renderer::DrawWater()
{

	Matrix4 oldModelMatrix = modelMatrix;
	Matrix4 oldTxtMatrix = textureMatrix;
	SetCurrentShader(reflectShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "cubeTex"), 2);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	float heightX = (RAW_WIDTH * HEIGHTMAP_X / 2.0f);
	float heightY = 256 * HEIGHTMAP_Y / 3.0f;
	float heightZ = (RAW_HEIGHT * HEIGHTMAP_Z / 2.0f);

	modelMatrix = Matrix4::Translation(Vector3(heightX, heightY, heightZ)) * Matrix4::Scale(Vector3(heightX, 1, heightZ)) * Matrix4::Rotation(90, Vector3(1.0f, 0.0f, 0.0f));
	textureMatrix = Matrix4::Scale(Vector3(10.0f, 10.0f, 10.0f)) * Matrix4::Rotation(waterRotate, Vector3(0.0f, 0.0f, 1.0f));
	UpdateShaderMatrices();

	quad->Draw();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	modelMatrix = oldModelMatrix;
	textureMatrix = oldTxtMatrix;

	glUseProgram(0);
}

void Renderer::DrawShadowScene()
{
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	SetCurrentShader(shadowShader);

	projMatrix = Matrix4::Perspective(SHADOW_ZNEAR, ZFAR, (float)width / (float)height, 45.0f);
	viewMatrix = Matrix4::BuildViewMatrix(light->GetPosition(), Vector3(RAW_WIDTH * HEIGHTMAP_X / 2, 0, RAW_WIDTH * HEIGHTMAP_X / 2));
	textureMatrix = biasMatrix*(projMatrix*viewMatrix);
	UpdateShaderMatrices();
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);

	DrawRootNode();
	
	glUseProgram(0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);
	projMatrix = Matrix4::Perspective(ZNEAR, ZFAR, (float)width / (float)height, 45.0f);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void Renderer::DrawCombinedScene()
{
	SetCurrentShader(normalShader);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "bumpTex"), 1);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "shadowTex"), 2);

	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	SetShaderLight(*light);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	viewMatrix = camera->BuildViewMatrix();

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_L)) {
		viewMatrix = Matrix4::BuildViewMatrix(light->GetPosition(), Vector3(RAW_WIDTH * HEIGHTMAP_X / 2, 0, RAW_WIDTH * HEIGHTMAP_X / 2));
	}

	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);

	DrawRootNode();

	if (trees[0].getFlowers()[0]->GetModelScale().x > MAXIMUM_TREE_SCALE * 2)
	{
		SetCurrentShader(particleShader);
		SetShaderParticleSize(8);
		UpdateShaderMatrices();
		DrawNode(particleRoot);
	}
	if (snow && !rain)
	{
		SetCurrentShader(snowShader);
		SetShaderParticleSize(8);
		UpdateShaderMatrices();
		DrawNode(&(snowParticleNode[0]));
	}
	if (rain && !snow)
	{
		SetCurrentShader(snowShader);
		SetShaderParticleSize(8);
		UpdateShaderMatrices();
		DrawNode(&(rainParticleNode[0]));
	}

	glUseProgram(0);
}

void Renderer::reset()
{
	for (int i = 0; i < NUMBER_OF_TREES; i++)
	{
		trees[i].resetTree();
	}
	waterRotate = 0;
	camera->SetPostion(Vector3(2000, 1350, -1100));
	camera->SetPitch(-25);
	camera->SetYaw(180);
	Vector3 centerCoord = Vector3(RAW_WIDTH * HEIGHTMAP_X / 2, 0, RAW_HEIGHT * HEIGHTMAP_Z / 2);
	light->SetPosition(centerCoord + Vector3((float)sin(0) * RAW_WIDTH * HEIGHTMAP_X, 1200, (float)cos(0) * RAW_WIDTH * HEIGHTMAP_X));
	angle = 0;
}

string Renderer::getMemoryUsedForTrees()
{
	unsigned int memory = 0;
	string memoryStr;
	for (int i = 0; i < NUMBER_OF_TREES; ++i)
	{
		memory += trees[i].getMemory();
	}
	if (memory > 1000 && memory <= 1000000)
	{
		memoryStr = std::to_string((float)memory / 1000) + "KB";
	}
	else if (memory > 1000000)
	{
		memoryStr = std::to_string(((float)memory / 1000) / 1000) + "MB";
	}

	return "Memory: " + memoryStr;
}

void Renderer::SetShaderParticleSize(float f) {
	glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "particleSize"), f);
}

void Renderer::CreateSnow()
{
	snowTex = SOIL_load_OGL_texture(TEXTUREDIR"Snowflake.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	snowParticleNode = new SceneNode[NUMBER_OF_WEATHER_NODES];
	snowParticleNode[0].SetTranslation(Vector3(0, 2000, 0));
	snowParticleNode[0].SetModelScale(Vector3(1, 1, 1));
	for (int i = 1; i < NUMBER_OF_WEATHER_NODES; i++)
	{
		ParticleEmitter *weatherParticle = new ParticleEmitter(snowTex);
		weatherParticle->SetParticleRate(15000.0f);
		weatherParticle->SetParticleLifetime(20000.0f);
		weatherParticle->SetParticleSize(8.0f);
		weatherParticle->SetParticleVariance(0.0f);
		weatherParticle->SetParticleSpeed(0.001f);
		weatherParticle->SetLaunchParticles(1);
		weatherParticle->SetDirection(Vector3(0, -1, 0));
		weatherParticle->Update((float)(rand() % 100000));
		snowParticleNode[i].SetMesh(weatherParticle);
		snowParticleNode[i].SetTranslation(Vector3((rand() % ((int)(RAW_WIDTH * HEIGHTMAP_X))) / 100.0f, 0, (rand() % ((int)(RAW_HEIGHT * HEIGHTMAP_Z))) / 100.0f));
		snowParticleNode[i].SetModelScale(Vector3(100, 100, 100));
		snowParticleNode[i].Update(0);
		snowParticleNode[0].AddChild(&(snowParticleNode[i]));
	}
	snowParticleNode[0].Update(0);
}

void Renderer::CreateRain()
{
	rainTex = SOIL_load_OGL_texture(TEXTUREDIR"RainDrop.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	rainParticleNode = new SceneNode[NUMBER_OF_WEATHER_NODES];
	rainParticleNode[0].SetTranslation(Vector3(0, 2000, 0));
	rainParticleNode[0].SetModelScale(Vector3(1, 1, 1));
	for (int i = 1; i < NUMBER_OF_WEATHER_NODES; i++)
	{
		ParticleEmitter *weatherParticle = new ParticleEmitter(rainTex);
		weatherParticle->SetParticleRate(100.0f);
		weatherParticle->SetParticleLifetime(200.0f);
		weatherParticle->SetParticleSize(8.0f);
		weatherParticle->SetParticleVariance(0.0f);
		weatherParticle->SetParticleSpeed(1.0f);
		weatherParticle->SetLaunchParticles(1);
		weatherParticle->SetDirection(Vector3(0, -1, 0));
		weatherParticle->Update((float)(rand() % 100000));
		rainParticleNode[i].SetMesh(weatherParticle);
		rainParticleNode[i].SetTranslation(Vector3((rand() % ((int)(RAW_WIDTH * HEIGHTMAP_X))) / 10.0f, 0, (rand() % ((int)(RAW_HEIGHT * HEIGHTMAP_Z))) / 10.0f));
		rainParticleNode[i].SetModelScale(Vector3(10, 10, 10));
		rainParticleNode[i].Update(0);
		rainParticleNode[0].AddChild(&(rainParticleNode[i]));
	}
	rainParticleNode[0].Update(0);
}

void Renderer::DrawPostProcess()
{
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	SetCurrentShader(blurShader);
	projMatrix = Matrix4::Orthographic(-1, 1, 1, -1, -1, 1);
	viewMatrix.ToIdentity();
	UpdateShaderMatrices();

	glDisable(GL_DEPTH_TEST);

	glUniform2f(glGetUniformLocation(currentShader->GetProgram(), "pixelSize"), 1.0f / width, 1.0f / height);

	for (int i = 0; i < POST_PASSES; ++i)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);
		glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "isVertical"), 0);
		blurQuad->SetTexture(bufferColourTex[0]);
		blurQuad->Draw();

		glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "isVertical"), 1);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);
		blurQuad->SetTexture(bufferColourTex[1]);
		blurQuad->Draw();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);

	glEnable(GL_DEPTH_TEST);
}

void Renderer::PresentScene()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	SetCurrentShader(textureShader);
	projMatrix = Matrix4::Orthographic(-1, 1, 1, -1, -1, 1);
	viewMatrix.ToIdentity();
	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();
	UpdateShaderMatrices();
	blurQuad->SetTexture(bufferColourTex[0]);
	blurQuad->Draw();
	glUseProgram(0);
}

void Renderer::toggleWeather()
{
	if (snow)
	{
		snow = false;
		rain = true;
	}
	else if (rain)
	{
		snow = false;
		rain = false;
	}
	else
	{
		snow = true;
		rain = false;
	}
}

void Renderer::GenerateScreenTexture(GLuint &into, bool depth)
{
	glGenTextures(1, &into);
	glBindTexture(GL_TEXTURE_2D, into);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, depth ? GL_DEPTH_COMPONENT24 : GL_RGBA8, width, height, 0, depth ? GL_DEPTH_COMPONENT : GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::DrawPointLights()
{
	SetCurrentShader(pointlightShader);
	glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBO);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glBlendFunc(GL_ONE, GL_ONE);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "depthTex"), 3);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "normTex"), 4);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, bufferNormalTex);

	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "cameraPos"),(GLsizei) 1.0f, (float*)&camera->GetPosition());
	glUniform2f(glGetUniformLocation(currentShader->GetProgram(), "pixelSize"), 1.0f / width, 1.0f / height);

	Vector3 translate = Vector3((RAW_HEIGHT * HEIGHTMAP_X / 2.0f), 500, (RAW_HEIGHT*HEIGHTMAP_Z / 2.0f));

	Matrix4 pushMatrix = Matrix4::Translation(translate);
	Matrix4 popMatrix = Matrix4::Translation(-translate);

	for (int x = 0; x < LIGHTNUM; ++x)
	{
		for (int z = 0; z < LIGHTNUM; ++z)
		{
			Light &l = pointLights[(x * LIGHTNUM) + z];
			float radius = l.GetRadius();

			modelMatrix = pushMatrix * Matrix4::Rotation(rotation, Vector3(0, 1, 0)) * popMatrix * Matrix4::Translation(l.GetPosition()) * Matrix4::Scale(Vector3(radius, radius, radius));

			l.SetPosition(modelMatrix.GetPositionVector());

			SetShaderLight(l);

			UpdateShaderMatrices();

			float dist = (l.GetPosition() - camera->GetPosition()).Length();

			if (dist < radius)
			{
				glCullFace(GL_FRONT);
			}
			else
			{
				glCullFace(GL_BACK);
			}

			sphere->Draw();
		}
	}

	glCullFace(GL_BACK);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.2f, 0.2f, 0.2f, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
}

void Renderer::CombineBuffers()
{
	SetCurrentShader(combineShader);

	projMatrix = Matrix4::Orthographic(-1, 1, 1, -1, -1, 1);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	textureMatrix.ToIdentity();
	UpdateShaderMatrices();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 2);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "emissiveTex"), 3);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "specularTex"), 4);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, deferedBufferColourTex);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, lightEmissiveTex);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, lightSpecularTex);

	deferedQuad->Draw();

	glUseProgram(0);
}

void	Renderer::BuildNodeLists(SceneNode* from)	{
	Vector3 direction = from->GetWorldTransform().GetPositionVector() - camera->GetPosition();
	from->SetCameraDistance(Vector3::Dot(direction, direction));

	if (frameFrustum.InsideFrustum(*from)) {
		if (from->GetColour().w < 1.0f) {
			transparentNodeList.push_back(from);
		}
		else{
			nodeList.push_back(from);
		}
	}

	for (vector<SceneNode*>::const_iterator i = from->GetChildIteratorStart(); i != from->GetChildIteratorEnd(); ++i) {
		BuildNodeLists((*i));
	}
}

void	Renderer::DrawNodes()	 {
	for (vector<SceneNode*>::const_iterator i = nodeList.begin(); i != nodeList.end(); ++i) {
		DrawNode((*i));
	}

	for (vector<SceneNode*>::const_reverse_iterator i = transparentNodeList.rbegin(); i != transparentNodeList.rend(); ++i) {
		DrawNode((*i));
	}
}

void	Renderer::SortNodeLists()	{
	std::sort(transparentNodeList.begin(), transparentNodeList.end(), SceneNode::CompareByCameraDistance);
	std::sort(nodeList.begin(), nodeList.end(), SceneNode::CompareByCameraDistance);
}

void	Renderer::ClearNodeLists()	{
	transparentNodeList.clear();
	nodeList.clear();
}

void Renderer::DrawRootNode()
{
	vector<SceneNode*> rootsChildren = root->getChildren();

	for (vector<SceneNode*>::iterator i = rootsChildren.begin(); i != rootsChildren.end(); i++)
	{
		DrawNode(*i);
	}
}
