// GraphicsCourseWork.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Window.h"
#include "Renderer.h"
#include "MyGame.h"
#include <time.h>

#pragma comment(lib, "nclgl.lib")

//use this method to quit the thread runing physics systems, and destroy everything
int Quit(bool pause = false, const string &reason = "") {
	PhysicsSystem::Destroy();
	Window::Destroy();
	Renderer::Destroy();

	if (pause) {
		std::cout << reason << std::endl;
		system("PAUSE");
	}

	return 0;
}

//this is the lambda function created for the physics thread. use this to create
//physics thread.
auto updatePhysicsThread = [&](MyGame* game, bool* running)
{
	float lastTime = Window::GetWindow().GetTimer()->GetMS();
	while (*running)
	{
		float currentTime = Window::GetWindow().GetTimer()->GetMS();
		float timePassed = currentTime - lastTime;
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_NUMPAD5))
		{
			lastTime = currentTime;
			continue;
		}
		else if (Window::GetKeyboard()->KeyDown(KEYBOARD_NUMPAD8))
			timePassed *= 4;
		else if (Window::GetKeyboard()->KeyDown(KEYBOARD_NUMPAD6))
			timePassed *= 8;
		else if (Window::GetKeyboard()->KeyDown(KEYBOARD_NUMPAD2))
			timePassed *= 0.5f;
		else if (Window::GetKeyboard()->KeyDown(KEYBOARD_NUMPAD4))
			timePassed *= 0.2f;

		game->UpdatePhysics(timePassed);
		lastTime = currentTime;
		
	}
};

int main() {
	//initialise the random number generator.
	srand((unsigned int)time(NULL));
	if (!Window::Initialise("Game Technologies", 1920, 1080, true)) {
		return Quit(true, "Window failed to initialise!");
	}

	if (!Renderer::Initialise()) {
		return Quit(true, "Renderer failed to initialise!");
	}

	PhysicsSystem::Initialise();


	MyGame* game = new MyGame();

	bool running;

	double lastTime = Window::GetWindow().GetTimer()->GetMS();
	int nbFrames = 0;
	Renderer::GetRenderer().setCurrentFPS(60);

	//collision detection counter text
	string* cDCT = new string("Number of Collision Detection Made: " + std::to_string(0));

	//initialize the collisionDetection counter text
	text_attribute collisionDetectionCounterText;
	collisionDetectionCounterText.fontSize = DEFAULT_FONT_SIZE;
	collisionDetectionCounterText.positionOnScreen = Vector3(300, 0, 0);
	collisionDetectionCounterText.textContent = cDCT;


	Renderer::GetRenderer().text_attributes.push_back(&collisionDetectionCounterText);

	Window::GetWindow().LockMouseToWindow(true);
	Window::GetWindow().ShowOSPointer(false);

	//create the physics thread.
	//update physics thread separately from the renderer.
	std::thread physicsThread(updatePhysicsThread, game, &running);

	while (Window::GetWindow().UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)){
		double currentTime = Window::GetWindow().GetTimer()->GetMS();
		++nbFrames;
		if (currentTime - lastTime > 1000)
		{
			Renderer::GetRenderer().setCurrentFPS(nbFrames);
			Renderer::GetRenderer().setCurrentPFPS(PhysicsSystem::GetPhysicsSystem().GetFPSCounter());
			PhysicsSystem::GetPhysicsSystem().FPSCounterToZero();
			nbFrames = 0;
			lastTime = currentTime;
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_P))
		{
			//enter the polygon mode
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_Q))
		{
			//toggle the weather, it's snow->rain->nothing->snow....
			Renderer::GetRenderer().toggleWeather();
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_H))
		{
			//toggle the more instuction menu
			Renderer::GetRenderer().toggleShowHelp();
		}
		//reset the whole scene
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_R))
			Renderer::GetRenderer().reset();


		float i = Window::GetWindow().GetTimer()->GetTimedMS();

		//put the collision detection number into the string to show on the screen
		*cDCT = "Number of Collision Detection Made: " + std::to_string(PhysicsSystem::GetPhysicsSystem().GetCollisionDetectionCounter());
		
		game->UpdateRenderer(i);
		game->UpdateGame(i);	//Update our game logic
	}

	//after everything is finished and the program is about to end,
	//put running into false to stop the update of physics system.
	running = false;

	//after that, wait for the physics system to join, namely to terminate.
	physicsThread.join();
 	delete game;

	return  Quit();
}

