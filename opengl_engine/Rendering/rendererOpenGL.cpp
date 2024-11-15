#include "rendererOpenGL.h"
#include <Assets/assetManager.h>
#include <ServiceLocator/locator.h>

#include <iostream>
#include <algorithm>


void RendererOpenGL::draw()
{
	//  clear with flat color
	glClearColor(clearColor.r / 255.0f, clearColor.g / 255.0f, clearColor.b / 255.0f, clearColor.a / 255.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	
	if (!currentCam) return;

	//  RENDERING 3D
	// ===================


	Matrix4 view = currentCam->getViewMatrix();
	Matrix4 projection = Matrix4::createPerspectiveFOV(Maths::toRadians(currentCam->getFov()), windowSize.x, windowSize.y, 0.1f, 100.0f);

	//  loop through all shaders
	for (auto& materials_by_shaders : materials)
	{
		//  retrieve the shader
		Shader* shader = materials_by_shaders.first;

		if (!shader->isLoaded()) continue;

		//  activate the shader and set the primary uniforms
		shader->use();
		shader->setMatrix4("view", view.getAsFloatPtr());
		shader->setMatrix4("projection", projection.getAsFloatPtr());

		ShaderType shader_type = shader->getShaderType();
		switch (shader_type) //  feels a bit hardcoded, should be cool to find a better way to do this
		{
		case ShaderType::Lit:
			//  use lights
			for (auto light_t : lights)
			{
				LightType light_type = light_t.first;

				int light_type_used = 0;
				for (auto light : light_t.second)
				{
					if (!light->isLoaded()) continue;

					light->use(*shader, light_type_used);

					light_type_used++;
					if (light_type_used >= LIGHTS_LIMITS.at(light_type))
					{
						break;
					}
				}

				switch (light_type)
				{
				case EPointLight:
					shader->setInt("nbPointLights", light_type_used++);
					break;
				case ESpotLight:
					shader->setInt("nbSpotLights", light_type_used++);
					break;
				}
			}

			shader->setVec3("viewPos", currentCam->getPosition());

			break;

		case ShaderType::Unlit:
			//  nothing else to do
			break;
		}
		
		//  loop through all materials that use the shader
		for (auto& material : materials_by_shaders.second)
		{
			shader->setBool("beta_prevent_tex_scaling", false); //  should do a better thing for all beta parameters

			material->use();

			//  loop through all objects to draw all meshes that uses the material
			for (auto& object : objects)
			{
				object->draw(*material);
			}
		}
	}

	if (drawDebugMode)
	{
		Material& debug_collision_mat = AssetManager::GetMaterial("debug_collisions");
		Shader& debug_collision_shader = debug_collision_mat.getShader();
		debug_collision_shader.use();
		debug_collision_shader.setMatrix4("view", view.getAsFloatPtr());
		debug_collision_shader.setMatrix4("projection", projection.getAsFloatPtr());

		debug_collision_mat.use();

		Locator::getPhysics().DrawCollisionsDebug(debug_collision_mat);
	}



	//  RENDERING HUD
	// ===================

	glDisable(GL_DEPTH_TEST);

	Matrix4 hud_projection = Matrix4::createSimpleViewProj(windowSize.x, windowSize.y);

	//  prepare the shader used in text rendering
	Shader& text_render_shader = AssetManager::GetShader("text_render");
	text_render_shader.use();
	text_render_shader.setMatrix4("projection", hud_projection.getAsFloatPtr());

	//  bind the char vertex array
	AssetManager::GetVertexArray("hud_quad").setActive();

	for (auto& text : texts)
	{
		//  check text enabled
		if (!text->isEnabled()) continue;

		//  set text color
		text_render_shader.setVec3("textColor", text->getTextColor().toVector());

		//  prepare arrays of datas that will be sent to the shader
		int char_map_ids[TEXT_CHARS_LIMIT]{ 0 };
		Vector4 char_pos_scales[TEXT_CHARS_LIMIT]{ Vector4::zero };

		//  retrieve datas of the text
		float x = text->getTextScreenPosition().x;
		float y = text->getTextScreenPosition().y;
		const float begin_x = x;

		const std::string& text_text = text->getText();
		const float text_scale = text->getTextScale();

		//  get font and bind font texture array
		const Font& text_font = text->getTextFont();
		text_font.use();
		const int font_size = text_font.getFontSize();

		//  iterate through all characters
		std::string::const_iterator c;
		int index = 0;
		for (c = text_text.begin(); c != text_text.end(); c++)
		{
			if (index >= TEXT_CHARS_LIMIT - 1)
			{
				break;
			}

			FontCharacter ch = text_font.getCharacter(*c);

			if (*c == '\n')
			{
				y -= ((ch.Size.y)) * 1.6f * text_scale;
				x = begin_x;
			}
			else if (*c == ' ')
			{
				x += (ch.Advance >> 6) * text_scale; // bitshift by 6 (2^6 = 64) to advance the space character size
			}
			else
			{
				const float x_pos = x + ch.Bearing.x * text_scale;
				const float y_pos = y - (float(font_size) - ch.Bearing.y) * text_scale;
				const float x_scale = float(font_size) * text_scale;
				const float y_scale = float(font_size) * text_scale;

				char_pos_scales[index] = Vector4{ x_pos, y_pos, x_scale, y_scale };
				char_map_ids[index] = ch.TextureID;

				x += (ch.Advance >> 6) * text_scale; // bitshift by 6 (2^6 = 64) to advance the character size

				index++;
				if (index >= TEXT_CHARS_LIMIT)
				{
					//  draw array of max TEXT_CHARS_LIMIT chars
					text_render_shader.setVec4Array("textPosScales", &char_pos_scales[0], index);
					text_render_shader.setIntArray("letterMap", &char_map_ids[0], index);
					glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, index);

					index = 0;
				}
			}
		}

		//  draw array of remaining chars
		if (index > 0)
		{
			text_render_shader.setVec4Array("textPosScales", &char_pos_scales[0], index);
			text_render_shader.setIntArray("letterMap", &char_map_ids[0], index);
			glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, index);
		}

		//  unbind font texture array
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	}

	//  unbind char vertex array
	glBindVertexArray(0);
 }



