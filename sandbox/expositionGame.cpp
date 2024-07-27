#include "expositionGame.h"
#include <Assets/defaultAssets.h>
#include <Assets/assetManager.h>

ExpositionGame::ExpositionGame()
{
}

void ExpositionGame::updateGame(float dt)
{
}


void ExpositionGame::loadGameAssets()
{
	DefaultAssets::LoadDefaultAssets(*renderer);

	//  shaders, textures and materials
	AssetManager::CreateShaderProgram("lit_object", "Lit/object_lit.vert", "Lit/object_lit.frag", ShaderType::Lit);

	AssetManager::LoadTexture("container_diffuse", "container2.png", false);
	AssetManager::LoadTexture("container_specular", "container2_specular.png", false);
	AssetManager::LoadTexture("container_emissive", "matrix.jpg", false);

	AssetManager::LoadTexture("backpack_diffuse", "backpack/backpack_basecolor.jpg", false);
	AssetManager::LoadTexture("backpack_specular", "backpack/backpack_specular.jpg", false);

	Material& container_mat = AssetManager::CreateMaterial("container", &AssetManager::GetShader("lit_object"));
	container_mat.addTexture(&AssetManager::GetTexture("container_diffuse"), TextureType::Diffuse);
	container_mat.addTexture(&AssetManager::GetTexture("container_specular"), TextureType::Specular);
	container_mat.addTexture(&AssetManager::GetTexture("default_black"), TextureType::Emissive);
	container_mat.addParameter("material.shininess", 32.0f);

	Material& backpack_mat = AssetManager::CreateMaterial("backpack", &AssetManager::GetShader("lit_object"));
	backpack_mat.addTexture(&AssetManager::GetTexture("backpack_diffuse"), TextureType::Diffuse);
	backpack_mat.addTexture(&AssetManager::GetTexture("default_black"), TextureType::Specular);
	backpack_mat.addTexture(&AssetManager::GetTexture("default_black"), TextureType::Emissive);
	backpack_mat.addParameter("material.shininess", 32.0f);

	Material& light_source_mat = AssetManager::CreateMaterial("light_source", &AssetManager::GetShader("flat_emissive"));
	light_source_mat.addParameter("emissive", 1.0f, 1.0f, 1.0f);

	renderer->addMaterial(&AssetManager::GetMaterial("container"));
	renderer->addMaterial(&AssetManager::GetMaterial("light_source"));
	renderer->addMaterial(&AssetManager::GetMaterial("backpack"));


	//  models 
	AssetManager::CreateModel("container");
	AssetManager::GetModel("container").addMesh(&AssetManager::GetSingleMesh("default_cube"), &AssetManager::GetMaterial("container"));

	AssetManager::CreateModel("light_cube");
	AssetManager::GetModel("light_cube").addMesh(&AssetManager::GetSingleMesh("default_cube"), &AssetManager::GetMaterial("light_source"));

	AssetManager::LoadMeshCollection("backpack", "backpack/backpack.fbx");
	AssetManager::CreateModel("backpack").addMeshes(&AssetManager::GetMeshCollection("backpack"), &AssetManager::GetMaterial("backpack"));
}

void ExpositionGame::loadGame()
{
	loadScene(&expositionScene);
}


void ExpositionGame::unloadGame()
{
	renderer->removeMaterial(&AssetManager::GetMaterial("container"));
	renderer->removeMaterial(&AssetManager::GetMaterial("light_source"));
	renderer->removeMaterial(&AssetManager::GetMaterial("backpack"));
}