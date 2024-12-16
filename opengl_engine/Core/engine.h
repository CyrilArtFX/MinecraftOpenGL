#pragma once

#include "game.h"
#include "window.h"
#include <Rendering/rendererOpenGL.h>
#include <Rendering/camera.h>
#include <Rendering/texture.h>
#include <Rendering/Text/textRendererComponent.h>

#include <Audio/audioManager.h>
#include <Audio/audioSound.h>

#include <Log/logManager.h>

#include <Maths/matrix4.h>
#include <Maths/vector3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>

class Engine
{
public:
	Engine();
	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;

	bool initialize(int wndw_width = 1280, int wndw_height = 720, std::string wndw_name = "OpenGL Engine", bool wndw_capturemouse = true);
	void run();
	void close();

	void loadGame(std::weak_ptr<Game> game_);
	void unloadGame();

	void engineUpdate(GLFWwindow* glWindow);

	//  window resize function redirected
	void windowResize(GLFWwindow* glWindow, int width, int height);

private:
	//  game
	std::shared_ptr<Game> game;

	//  window
	Window window;

	//  renderer
	RendererOpenGL* renderer;

	//  audio manager
	AudioManager* audio;

	//  log manager
	LogManager* log;

	//  freecam
	Camera freecam;

	//  debug text
	TextRendererComponent* fpsText;
	int frameCounter = 0;
	float frameTimeCounter = 0.0f;

	//  time
	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	//  pause, freecam and debug view
	bool gamePaused{ false };
	bool oneFrame{ false };
	bool freecamMode{ false };
	bool debugViewMode{ false };
	void pauseGame();
	void unpauseGame();
	void advanceOneFrame();
	void enableFreecam();
	void disableFreecam();
	void enableDebugView();
	void disableDebugView();
};
