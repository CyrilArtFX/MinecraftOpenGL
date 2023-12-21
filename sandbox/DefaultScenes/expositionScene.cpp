#include "ExpositionScene.h"

#include <Inputs/input.h>
#include <GLFW/glfw3.h>


ExpositionScene::ExpositionScene()
{
}


void ExpositionScene::loadAssets()
{
	//  shaders, textures and materials
	AssetManager::CreateShaderProgram("lit_object", "Lit/object_lit.vert", "Lit/object_lit.frag", ShaderType::Lit);
	AssetManager::CreateShaderProgram("flat_emissive", "Unlit/flat_emissive.vert", "Unlit/flat_emissive.frag", ShaderType::Unlit);

	AssetManager::LoadTexture("container_diffuse", "container2.png", GL_RGBA, false);
	AssetManager::LoadTexture("container_specular", "container2_specular.png", GL_RGBA, false);
	AssetManager::LoadTexture("container_emissive", "matrix.jpg", GL_RGB, false);

	Material& container_mat = AssetManager::CreateMaterial("container", &AssetManager::GetShader("lit_object"));
	container_mat.addTexture(&AssetManager::GetTexture("container_diffuse"), TextureType::Diffuse); 
	container_mat.addTexture(&AssetManager::GetTexture("container_specular"), TextureType::Specular);
	container_mat.addTexture(&AssetManager::GetTexture("default_black"), TextureType::Emissive);
	container_mat.addParameter("material.shininess", 32.0f);

	Material& light_source_mat = AssetManager::CreateMaterial("light_source", &AssetManager::GetShader("flat_emissive"));
	light_source_mat.addParameter("emissive", 1.0f, 1.0f, 1.0f);


	//  models 
	AssetManager::CreateModel("container");
	AssetManager::GetModel("container").addMesh(&AssetManager::GetSingleMesh("default_cube"), &AssetManager::GetMaterial("container"));

	AssetManager::CreateModel("light_cube");
	AssetManager::GetModel("light_cube").addMesh(&AssetManager::GetSingleMesh("default_cube"), &AssetManager::GetMaterial("light_source"));
}

void ExpositionScene::loadScene()
{
	renderer->setClearColor(Color{ 50, 75, 75, 255 });

	renderer->addMaterial(&AssetManager::GetMaterial("container"));
	renderer->addMaterial(&AssetManager::GetMaterial("light_source"));


	//  camera
	camera.setPosition(Vector3{ 0.0f, 0.0f, -3.0f });
	currentCam = &camera;
	renderer->setCamera(&camera);


	//  objects
	cube1.addModel(&AssetManager::GetModel("container"));
	cube2.addModel(&AssetManager::GetModel("container"));
	cube3.addModel(&AssetManager::GetModel("container"));

	lightCube1.addModel(&AssetManager::GetModel("light_cube"));
	lightCube2.addModel(&AssetManager::GetModel("light_cube"));

	registerObject(&cube1);
	registerObject(&cube2);
	registerObject(&cube3);
	registerObject(&lightCube1);
	registerObject(&lightCube2);

	cube1.setPosition(Vector3{ 0.0f, 0.0f, 0.0f });
	cube1.setRotation(Quaternion{ Vector3::unitY, Maths::toRadians(45.0f) });
	cube2.setPosition(Vector3{ 2.0f, 1.5f, 2.0f });
	cube3.setPosition(Vector3{ 2.0f, -1.0f, -1.0f });
	lightCube1.setPosition(Vector3{ 1.0f, 2.0f, 1.0f });
	lightCube1.setScale(0.2f);
	lightCube2.setPosition(Vector3{ 1.5f, 1.0f, -0.5f });
	lightCube2.setScale(0.2f);


	//  lights
	sunLight.load(Color::white, Vector3{ -0.4f, -0.5f, 1.0f });
	pointLight1.load(Color::white, Vector3{ 1.0f, 2.0f, 1.0f });
	pointLight2.load(Color::white, Vector3{ 1.5f, 1.0f, -0.5f });
	flashLight.load(Color::white, Vector3::zero, Vector3::unitX);

	registerLight(&sunLight);
	registerLight(&pointLight1);
	registerLight(&pointLight2);
	registerLight(&flashLight);
}


void ExpositionScene::unloadScene()
{
	renderer->removeMaterial(&AssetManager::GetMaterial("container"));
	renderer->removeMaterial(&AssetManager::GetMaterial("light_source"));
}


void ExpositionScene::update(float dt)
{
	//  move camera
	if (Input::IsKeyDown(GLFW_KEY_W))
		camera.freecamKeyboard(Forward, dt);

	if (Input::IsKeyDown(GLFW_KEY_S))
		camera.freecamKeyboard(Backward, dt);

	if (Input::IsKeyDown(GLFW_KEY_A))
		camera.freecamKeyboard(Left, dt);

	if (Input::IsKeyDown(GLFW_KEY_D))
		camera.freecamKeyboard(Right, dt);

	if (Input::IsKeyDown(GLFW_KEY_SPACE))
		camera.freecamKeyboard(Up, dt);

	if (Input::IsKeyDown(GLFW_KEY_LEFT_SHIFT))
		camera.freecamKeyboard(Down, dt);

	Vector2 mouse_delta = Input::GetMouseDelta();
	camera.freecamMouseMovement(mouse_delta.x, mouse_delta.y);

	camera.freecamMouseScroll(Input::GetScrollOffset());



	flashLight.setPosition(camera.getPosition());
	flashLight.setDirection(camera.getForward());

	cube3.incrementRotation(Quaternion{ Vector3::unitX, Maths::toRadians(90.0f) * dt });
}