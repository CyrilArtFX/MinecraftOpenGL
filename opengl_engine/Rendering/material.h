#pragma once

#include "shader.h"
#include "texture.h"
#include <Maths/Vector3.h>

#include <vector>
#include <unordered_map>
#include <string>


enum class TextureType : uint8_t
{
	Undefined = 0,
	Diffuse = 1,
	Specular = 2,
	Emissive = 3
};


class Material
{
public:
	Material();
	void load(Shader* shaderUsed);

	void use();

	Shader& getShader() { return *shader; }
	Shader* getShaderPtr() { return shader; } 

	void addTexture(Texture* texture, TextureType type);

	void addParameter(std::string name, bool boolParameter);
	void addParameter(std::string name, int intParameter);
	void addParameter(std::string name, float floatParameter);
	void addParameter(std::string name, Vector3 vec3Parameter);
	void addParameter(std::string name, float vec3ParameterX, float vec3ParameterY, float vec3ParameterZ);


	static std::string TypeToString(TextureType textureType);


private:
	Shader* shader;
	std::unordered_map<TextureType, std::vector<Texture*>> textures;

	std::unordered_map<std::string, bool> boolParameters;
	std::unordered_map<std::string, int> intParameters;
	std::unordered_map<std::string, float> floatParameters;
	std::unordered_map<std::string, Vector3> vector3Parameters;
};