#include <glad/glad.h>
#include <GLFW/glfw3.h>

//  if this line is not here, the project don't compile (bruh)
#define STB_IMAGE_IMPLEMENTATION

#include "Core/window.h"

#include "Rendering/shader.h"
#include "Rendering/texture.h"
#include "Rendering/vertexArray.h"
#include "Rendering/camera.h"

#include "Maths/Matrix4.h"

#include <iostream>


//  declare functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


//  global settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 720;


//  mouse
bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;


//  time
float deltaTime = 0.0f;
float lastFrame = 0.0f;


//  camera
Camera camera;


int main()
{
	//  create window and initialize glfw
	Window window = Window(SCR_WIDTH, SCR_HEIGHT, "Minecraft OpenGL", true);

	GLFWwindow* glWindow = window.getGLFWwindow();
	if (glWindow == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwSetFramebufferSizeCallback(glWindow, framebuffer_size_callback); //  link window resize callback function
	glfwSetCursorPosCallback(glWindow, mouse_callback); //  link mouse pos callback function
	glfwSetScrollCallback(glWindow, scroll_callback); //  link mouse scroll callback function


	//  initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}


	//  configure global OpenGL properties
	glEnable(GL_DEPTH_TEST);


	//  build and compile shaders
	Shader basicShader("Shaders/basic.vert", "Shaders/basic.frag");
	Shader textureShader("Shaders/texture.vert", "Shaders/texture.frag");
	Shader object3DShader("Shaders/object.vert", "Shaders/object.frag");


	//  cube vertices data
	float cubeVertices[180] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	VertexArray cube = VertexArray(cubeVertices, 36);


	Texture containerTex("Resources/container.jpg", GL_RGB, true);
	Texture faceTex("Resources/awesomeface.png", GL_RGBA, true);


	//  manually set the textures unit on the shader (need to be done only once)
	object3DShader.use(); //  activate the shader on which you want to set the texture unit before doing it
	object3DShader.setInt("texture1", 0);
	object3DShader.setInt("texture2", 1);

	Vector3 cubePositions[] = {
		Vector3{0.0f, 0.0f, 0.0f},
		Vector3{2.0f, 5.0f, 15.0f},
		Vector3{-1.5f, -2.2f, 2.5f},
		Vector3{-3.8f, -2.0f, 12.3f},
		Vector3{2.4f, -0.4f, 3.5f},
		Vector3{-1.7f, 3.0f, 7.5f},
		Vector3{1.3f, -2.0f, 2.5f},
		Vector3{1.5f, 2.0f, 2.5f},
		Vector3{1.5f, 0.2f, 1.5f},
		Vector3{-1.3f, 1.0f, 1.5f}
	};


	camera = Camera(Vector3{ 0.0f, 0.0f, -3.0f });


	//  main loop
	while (!glfwWindowShouldClose(glWindow))
	{
		//  time logic
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;



		//  inputs part
		// -------------
		processInput(glWindow);


		//  rendering part
		// ----------------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //  clear window with flat color

		//  draw
		object3DShader.use();

		glActiveTexture(GL_TEXTURE0);
		containerTex.use();
		glActiveTexture(GL_TEXTURE1);
		faceTex.use();


		float timeValue = glfwGetTime();

		const float radius = 10.0f;
		float camX = sin(timeValue) * radius;
		float camZ = cos(timeValue) * radius;

		Matrix4 view = camera.GetViewMatrix();
		Matrix4 projection = Matrix4::createPerspectiveFOV(Maths::toRadians(camera.getFov()), SCR_WIDTH, SCR_HEIGHT, 0.1f, 100.0f);


		Vector3 rotationAxis = Vector3{ 1.0f, 0.3f, 0.5f };
		rotationAxis.normalize();

		object3DShader.setMatrix4("view", view.getAsFloatPtr());
		object3DShader.setMatrix4("projection", projection.getAsFloatPtr());

		cube.setActive();
		glDrawArrays(GL_TRIANGLES, 0, cube.getNBVertices());

		for (int i = 0; i < 10; i++)
		{
			Quaternion rotation = Quaternion{ rotationAxis, Maths::toRadians(i * 20.0f) };
			Matrix4 model = Matrix4::createFromQuaternion(rotation) * 
				Matrix4::createTranslation(cubePositions[i]);
				
			object3DShader.setMatrix4("model", model.getAsFloatPtr());

			glDrawArrays(GL_TRIANGLES, 0, cube.getNBVertices());
		}



		//  events and buffer swap
		glfwSwapBuffers(glWindow);
		glfwPollEvents();
	}


	//  delete all resources that are not necessary anymore (optionnal)
	cube.deleteObjects();
	basicShader.deleteProgram();
	textureShader.deleteProgram();
	object3DShader.deleteProgram();


	//  properly clear GLFW before closing app
	glfwTerminate();
	return 0;
}


void framebuffer_size_callback(GLFWwindow* glWindow, int width, int height)
{
	glViewport(0, 0, width, height); //  resize OpenGL viewport when GLFW is resized
}

void processInput(GLFWwindow* window)
{
	//  close window when escape is pressed
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	//  move camera
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(Forward, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(Backward, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(Left, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(Right, deltaTime);
}

void mouse_callback(GLFWwindow* glWindow, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = lastX - xpos;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* glWindow, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(float(yoffset));
}