void RendererOpenGL::SetCamera(Camera* camera)
{
	currentCam = camera;
}

const Camera& RendererOpenGL::GetCamera() const
{
	return *currentCam;
}


void RendererOpenGL::SetClearColor(Color clearColor_)
{
	clearColor = clearColor_;
}

const Color RendererOpenGL::GetClearColor() const
{
	return clearColor;
}


void RendererOpenGL::AddMaterial(Material* material)
{
	materials[material->getShaderPtr()].push_back(material);
}

void RendererOpenGL::RemoveMaterial(Material* material)
{
	auto iter = std::find(materials[material->getShaderPtr()].begin(), materials[material->getShaderPtr()].end(), material);
	if (iter == materials[material->getShaderPtr()].end())
	{
		std::cout << "Renderer can't remove a material that doesn't exist.\n";
		return;
	}

	std::iter_swap(iter, materials[material->getShaderPtr()].end() - 1);
	materials[material->getShaderPtr()].pop_back();
}


void RendererOpenGL::AddLight(Light* light)
{
	lights[light->getLightType()].push_back(light);

	if (lights[light->getLightType()].size() > LIGHTS_LIMITS.at(light->getLightType()))
	{
		std::cout << "Renderer Warning : A light has been added but will not be used as the lit shader array has a too small size.\n";
	}
}

void RendererOpenGL::RemoveLight(Light* light)
{
	auto iter = std::find(lights[light->getLightType()].begin(), lights[light->getLightType()].end(), light);
	if (iter == lights[light->getLightType()].end())
	{
		std::cout << "Renderer can't remove a light that doesn't exist.\n";
		return;
	}

	std::iter_swap(iter, lights[light->getLightType()].end() - 1);
	lights[light->getLightType()].pop_back();
}


void RendererOpenGL::AddObject(Object* object)
{
	objects.push_back(object);
}

void RendererOpenGL::RemoveObject(Object* object)
{
	auto iter = std::find(objects.begin(), objects.end(), object);
	if (iter == objects.end())
	{
		std::cout << "Renderer can't remove an object that doesn't exist.\n";
		return;
	}

	std::iter_swap(iter, objects.end() - 1);
	objects.pop_back();
}


void RendererOpenGL::AddText(TextRendererComponent* text)
{
	texts.push_back(text);
}

void RendererOpenGL::RemoveText(TextRendererComponent* text)
{
	auto iter = std::find(texts.begin(), texts.end(), text);
	if (iter == texts.end())
	{
		std::cout << "Renderer can't remove an text that doesn't exist.\n";
		return;
	}

	std::iter_swap(iter, texts.end() - 1);
	texts.pop_back();
}



void RendererOpenGL::initializeRenderer(Color clearColor_, Vector2Int windowSize_)
{
	clearColor = clearColor_;
	windowSize = windowSize_;
}

void RendererOpenGL::setWindowSize(Vector2Int windowSize_)
{
	windowSize = windowSize_;
}