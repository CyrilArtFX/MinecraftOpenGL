#include "game.h"
#include <Assets/assetManager.h>
#include <Inputs/input.h>


Game::Game()
{
}


bool Game::initialize(int wndw_width, int wndw_height, std::string wndw_name, bool wndw_capturemouse)
{
	//  create window and initialize glfw
	window = std::make_unique<Window>(wndw_width, wndw_height, wndw_name, wndw_capturemouse);

	GLFWwindow* gl_window = window->getGLFWwindow();
	if (gl_window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}


	glfwSetWindowUserPointer(gl_window, this);

	glfwSetFramebufferSizeCallback(gl_window, [](GLFWwindow* window, int width, int height)
		{
			auto self = static_cast<Game*>(glfwGetWindowUserPointer(window));
			self->windowResize(window, width, height);
		}
	); //  link window resize callback function

	glfwSetCursorPosCallback(gl_window, [](GLFWwindow* window, double xpos, double ypos)
		{
			Input::ProcessMouse(window, xpos, ypos);
		}
	); //  link mouse pos callback function

	glfwSetScrollCallback(gl_window, [](GLFWwindow* window, double xoffset, double yoffset)
		{
			Input::ProcessScroll(window, xoffset, yoffset);
		}
	); //  link mouse scroll callback function

	glfwSetKeyCallback(gl_window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			Input::ProcessKeyboard(window, key, scancode, action, mods);
		}
	); //  link keyboard callback function

	glfwSetMouseButtonCallback(gl_window, [](GLFWwindow* window, int button, int action, int mods)
		{
			Input::ProcessMouseButton(window, button, action, mods);
		}
	); //  link mouse button callback function


	//  create renderer
	renderer = std::make_shared<Renderer>(Color{ 50, 75, 75, 255 }, *window);

	//  create freecam
	freecam = std::make_shared<Camera>(Vector3::zero);
	freecam->setSpeed(4.0f);


	//  initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}


	//  initialize input system
	Input::Initialize(); 


	//  configure global OpenGL properties
	glEnable(GL_DEPTH_TEST);


	return true;
}


void Game::run()
{
	//  main loop
	while (!glfwWindowShouldClose(window->getGLFWwindow()))
	{
		//  time logic part
		// -----------------
		float current_frame = glfwGetTime();
		deltaTime = current_frame - lastFrame;
		lastFrame = current_frame;


		//  inputs update part
		// -------------
		Input::UpdateInputSystem(); //  update the keys that were registered during the last frame


		//  update part
		// -------------
		engineUpdate(window->getGLFWwindow());

		if (!gamePaused)
		{
			if (scene) scene->update(deltaTime);
		}


		//  rendering part
		// ----------------
		renderer->draw();



		//  events and buffer swap part
		// -----------------------------
		glfwSwapBuffers(window->getGLFWwindow());
		glfwPollEvents();
	}

	//  close engine
	unloadScene();
	AssetManager::DeleteObjects();
}


void Game::close()
{
	//  properly clear GLFW before closing app
	glfwTerminate();
}

void Game::loadScene(std::weak_ptr<Scene> scene_)
{
	scene = scene_.lock();
	scene->load(renderer);
}

void Game::unloadScene()
{
	if (scene) scene->unload();
}


void Game::engineUpdate(GLFWwindow* glWindow)
{
	//  close window when escape is pressed
	if (Input::IsKeyPressed(GLFW_KEY_ESCAPE))
	{
		glfwSetWindowShouldClose(glWindow, true);
	}


	//  pause/unpause the game when p is pressed
	if (Input::IsKeyPressed(GLFW_KEY_P))
	{
		if (!gamePaused) pauseGame();
		else unpauseGame();
	}

	//  active/desactive the freecam mode when m is pressed
	if (Input::IsKeyPressed(GLFW_KEY_SEMICOLON))
	{
		if (!freecamMode) enableFreecam();
		else disableFreecam();
	}

	if (freecamMode)
	{
		//  move freecam
		if (Input::IsKeyDown(GLFW_KEY_W))
			freecam->freecamKeyboard(Camera_Movement::Forward, deltaTime);

		if (Input::IsKeyDown(GLFW_KEY_S))
			freecam->freecamKeyboard(Camera_Movement::Backward, deltaTime);

		if (Input::IsKeyDown(GLFW_KEY_A))
			freecam->freecamKeyboard(Camera_Movement::Left, deltaTime);

		if (Input::IsKeyDown(GLFW_KEY_D))
			freecam->freecamKeyboard(Camera_Movement::Right, deltaTime);

		if (Input::IsKeyDown(GLFW_KEY_SPACE))
			freecam->freecamKeyboard(Camera_Movement::Up, deltaTime);

		if (Input::IsKeyDown(GLFW_KEY_LEFT_SHIFT))
			freecam->freecamKeyboard(Camera_Movement::Down, deltaTime);

		Vector2 mouse_delta = Input::GetMouseDelta();
		freecam->freecamMouseMovement(mouse_delta.x, mouse_delta.y);

		freecam->freecamMouseScroll(Input::GetScrollOffset());
	}
}


void Game::pauseGame()
{
	gamePaused = true;
	std::cout << "Game paused.\n";
}

void Game::unpauseGame()
{
	gamePaused = false;
	if(freecamMode) disableFreecam();
	std::cout << "Game unpaused.\n";
}

void Game::enableFreecam()
{
	freecamMode = true;
	if (!gamePaused) pauseGame();
	std::cout << "Freecam mode enabled.\n";
	freecam->copyCameraTransform(*scene->getCamera().lock());
	renderer->setCamera(freecam);
}

void Game::disableFreecam()
{
	freecamMode = false;
	std::cout << "Freecam mode disabled.\n";
	renderer->setCamera(scene->getCamera());
}


//  window resize callback functions
void Game::windowResize(GLFWwindow* glWindow, int width, int height)
{
	glViewport(0, 0, width, height); //  resize OpenGL viewport when GLFW is resized
	window->changeSize(width, height);
}
