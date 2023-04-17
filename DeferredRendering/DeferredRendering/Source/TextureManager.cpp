#include "TextureManager.h"

int TextureManager::currentTexId = 0;

TextureManager::TextureManager()
{
	currentTexId = 0;
}

Texture TextureManager::AddTexture(const char* texFilePath)
{
	textures[textureCount] = Texture(currentTexId);
	textures[textureCount].CreateTexture(texFilePath);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[textureCount].GetTexture());

	textureCount++;
	currentTexId++;

	return textures[textureCount - 1];
}

Texture TextureManager::AddNormalMap(const char* normalFilePath, Texture *albedoTex)
{
	normals[normalCount] = Texture(currentTexId);
	normals[normalCount].CreateTexture(normalFilePath);

	albedoTex->SetNormalMap(&normals[normalCount]);

	normalCount++;
	currentTexId++;

	return normals[normalCount - 1];
}

Texture TextureManager::AddSpecularMap(const char* specFilePath, Texture *albedoTex)
{
	speculars[specularCount] = Texture(currentTexId);
	speculars[specularCount].CreateTexture(specFilePath);

	albedoTex->SetSpecularMap(&speculars[specularCount]);

	specularCount++;
	currentTexId++;

	return speculars[specularCount - 1];
}

void TextureManager::BindTexture(int index)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[index].GetTexture());

	for (int i = 0; i < normalCount; i++)
	{
		glActiveTexture(GL_TEXTURE0 + normals[i].texNumber);
		glBindTexture(GL_TEXTURE_2D, normals[i].GetTexture());
	}

	for (int i = 0; i < specularCount; i++)
	{
		glActiveTexture(GL_TEXTURE0 + speculars[i].texNumber);
		glBindTexture(GL_TEXTURE_2D, speculars[i].GetTexture());
	}
}