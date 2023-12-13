#pragma once
#include <Rendering/texture.h>

class AssetTexture
{
public:
	/**
	* Load a texture from file using stbi.
	*/
	static Texture LoadTexture(const std::string texturePath, TextureType textureType, unsigned int glFormat = GL_RGBA, bool flipVertical = false);
};

