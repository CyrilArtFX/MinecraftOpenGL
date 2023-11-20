#pragma once

#include <Rendering/camera.h>
#include <Objects/object.h>
#include <GLFW/glfw3.h>


class Player
{
public:
	Player(float height, std::weak_ptr<Material> materialToUse);

	void unload();

	void update(float dt);

	void processInputs(GLFWwindow* glWindow, float dt);
	void processMouse(float xOffset, float yOffset);
	void processScroll(float scrollOffset);


	std::weak_ptr<Camera> getCamera() { return camera; }

private:
	std::shared_ptr<Camera> camera;
	std::shared_ptr<VertexArray> vaBlank;
	std::unique_ptr<Object> blankObject;
};

