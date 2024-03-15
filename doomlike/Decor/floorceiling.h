#pragma once
#include <Objects/object.h>

#include <Assets/assetManager.h>
#include <Rendering/renderer.h>


class FloorObj : public Object
{
public:
	FloorObj() {}
	FloorObj(Vector3 position, bool hasCollision = true);

	void load() override;
	void setup(Vector3 position, bool hasCollision = true);
};


class Ceiling : public Object
{
public:
	Ceiling() {}
	Ceiling(Vector3 position, bool hasCollision = true);

	void load() override;
	void setup(Vector3 position, bool hasCollision = true);
};









// ============================================================================
//  ---------------- Compact setup for code clarity --------------------------
// ============================================================================

namespace FloorCeilingSetup
{
	static void SetupFloorCeilings(Renderer& rendererRef)
	{
		AssetManager::LoadTexture("floor_diffuse", "pavement.jpg", GL_RGB, false);

		AssetManager::LoadTexture("ceiling_diffuse", "doomlike/tex_woodceiling/woodceiling_basecolor.jpg", GL_RGB, false);
		//AssetManager::LoadTexture("ceiling_specular", "doomlike/tex_woodceiling/woodceiling_roughness.jpg", GL_RED, false);

		Material& floor_mat = AssetManager::CreateMaterial("floor", &AssetManager::GetShader("lit_object"));
		floor_mat.addTexture(&AssetManager::GetTexture("floor_diffuse"), TextureType::Diffuse);
		floor_mat.addTexture(&AssetManager::GetTexture("default_black"), TextureType::Specular);
		floor_mat.addTexture(&AssetManager::GetTexture("default_black"), TextureType::Emissive);
		floor_mat.addParameter("material.shininess", 32.0f);
		floor_mat.addParameter("beta_prevent_tex_scaling", true);

		Material& ceiling_mat = AssetManager::CreateMaterial("ceiling", &AssetManager::GetShader("lit_object"));
		ceiling_mat.addTexture(&AssetManager::GetTexture("ceiling_diffuse"), TextureType::Diffuse);
		ceiling_mat.addTexture(&AssetManager::GetTexture("default_black"), TextureType::Specular);
		ceiling_mat.addTexture(&AssetManager::GetTexture("default_black"), TextureType::Emissive);
		ceiling_mat.addParameter("material.shininess", 32.0f);
		ceiling_mat.addParameter("beta_prevent_tex_scaling", true);

		rendererRef.addMaterial(&AssetManager::GetMaterial("floor"));
		rendererRef.addMaterial(&AssetManager::GetMaterial("ceiling"));

		AssetManager::CreateModel("floor");
		AssetManager::GetModel("floor").addMesh(&AssetManager::GetSingleMesh("default_plane"), &AssetManager::GetMaterial("floor"));

		AssetManager::CreateModel("ceiling");
		AssetManager::GetModel("ceiling").addMesh(&AssetManager::GetSingleMesh("default_plane"), &AssetManager::GetMaterial("ceiling"));
	}

	static void ReleaseFloorCeilings(Renderer& rendererRef)
	{
		rendererRef.removeMaterial(&AssetManager::GetMaterial("floor"));
		rendererRef.removeMaterial(&AssetManager::GetMaterial("ceiling"));
	}
};