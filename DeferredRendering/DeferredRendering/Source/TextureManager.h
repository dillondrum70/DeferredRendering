#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include "Texture.h"

const int MAX_TEXTURES = 10;

class TextureManager
{
public:
	GLuint startTex = GL_TEXTURE0;

	Texture textures[MAX_TEXTURES];
	int textureCount = 0;

	Texture normals[MAX_TEXTURES];
	int normalCount = 0;

	Texture speculars[MAX_TEXTURES];
	int specularCount = 0;

	static int currentTexId;	//Stores the current ID of the last added texture, keeps IDs unique

	TextureManager();

	Texture AddTexture(const char* texFilePath);
	Texture AddNormalMap(const char* normalFilePath, Texture *albedoTex);
	Texture AddSpecularMap(const char* specFilePath, Texture *albedoTex);

	void BindTexture(int index);
};

#endif