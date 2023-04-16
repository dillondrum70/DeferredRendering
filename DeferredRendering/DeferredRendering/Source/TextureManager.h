#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include "Texture.h"

const int MAX_TEXTURES = 16;
const int MAX_NORMALS = 16;

class TextureManager
{
public:
	Texture textures[MAX_TEXTURES];
	int textureCount = 0;

	Texture normals[MAX_NORMALS];
	int normalCount = 0;

	TextureManager();

	Texture AddTexture(const char* texFilePath);
	Texture AddNormalMap(const char* normalFilePath);
};

#endif